/*---------------------------------------------------------------------------*
 *  SrecVoicetagItem.cpp                                                     *
 *                                                                           *
 *  Copyright 2007, 2008 Nuance Communciations, Inc.                               *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the 'License');          *
 *  you may not use this file except in compliance with the License.         *
 *                                                                           *
 *  You may obtain a copy of the License at                                  *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an 'AS IS' BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *---------------------------------------------------------------------------*/

//Memory leak detection
#if defined(_DEBUG) && defined(_WIN32)
#include "crtdbg.h"
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif

#include "SrecVoicetagItemImpl.h"
#include "SrecHelper.h"
#include "File.h"
#include "WorkerQueue.h"
#include "WorkerQueueFactory.h"
#include "LoggerImpl.h"
#include "System.h"
#include "SR_Nametag.h"
#include "SrecRecognizerImpl.h"

#if defined(UAPI_WIN32) || defined(UAPI_LINUX)
# include "FileASCII.h"
#endif

using namespace android::speech::recognition;
using namespace android::speech::recognition::impl;
using namespace android::speech::recognition::utilities;
using namespace android::speech::recognition::srec;


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace srec
      {
        DEFINE_SMARTPROXY(srec, SrecVoicetagItemImplProxy, VoicetagItemImplProxy, SrecVoicetagItemImpl)
        
        SrecVoicetagItemImplProxy SrecVoicetagItemImpl::create(SR_Nametag* nametag, const INT16* waveform,
        ARRAY_LIMIT size, VoicetagItemListenerProxy& listener, ReturnCode::Type& returnCode)
        {
          SrecVoicetagItemImpl* object = new SrecVoicetagItemImpl(nametag, waveform, size, listener);
          if (object == 0)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return SrecVoicetagItemImplProxy();
          }
          SrecVoicetagItemImplProxy result(object);
          if (!result)
            returnCode = ReturnCode::OUT_OF_MEMORY;
          object->rootProxy = result.getRoot();
          return result;
        }
        
        SrecVoicetagItemImplProxy SrecVoicetagItemImpl::create(const char* filename,
                                                      VoicetagItemListenerProxy &listener,
                                                      ReturnCode::Type& returnCode)
        {
           UAPI_FN_SCOPE("SrecVoicetagItemImpl::create");

           SrecVoicetagItemImpl* object = new SrecVoicetagItemImpl(filename,listener,returnCode);
           if (object == 0)
           {
               returnCode = ReturnCode::OUT_OF_MEMORY;
               return SrecVoicetagItemImplProxy();
           }
           else if (returnCode)
           {
                delete object;
                return SrecVoicetagItemImplProxy();
           }
           SrecVoicetagItemImplProxy result(object);
           if (!result)
               returnCode = ReturnCode::OUT_OF_MEMORY;
           object->rootProxy = result.getRoot();
           returnCode = ReturnCode::SUCCESS;
           return result;
        }

        SrecVoicetagItemImpl::SrecVoicetagItemImpl(SR_Nametag* nametag, const INT16* _waveform,
            ARRAY_LIMIT size, VoicetagItemListenerProxy &listener):
            VoicetagItemImpl(0,0),
            _filename(0), 
            _nametag(nametag),
            _listener(listener),
            _file(0),
            _workerQueue(0),
            rootProxy(0)
        {
            UAPI_FN_SCOPE("SrecVoicetagItemImpl::SrecVoicetagItemImpl");

            // We need to make a copy of the waveform, because 
            // SR_RecognizerResultGetWaveform will return pointer to buffer holding
            // audio data in it. This buffer is NOT under the application's control
            // and MUST only be read from.
            waveformSize = size;
            waveform = new INT16[size];
            if (waveform == 0)
            {
                UAPI_ERROR(fn,"Could not create _waveform, out of memory\n");
                waveformSize = 0;
            }
            memcpy((void*)waveform,_waveform,size*sizeof(INT16));
        }

        SrecVoicetagItemImpl::SrecVoicetagItemImpl(const char* filename, VoicetagItemListenerProxy &listener,
            ReturnCode::Type& returnCode):
             VoicetagItemImpl(0,0),
            _nametag(0),
            _listener(listener),
            _workerQueue(0),
            rootProxy(0)
        {

           ReturnCode::Type temp;
            UAPI_FN_SCOPE("SrecVoicetagItemImpl::SrecVoicetagItemImpl");
            UAPI_TRACE(fn,"this=%p\n", this);
           if (!filename)
           {
               returnCode = ReturnCode::ILLEGAL_ARGUMENT;
               return ;
           }
             // Create file object
            #if defined(UAPI_WIN32) || defined(UAPI_LINUX)
                _file = new FileASCII;
            #endif
            if (_file == 0)
            {
                returnCode=ReturnCode::OUT_OF_MEMORY;
                return;
            }
            _file->open(filename, "rb", returnCode);
           if (returnCode != ReturnCode::SUCCESS)
           {
                UAPI_ERROR(fn,"Could not open file %s reason %s\n", filename, ReturnCode::toString(returnCode));
                 return;
           }
           _file->close(temp);
            delete _file;

            // Store filename
            _filename = new char[strlen(filename)+1];
            if (_filename == 0)
            {
                returnCode=ReturnCode::OUT_OF_MEMORY;
                UAPI_ERROR(fn,"Could not create _filename, reason %s\n", ReturnCode::toString(returnCode));
                return;
            }
            strcpy(_filename, filename);         
        }

        
        SrecVoicetagItemImpl::~SrecVoicetagItemImpl()
        {
            UAPI_FN_SCOPE("SrecVoicetagItemImpl::~SrecVoicetagItemImpl");
            UAPI_TRACE(fn,"this=%p\n", this);
            if (_filename)
            {
                delete[] _filename;

            }
            _filename = 0;
            if (_file)
            {
                delete _file;
                _file = 0;
            }
            // Very important to destroy the nametag
            if (_nametag)
            {
               SR_NametagDestroy(_nametag);
              _nametag= NULL;
            }

        }
       
        bool SrecVoicetagItemImpl::isEnableGetWaveformOn()
        {
            ReturnCode::Type returnCode;
            srec::SrecRecognizerImplProxy nativeRecognizer = (srec::SrecRecognizerImplProxy) EmbeddedRecognizer::getInstance(returnCode);
            if (returnCode != ReturnCode::SUCCESS)
                return false;
            return nativeRecognizer->isSRBoolParameter(L("enableGetWaveform"));
        }

        void SrecVoicetagItemImpl::getAudio(const INT16** _waveform, ARRAY_LIMIT* _size,
                                ReturnCode::Type& returnCode) const
        {

            if (_waveform == 0 || _size == 0 || !isEnableGetWaveformOn())
            {
                returnCode = ReturnCode::INVALID_STATE;
                return;
            }
            *_waveform = waveform;
            *_size = waveformSize;
            returnCode = ReturnCode::SUCCESS;
        }

        void SrecVoicetagItemImpl::setAudio(const INT16* _waveform, ARRAY_LIMIT _size,
                                        ReturnCode::Type& returnCode)
        {
            if (_waveform == 0 || _size <= 0)
            {
                returnCode = ReturnCode::INVALID_STATE;
                return;
            }
            waveform = _waveform;
            waveformSize = _size;
            returnCode = ReturnCode::SUCCESS;
        }

        /**
        * Save the Voicetag Item.
        *
        * @param the path where the Voicetag will be saved. We strongly recommend to set the filename with the same value of the VoicetagId. If the filename is ommited, then the VoicetagId (assigned upon creation) will be used as default. 
        * @param returnCode the return code
        */
        void SrecVoicetagItemImpl::save(const char* path, ReturnCode::Type& returnCode)
        {
            UAPI_FN_SCOPE("SrecVoicetagItemImpl::save");
            if (!path)
            {
                returnCode = ReturnCode::ILLEGAL_ARGUMENT;
                return;
            }
            
            WorkerQueueFactory* workerFactory = WorkerQueueFactory::getInstance(returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {   
                UAPI_ERROR(fn,"Could not create the worker queue factory\n");
                return;
            }
            _workerQueue = workerFactory->getWorkerQueue(returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
                UAPI_ERROR(fn,"Could not create the worker queue\n");
                return;
            }
            // Copy path 
            char* pathCopy = 0;
            SaveVoicetagTask* task = 0;
            pathCopy = new char[strlen(path)+1];
            if (pathCopy == 0)
            {
                returnCode = ReturnCode::OUT_OF_MEMORY;
                goto CLEANUP;
            }
            strcpy(pathCopy, path);
            // Create Save Task
            {
                SrecVoicetagItemImplProxy proxy(rootProxy);
                if (!proxy)
                {
                    UAPI_ERROR(fn,"Could not create proxy\n");
                    returnCode = ReturnCode::INVALID_STATE;
                    delete[] pathCopy;
                    goto CLEANUP;
                }
                task = new SaveVoicetagTask(proxy, pathCopy);
                if (!task)
                {
                    UAPI_ERROR(fn,"Could not create SaveVoicetagTask\n");
                    returnCode = ReturnCode::OUT_OF_MEMORY;
                    delete[] pathCopy;
                    goto CLEANUP;
                }
                _workerQueue->enqueue(task, returnCode);
                if (returnCode != ReturnCode::SUCCESS)
                {
                    UAPI_ERROR(fn,"Could not enqueue SaveVoicetagTask\n");
                    delete task;
                    goto CLEANUP;
                }
            }
CLEANUP:
            return;
         }

        /**
        * Load a Voicetag Item.
        *
        * @param returnCode the return code
        */
        void SrecVoicetagItemImpl::load(ReturnCode::Type& returnCode)
        {
           UAPI_FN_SCOPE("SrecVoicetagItemImpl::load");
           if (!_filename)
           {
               UAPI_ERROR(fn,"Could not load this voicetag, because was not created from file\n");
               returnCode = ReturnCode::INVALID_STATE;
               return;
           }
           WorkerQueueFactory* workerFactory = WorkerQueueFactory::getInstance(returnCode);
           if (returnCode != ReturnCode::SUCCESS)
           {   
               UAPI_ERROR(fn,"Could not create the worker queue factory\n");
               return;
           }
           _workerQueue = workerFactory->getWorkerQueue(returnCode);
           if (returnCode != ReturnCode::SUCCESS)
           {
               UAPI_ERROR(fn,"Could not create the worker queue\n");
               return;
           }
           // Create load Task
            LoadVoicetagTask* task = 0;
           {
               SrecVoicetagItemImplProxy proxy(rootProxy);
               if (!proxy)
               {
                   UAPI_ERROR(fn,"Could not create proxy\n");
                   returnCode = ReturnCode::INVALID_STATE;
                   return;
               }
               task = new LoadVoicetagTask(proxy);
               if (!task)
               {
                   UAPI_ERROR(fn,"Could not create LoadVoicetagTask;\n");
                   returnCode = ReturnCode::OUT_OF_MEMORY;
                   return;
               }
               _workerQueue->enqueue(task, returnCode);
               if (returnCode != ReturnCode::SUCCESS)
               {
                   UAPI_ERROR(fn,"Could not enqueue LoadVoicetagTask\n");
                   delete task;
                   return;
               }
           }
        }

        SR_Nametag* SrecVoicetagItemImpl::getNametag()
        {
          return _nametag;
        }

        void SrecVoicetagItemImpl::onSave(const char* path)
        {
            UAPI_FN_SCOPE("SrecVoicetagItemImpl::onSave");
                
            ReturnCode::Type returnCode;
            char VoiceTagFilename[255];
            char SoundFilename[255];
            const char* value = NULL;
            UINT32 count;

            // If path not equal to null.
            if (!path)
            {
                returnCode=ReturnCode::ILLEGAL_ARGUMENT;
                goto CLEAN_UP;
            }
          
            strcpy((char*)&VoiceTagFilename,path);
            strcpy((char*)&SoundFilename,path);
            strcat((char*)&SoundFilename,"_raw");

            // Create file object
            #if defined(UAPI_WIN32) || defined(UAPI_LINUX)
                _file = new FileASCII;
            #endif
            if (_file == 0)
            {
                returnCode=ReturnCode::OUT_OF_MEMORY;
                goto CLEAN_UP;
            }

            /**
            * Save VoiceTag
            */
            _file->open(VoiceTagFilename, "wb+", returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
                UAPI_ERROR(fn,"Failed to create file %s reason %s\n", VoiceTagFilename,
                       ReturnCode::toString(returnCode));
                delete _file;
                _file = 0;
                goto CLEAN_UP;
            }

            ESR_ReturnCode  rc;
            size_t len,lenId;
            LCHAR* id;
            count = 1;
            rc = SR_NametagGetID(_nametag, &id);
            if (rc != ESR_SUCCESS)
            {
                 returnCode = SrecHelper::toUAPI(rc);
                 UAPI_ERROR(fn,"Failed to retrieve the nametag id reason:%s\n",
                       ReturnCode::toString(returnCode));
                 goto CLOSE_FILE;
            }
            lenId = strlen((const char*)id);
            
            rc = SR_NametagGetValue( _nametag, &value, &len);
            if (rc != ESR_SUCCESS)
            {
                 returnCode = SrecHelper::toUAPI(rc);
                 UAPI_ERROR(fn,"Failed to retrieve the nametag value reason:%s\n",
                       ReturnCode::toString(returnCode));
                 goto CLOSE_FILE;
            }

            if (lenId > 0)
            { 
               _file->write(&lenId,sizeof(size_t),count,returnCode);
               if (returnCode != ReturnCode::SUCCESS)
               {
                  UAPI_ERROR(fn,"Failed to write id size reason %s\n", ReturnCode::toString(returnCode));
                  goto CLOSE_FILE;
                }
               count = (UINT32)lenId;
               _file->write(id, sizeof(LCHAR), count, returnCode);
               if (returnCode != ReturnCode::SUCCESS)
               {
                  UAPI_ERROR(fn,"Failed to write ID %s reason %s\n", id, ReturnCode::toString(returnCode));
                  goto CLOSE_FILE;
                }
               count = 1;
               _file->write(&len,sizeof(size_t),count,returnCode);
               if (returnCode != ReturnCode::SUCCESS)
               {
                  UAPI_ERROR(fn,"Failed to write value size reason %s\n", ReturnCode::toString(returnCode));
                  goto CLOSE_FILE;
                }
               count = (UINT32)len;
               if (count>0)
               {
                    _file->write((void*)value, sizeof(char), count, returnCode);
                    if (returnCode != ReturnCode::SUCCESS)
                    {
                        UAPI_ERROR(fn,"Failed to write value %s reason %s\n", value, ReturnCode::toString(returnCode));
                        goto CLOSE_FILE;
                    }
               }
            }
            _file->close(returnCode);
            if (returnCode != ReturnCode::SUCCESS)
                goto CLEAN_UP;

            /**
            * Save Raw Sound
            */
            _file->open((const char*) &SoundFilename, "wb+", returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
                UAPI_ERROR(fn,"Failed to create file %s reason %s\n", SoundFilename,
                       ReturnCode::toString(returnCode));
                delete _file;
                _file = 0;
                goto CLEAN_UP;
            }
            
            if (waveformSize>0)
            {
                int szWave = waveformSize/2;
                count = szWave;
                if (count>0)
                {
                    _file->write((void*)waveform, sizeof(INT16), count, returnCode);
                    if (returnCode != ReturnCode::SUCCESS)
                    {
                        UAPI_ERROR(fn,"Failed to write waveform reason %s\n", ReturnCode::toString(returnCode));
                        goto CLOSE_FILE;
                    }
                }    
            }
CLOSE_FILE:
            // Close the file
            if (_file)
            {
                if (returnCode != ReturnCode::SUCCESS)
                {
                    // Already an error so don't care about it
                    ReturnCode::Type dummy;
                    _file->close(dummy);
                }
                else
                {  // So far so good let's close the file 
                   _file->close(returnCode);
                }
                delete _file;
                _file = NULL;
            }
CLEAN_UP:                  
            // Report status to listener
            if (_listener)
            {
                if (returnCode != ReturnCode::SUCCESS)
                    _listener->onError(returnCode);
                else 
                    _listener->onSaved(path);
            }
        }

        SaveVoicetagTask::~SaveVoicetagTask()
        {
            delete[] path;
        }

        SaveVoicetagTask::SaveVoicetagTask(SrecVoicetagItemImplProxy& _voicetag, const char* _path):
            Task("SaveVoicetagTask"),
            path(_path),
            voicetag(_voicetag)
        {}

        void SaveVoicetagTask::run()
        {
            voicetag->onSave(path);
        }

        void SrecVoicetagItemImpl::onLoad()
        {
            UAPI_FN_SCOPE("SrecVoicetagItemImpl::onLoad");
       
            ReturnCode::Type returnCode;
            char VoiceTagFilename[255];
            char SoundFilename[255];
            char* value = NULL;
            const INT16* _waveform = NULL;
            ARRAY_LIMIT _waveformSize = 0;   
            UINT32 fileLength = 0;

            // If path not equel to null.
            if (!_filename)
            {
                returnCode=ReturnCode::ILLEGAL_ARGUMENT;
                goto CLEAN_UP;
            }
        
            strcpy((char*)&VoiceTagFilename,_filename);
            strcpy((char*)&SoundFilename,_filename);
            strcat((char*)&SoundFilename,"_raw");

            // Create file object
            #if defined(UAPI_WIN32) || defined(UAPI_LINUX)
                _file = new FileASCII;
            #endif
            if (_file == 0)
            {
                returnCode=ReturnCode::OUT_OF_MEMORY;
                goto CLEAN_UP;
            }

            ESR_ReturnCode  rc;
            LCHAR* id;
            UINT32 szValue, szId, szWaveform;
            UINT32 count;

            /**
            * Load VoiceTag
            */
            _file->open((const char*)&VoiceTagFilename, "rb", returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
                UAPI_ERROR(fn,"Could not open file %s reason %s\n", VoiceTagFilename, ReturnCode::toString(returnCode));
                delete _file;
                _file = 0;
                goto CLEAN_UP;
            }
	                
            // read data as a block:
            count = 1;
            _file->read(&szId, sizeof(UINT32), count, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
               UAPI_ERROR(fn,"Failed to read id size, reason %s\n", ReturnCode::toString(returnCode));
               goto CLOSE_FILE;
            }
            count = szId;
            if (count == 0)
            {
               UAPI_ERROR(fn,"Id size is Zero!\n");
               returnCode = ReturnCode::END_OF_STREAM;
               goto CLOSE_FILE;
            }
            id = new LCHAR[szId+1];
            if (id == 0)
            {
                returnCode=ReturnCode::OUT_OF_MEMORY;
                UAPI_ERROR(fn,"Could not create id, reason %s\n", ReturnCode::toString(returnCode));
                goto CLOSE_FILE;
            }
            _file->read((void*)id, sizeof(LCHAR), count, returnCode);
            id[szId]='\0';
            if (returnCode != ReturnCode::SUCCESS)
            {
               UAPI_ERROR(fn,"Failed to read id string, reason %s\n", ReturnCode::toString(returnCode));
               delete[] id;
               goto CLOSE_FILE;
            }
            count = 1;
            _file->read(&szValue, sizeof(UINT32), count, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
               UAPI_ERROR(fn,"Failed to read phoneme value size, reason %s\n", ReturnCode::toString(returnCode));
               delete[] id;
               goto CLOSE_FILE;
            }
            count = szValue;
            if (count == 0)
            {
               UAPI_ERROR(fn,"Phoneme value size is Zero!\n");
               returnCode = ReturnCode::END_OF_STREAM;
               goto CLOSE_FILE;
            }
            value = new char[szValue+1];
            if (value == 0)
            {
                returnCode=ReturnCode::OUT_OF_MEMORY;
                UAPI_ERROR(fn,"Could not create phoneme value, reason %s\n", ReturnCode::toString(returnCode));
                delete[] id;
                goto CLOSE_FILE;
            }
            _file->read((void*)value, sizeof(char), count, returnCode);
            value[szValue]='\0';
            if (returnCode != ReturnCode::SUCCESS)
            {
               UAPI_ERROR(fn,"Failed to read phone value, reason %s\n", ReturnCode::toString(returnCode));
               delete[] id;
               delete[] value;
               goto CLOSE_FILE;
            }
            _file->close(returnCode);
            if (returnCode != ReturnCode::SUCCESS)
               goto CLEAN_UP;

            if (_nametag)
            {
                rc = SR_NametagDestroy(_nametag);
	            _nametag = NULL; 
            }

            // Create new name tag
            rc = SR_NametagCreateFromValue(id,  value, szValue, &_nametag);
            if (rc != ESR_SUCCESS)
            {
                returnCode = SrecHelper::toUAPI(rc);
                UAPI_ERROR(fn,"Failed to create nametag from value:%s\n",
                      ReturnCode::toString(returnCode));
               delete[] id;
               delete[] value;
               goto CLEAN_UP;
            }
            delete[] id;
            delete[] value;
	        value = NULL;

            /**
            * Load Raw Sound
            */
            _file->open((const char*)&SoundFilename, "rb", returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
                UAPI_ERROR(fn,"Could not open file %s reason %s\n", SoundFilename, ReturnCode::toString(returnCode));
                delete _file;
                _file = 0;
                goto CLEAN_UP;
            }
            // get length of file:
           
            _file->seek(0, File::UAPI_SEEK_END, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
                UAPI_ERROR(fn,"Could seek to the end of the file reason %s\n", ReturnCode::toString(returnCode));
                goto CLOSE_FILE;
            }
            _file->getPosition(fileLength, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
                UAPI_ERROR(fn,"Could not get the file length reason %s\n", ReturnCode::toString(returnCode));
                goto CLOSE_FILE;
            }
            _file->seek(0, File::UAPI_SEEK_SET, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
                UAPI_ERROR(fn,"Could seek to the begin of the file reason %s\n", ReturnCode::toString(returnCode));
                goto CLOSE_FILE;
            }
            szWaveform = (int)fileLength/2;
            count = szWaveform;
            if (count > 0)
            {
                _waveformSize = (int)fileLength;
                _waveform = new INT16[szWaveform];
                if (_waveform == 0)
                {
                    returnCode=ReturnCode::OUT_OF_MEMORY;
                    UAPI_ERROR(fn,"Could not create _waveform, reason %s\n", ReturnCode::toString(returnCode));
                    goto CLOSE_FILE;
                }
                _file->read((void*)_waveform, sizeof(INT16), count, returnCode);
                if (returnCode != ReturnCode::SUCCESS)
                {
                    UAPI_ERROR(fn,"Failed to read _waveform, reason %s\n", ReturnCode::toString(returnCode));
                    delete[] _waveform;
                    goto CLOSE_FILE;
                }
            }
            if (_waveform) 
            {
                // Set audio
                waveform = _waveform;
                waveformSize = _waveformSize;
            }
CLOSE_FILE:
            // Close the file
            if (_file)
            {
                if (returnCode != ReturnCode::SUCCESS)
                {
                    // Already an error so don't care about it
                    ReturnCode::Type dummy;
                    _file->close(dummy);
                }
                else
                {  // So far so good let's close the file 
                   _file->close(returnCode);
                }
                delete _file;
                _file = NULL;
            }
CLEAN_UP:         
            // Report status to listener
            if (_listener)
            {
                if (returnCode != ReturnCode::SUCCESS)
                    _listener->onError(returnCode);
                else 
                    _listener->onLoaded();
            }
        }

        LoadVoicetagTask::LoadVoicetagTask(SrecVoicetagItemImplProxy& _voicetag):
            Task("LoadVoicetagTask"),
            voicetag(_voicetag)
        {}

        void LoadVoicetagTask::run()
        {
            voicetag->onLoad();
        }
        
      }
    }
  }
}
