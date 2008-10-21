/*---------------------------------------------------------------------------*
 *  MediaFileWriterImpl.cpp                                                  *
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
#include <string.h>
#include "MediaFileWriterImpl.h"
#include "MediaFileWriterListener.h"
#include "File.h"
#include "WorkerQueue.h"
#include "WorkerQueueFactory.h"
#include "AudioStreamImpl.h"
#include "AudioBuffer.h"
#include "LoggerImpl.h"
#include "LoggerImpl.h"
#include "System.h"
#include "WavePCMHelper.h"
#include "Codec.h"

#if defined(UAPI_WIN32) || defined(UAPI_LINUX)
# include "FileASCII.h"
#endif

using namespace android::speech::recognition::utilities;


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      MediaFileWriterProxy MediaFileWriter::create(MediaFileWriterListenerProxy& listener,
          ReturnCode::Type& returnCode)
      {
        UAPI_FN_SCOPE("DeviceSpeakerWIN::create");
          
        impl::MediaFileWriterImpl* mediaFileWriter = new impl::MediaFileWriterImpl(listener);
        if (!mediaFileWriter)
        {
          returnCode = ReturnCode::OUT_OF_MEMORY;
          return MediaFileWriterProxy();
        }
        impl::MediaFileWriterImplProxy result(mediaFileWriter);
        if (!result)
        {
          returnCode = ReturnCode::OUT_OF_MEMORY;
          return result;
        }
        mediaFileWriter->rootProxy = result.getRoot();
        returnCode = ReturnCode::SUCCESS;
        return result;
      }
      
      namespace impl
      {
        DEFINE_SMARTPROXY(impl, MediaFileWriterImplProxy, MediaFileWriterProxy, MediaFileWriterImpl)
        
        MediaFileWriterImpl::MediaFileWriterImpl(MediaFileWriterListenerProxy& _listener):
            file(0),
            workerQueue(0),
            listener(_listener),
            state(IDLE),
            rootProxy(0),
            bHeaderSaved(false)
        {
          UAPI_FN_NAME("MediaFileWriterImpl::MediaFileWriter");
            
          UAPI_TRACE(fn,"this=%p\n", this);
        }
        
        void MediaFileWriterImpl::closeFile(ReturnCode::Type& returnCode)
        {
            UAPI_FN_NAME("MediaFileWriterImpl::closeFile");
            if (file)
            {
                file->close(returnCode);
                if (returnCode)
                    UAPI_WARN(fn,"MediaFileWriterImpl::closeFile::Failed to close file\n");
            }
        }

        void MediaFileWriterImpl::init(const char* fileName, ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("MediaFileWriterImpl::init");
            
#if defined(UAPI_WIN32) || defined(UAPI_LINUX)
          file = new FileASCII;
#endif
          if (file == 0)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }

          
          file->open(fileName, "wb+", returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Failed to create file %s reason %s\n", fileName,
                       ReturnCode::toString(returnCode));
            delete file;
            file = 0;
            return;
          }
          
          WorkerQueueFactory* workerFactory = WorkerQueueFactory::getInstance(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not create the worker queue factory\n");
            return;
          }
          workerQueue = workerFactory->getWorkerQueue(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not create the worker queue\n");
            return;
          }
        }
        
        MediaFileWriterImpl::~MediaFileWriterImpl()
        {
          ReturnCode::Type returnCode;
          UAPI_FN_NAME("MediaFileWriterImpl::~MediaFileWriter");
          UAPI_TRACE(fn,"this=%p\n", this);
          
          if (file)
          {
            file->close(returnCode);
            delete file;
          }
        }
        
        void MediaFileWriterImpl::save(AudioStreamProxy& audio, const char* path, ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("MediaFileWriterImpl::save");
          ReturnCode::Type rc;  
          if (!audio)
          {
            returnCode = ReturnCode::ILLEGAL_ARGUMENT;
            return;
          }
          
          init(path, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
            return;
            
          if (file == 0 || workerQueue == 0)
          {
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          
          AudioStreamImplProxy& audioStreamImpl = (AudioStreamImplProxy&) audio;
          
          //Flag this AudioStreamImpl as being locked. This means that this audio cannot be
          //passed to other modules.
          audioStreamImpl->lock(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Cannot lock the audio stream\n");
            closeFile(rc);
            return;
          }
          
          ReturnCode::Type dummy;
          MediaFileWriterImplProxy proxy(rootProxy);
          if (!proxy)
          {
            UAPI_ERROR(fn,"Could not create proxy\n");
            cleanupAudioStreamProxy(audioStreamImpl, dummy);
            returnCode = ReturnCode::INVALID_STATE;
            closeFile(rc);
            return;
          }
          StartWriteFileTask* task = new StartWriteFileTask(proxy, audioStreamImpl);
          if (!task)
          {
            UAPI_ERROR(fn,"Could not create StartWriteFileTask\n");
            cleanupAudioStreamProxy(audioStreamImpl, dummy);
            returnCode = ReturnCode::OUT_OF_MEMORY;
            closeFile(rc);
            return;
          }
          workerQueue->enqueue(task, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Failed to enqueue StartWriteFileTask reason %s\n",
                       ReturnCode::toString(returnCode));
            cleanupAudioStreamProxy(audioStreamImpl, dummy);
            delete task;
            closeFile(rc);
            return;
          }
          returnCode = ReturnCode::SUCCESS;
          return;
        }
        
        void MediaFileWriterImpl::runStartWriteFileTask(AudioStreamImplProxy& audioStream)
        {
          UAPI_FN_SCOPE("MediaFileWriterImpl::runStartWriteFileTask");
            
          if (state != IDLE)
          {
            UAPI_ERROR(fn,"Must be in IDLE state to call save\n");
            if (listener)
              listener->onError(ReturnCode::INVALID_STATE);
            return;
          }
          
          state = SAVING;
          bHeaderSaved = false;
          
          runWriteFileTask(audioStream);
        }
        
        void MediaFileWriterImpl::runWriteFileTask(AudioStreamImplProxy& audioStream)
        {
          ReturnCode::Type returnCode = ReturnCode::SUCCESS;
          UAPI_FN_SCOPE("MediaFileWriterImpl::runWriteFileTask");
            
          while (state == SAVING && returnCode != ReturnCode::END_OF_STREAM)
          {
            AudioBuffer* buffer = audioStream->read(returnCode);
            if (buffer)
            {
              UINT32 size = buffer->size;
              if (size > 0)
              {

                if( bHeaderSaved == false )
                {
                  bHeaderSaved = true;

                  UINT16 sampleRate = Codec::getSampleRate( audioStream->getCodec(), returnCode);

                  UINT16 bitRate = Codec::getBitsPerSample(audioStream->getCodec(), returnCode);

                  //add the RIFF header to be beginning of the file. Set the
                  //size to 0 for now. It will get update once we are done
                  //saving the file.
                  WavePCMHelper::writeWavHeader(file, sampleRate, 1, bitRate, 0, returnCode);
                  if (returnCode != ReturnCode::SUCCESS)
                  {
                    UAPI_ERROR(fn,"Failed to write wave header %s\n",
                        ReturnCode::toString(returnCode));
                    sendOnErrorToListener(audioStream, returnCode);
                    return;
                  }
                }

                file->write(buffer->buffer, sizeof(char), size, returnCode);
                if (returnCode != ReturnCode::SUCCESS)
                {
                  UAPI_ERROR(fn,"Failed to write buffer of size %d reason %s\n", size, ReturnCode::toString(returnCode));
                  sendOnErrorToListener(audioStream, returnCode);
                  return;
                }
              }
              
              //make sure we release the buffer, it is ref counted.
              audioStream->release(buffer, returnCode);
              if (returnCode != ReturnCode::SUCCESS)
              {
                UAPI_ERROR(fn,"Failed to release audio buffer %p\n", buffer);
                sendOnErrorToListener(audioStream, returnCode);
                return;
              }
            }
            else
            {
              if (returnCode == ReturnCode::END_OF_STREAM)
              {

								//update the file length of the RIFF header.
								UINT32 total_size;
								file->getPosition(total_size, returnCode);
                if (returnCode != ReturnCode::SUCCESS)
                {
                  UAPI_ERROR(fn,"Failed to getPosition during END_OF_STREAM\n");
                  sendOnErrorToListener(audioStream, returnCode);
									return;
                }

								//substract 44, which is the size of the header. Remember that
								//the header was added at the beginning.
								WavePCMHelper::updateFileLength(file, total_size - 44, returnCode);
                if (returnCode != ReturnCode::SUCCESS)
                {
                  UAPI_ERROR(fn,"Failed to update the file length\n");
                  sendOnErrorToListener(audioStream, returnCode);
									return;
                }

                //nothing else to read, close the file.
                file->close(returnCode);
                if (returnCode != ReturnCode::SUCCESS)
                {
                  UAPI_ERROR(fn,"Failed to close file during END_OF_STREAM\n");
                  sendOnErrorToListener(audioStream, returnCode);
                }
                else
                  sendOnStoppedToListener(audioStream);   //NORMAL CASE
                return;
              }
              else if (returnCode == ReturnCode::PENDING_DATA)
              {
                //we were not able to read a buffer. We will have to try again
                //a little later (50 ms).
                ReturnCode::Type returnCode;
                MediaFileWriterImplProxy proxy(rootProxy);
                if (!proxy)
                {
                  UAPI_ERROR(fn,"Could not create proxy\n");
                  sendOnErrorToListener(audioStream, ReturnCode::OUT_OF_MEMORY);
                  return;
                }
                WriteFileTask* task = new WriteFileTask(proxy, audioStream, 50);
                if (!task)
                {
                  UAPI_ERROR(fn,"Could not create WriteFileTask\n");
                  sendOnErrorToListener(audioStream, ReturnCode::OUT_OF_MEMORY);
                  return;
                }
                workerQueue->enqueue(task, returnCode);
                if (returnCode != ReturnCode::SUCCESS)
                {
                  UAPI_ERROR(fn,"Failed to enqueue WriteFileTask reason %s\n", ReturnCode::toString(returnCode));
                  sendOnErrorToListener(audioStream, returnCode);
                  delete task;
                  return;
                }
              }
              break;
            }
          }
        }
        
        StartWriteFileTask::StartWriteFileTask(MediaFileWriterImplProxy& _mediaFileWriter,
                                               AudioStreamImplProxy& _audioStream):
            Task("StartWriteFileTask"),
            mediaFileWriter(_mediaFileWriter),
            audioStream(_audioStream)
        {}
        
        void StartWriteFileTask::run()
        {
          mediaFileWriter->runStartWriteFileTask(audioStream);
        }
        
        WriteFileTask::WriteFileTask(MediaFileWriterImplProxy& _mediaFileWriter,
                                     AudioStreamImplProxy& _audioStream,
                                     UINT32 timeout):
            ScheduledTask(timeout, "WriteFileTask"),
            mediaFileWriter(_mediaFileWriter),
            audioStream(_audioStream)
        {}
        
        void WriteFileTask::run()
        {
          mediaFileWriter->runWriteFileTask(audioStream);
        }
        
        void MediaFileWriterImpl::sendOnErrorToListener(AudioStreamImplProxy& audioStream,
            ReturnCode::Type error)
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("MediaFileWriterImpl::sendOnErrorToListener");
            
          cleanupAudioStreamProxy(audioStream, returnCode);
          //don't care about rc, we are already sending an error.
          
          // Adding this to fix error. Close file now!
          closeFile(returnCode);
            
          state = IDLE;

          if (listener)
            listener->onError(error);
        }
        
        void MediaFileWriterImpl::sendOnStoppedToListener(AudioStreamImplProxy& audioStream)
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("MediaFileWriterImpl::sendOnStoppedToListener");

          cleanupAudioStreamProxy(audioStream, returnCode);

          state = IDLE;

          if (returnCode) 
          {
            if (listener)
              listener->onError(returnCode);
          }
          else
          {
            //normal case
            if (listener)
              listener->onStopped();
          }
        }
        
        void MediaFileWriterImpl::cleanupAudioStreamProxy(AudioStreamImplProxy& audioStream, ReturnCode::Type& returnCode)
        {
          audioStream->unlock();
          audioStream = AudioStreamImplProxy();
          returnCode = ReturnCode::SUCCESS;
        }
      }
    }
  }
}
