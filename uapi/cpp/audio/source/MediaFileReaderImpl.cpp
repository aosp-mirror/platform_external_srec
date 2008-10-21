/*---------------------------------------------------------------------------*
 *  MediaFileReaderImpl.cpp                                                  *
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
#include "MediaFileReaderImpl.h"
#include "MediaFileReaderListener.h"
#include "File.h"
#include "WorkerQueue.h"
#include "WorkerQueueFactory.h"
#include "CodecHelper.h"
#include "AudioStreamImpl.h"
#include "AudioQueue.h"
#include "LoggerImpl.h"
#include "System.h"
#include "WavePCMHelper.h"

#include <stdlib.h>

#if defined(UAPI_WIN32) || defined(UAPI_LINUX)
# include "FileASCII.h"
#endif

using namespace android::speech::recognition;
using namespace android::speech::recognition::impl;
using namespace android::speech::recognition::utilities;


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      MediaFileReaderProxy MediaFileReader::create(const char* pszFileName,
          AudioSourceListenerProxy& listener,
          ReturnCode::Type& returnCode)
      {
        UAPI_FN_SCOPE("MediaFileReader::create");
          
        MediaFileReaderImpl* object =
          new MediaFileReaderImpl(pszFileName, listener, returnCode);
        if (object == 0)
        {
          returnCode = ReturnCode::OUT_OF_MEMORY;
          return MediaFileReaderProxy();
        }
        else if (returnCode)
        {
          delete object;
          return MediaFileReaderProxy();
        }
        MediaFileReaderProxy result(object);
        if (!result)
        {
          returnCode = ReturnCode::OUT_OF_MEMORY;
          return result;
        }
        object->rootProxy = result.getRoot();
        returnCode = ReturnCode::SUCCESS;
        return result;
      }
      
      namespace impl
      {
        DEFINE_SMARTPROXY(impl, MediaFileReaderImplProxy, MediaFileReaderProxy, MediaFileReaderImpl)
        
        MediaFileReaderImpl::MediaFileReaderImpl(const char* pszFileName,
            AudioSourceListenerProxy& _listener,
            ReturnCode::Type& returnCode):
            file(0),
            mode(ALL_AT_ONCE),
            headerOffset(0),
            listener(_listener),
            codec(Codec::PCM_16BIT_11K),
            workerQueue(0),
            audioQueue(0),
            state(IDLE),
            pendingTask(0),
            _numBufferNeededToCatchUp(2)
        {
          UAPI_FN_SCOPE("MediaFileReader::MediaFileReaderImpl");
          UAPI_TRACE(fn,"this=%p\n", this);
          
        
          
#if defined(UAPI_WIN32) || defined(UAPI_LINUX)
          file = new FileASCII;
#endif
          if (file == 0)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          
          file->open(pszFileName, "rb", returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not open file %s reason %s\n", pszFileName, ReturnCode::toString(returnCode));
            return;
          }
          
          WavePCMHelper::getWavFileInfo( file, codec, headerOffset, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"file %s is not a supported wav file\n", pszFileName);
            return;
          }

          audioQueue = AudioQueue::create(codec, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not create AudioQueue\n");
            return;
          }
          
          UINT32 fileLength = getFileLength(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"cannot get the file length reason %s\n", ReturnCode::toString(returnCode));
            ReturnCode::Type temp;
            file->close(temp);
            return;
          }
          
          if (headerOffset >= fileLength)
          {
            UAPI_ERROR(fn,"Header offset (%u) cannot be greater or equal to the file length (%u)\n", headerOffset, fileLength);
            file->close(returnCode);
            returnCode = ReturnCode::ILLEGAL_ARGUMENT;
            return;
          }
          
          WorkerQueueFactory* workerFactory = WorkerQueueFactory::getInstance(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not create the worker queue factory\n");
            ReturnCode::Type temp;
            file->close(temp);
            return;
          }
          workerQueue = workerFactory->getWorkerQueue(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not create the worker queue\n");
            ReturnCode::Type temp;
            file->close(temp);
            return;
          }
        }
             
        MediaFileReaderImpl::~MediaFileReaderImpl()
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("MediaFileReader::~MediaFileReaderImpl");
          UAPI_TRACE(fn,"this=%p\n", this);
          
          if (file)
          {
            file->close(returnCode);
            if (returnCode)
              UAPI_WARN(fn,"Failed to close file\n");
              
            delete file;
          }
          
          if (audioQueue != 0)
          {
            //NOTE: we don't call delete audioQueue. This class is ref counted.
            //Instead, we call Release on it and once no one is using the object, it
            //will get released.
            
            audioQueue->removeRef(returnCode);
            if (returnCode != ReturnCode::SUCCESS)
              UAPI_WARN(fn,"Failed to stop worker queue\n");
          }
        }
        
        
        void MediaFileReaderImpl::setReadingMode(ReadingMode _mode, ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("MediaFileReader::setReadingMode");
            
          mode = _mode;
          returnCode = ReturnCode::SUCCESS;
        }
        
        
        void MediaFileReaderImpl::start(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("MediaFileReaderImpl::start");
            
          if (file == 0)
          {
            UAPI_ERROR(fn,"file is null");
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          else if (workerQueue == 0)
          {
            UAPI_ERROR(fn,"workerQueue is null");
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          else if (audioQueue == 0)
          {
            UAPI_ERROR(fn,"audioQueue is null");
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          
          MediaFileReaderImplProxy proxy(rootProxy);
          if (!proxy)
          {
            UAPI_ERROR(fn,"Could not create MediaFileReaderImplProxy: %s\n", ReturnCode::toString(returnCode));
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          pendingTask = new StartReadingFileTask(proxy);
          if (!pendingTask)
          {
            UAPI_ERROR(fn,"Could not create StartReadingFileTask\n");
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          workerQueue->enqueue(pendingTask, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Failed to enqueue StartReadingFileTask\n");
            delete pendingTask;
            pendingTask = 0;
            return;
          }
          
          returnCode = ReturnCode::SUCCESS;
        }
        
        void MediaFileReaderImpl::runStartReadingFileTask()
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("MediaFileReader::runStartReadingFileTask");
            
          if (state != IDLE)
          {
            UAPI_ERROR(fn,"Must be in IDLE state to call start\n");
            if (listener)
              listener->onError(ReturnCode::INVALID_STATE);
            return;
          }
          
          state = READING;
          
          UINT32 fileLength = getFileLength(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"cannot get the file length reason %s\n", ReturnCode::toString(returnCode));
            if (listener)
              listener->onError(returnCode);
            return;
          }
          
          fileLength -= headerOffset; //substract header
          
          file->seek(headerOffset, File::UAPI_SEEK_SET, returnCode);   //skip the header
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not seek after the file header reason %s\n", ReturnCode::toString(returnCode));
            if (listener)
              listener->onError(returnCode);
            return;
          }
          
          UINT16 bufferSize = CodecHelper::GetPreferredFileReaderBufferSize(codec);
          numFullBuffers = fileLength / bufferSize;
          lastBuffNumBytes = (UINT16)(fileLength % bufferSize);
          
          if (lastBuffNumBytes > 0)
            ++numFullBuffers;
          
          if (mode == REAL_TIME)
          {
            //we are trying to read real time. To do this, we will compute the
            //time at which we should be done reading the file.
            expireTime = TimeInstant::now();
            UINT32 fileDurationMs = CodecHelper::GetNumMsecForNumBytes(codec, fileLength);
            UAPI_INFO(fn,"Audio file contains %u msec of audio\n", fileDurationMs);
            expireTime = expireTime.plus(fileDurationMs);
          }


          bufferReadIndex = 0;
          if (listener)
            listener->onStarted();
            
          runReadFileTask();
        }
        
        void MediaFileReaderImpl::runReadFileTask()
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("MediaFileReader::runReadFileTask");

          //we are in the worker queue thread.

          UINT32 numBufferToRead = 1;

          //compute the time we have to left to play the whole file.
          if (mode == REAL_TIME)
          {
            TimeInstant now = TimeInstant::now(); 
            INT32 timeLeftToPlay = expireTime.minus(now, returnCode);
            if (returnCode)
            {
              UAPI_INFO(fn,"TimeInstant.minus() error: %s\n", ReturnCode::toString(returnCode));
              //unexpected error, send an error to the application.
              if (listener)
                listener->onError(returnCode);
              return;
            }

            UINT32 timeLeftPerBuffer = 0;

            //here we will check how much time we have left per buffer
            if( timeLeftToPlay > 0 )
              timeLeftPerBuffer = timeLeftToPlay / (numFullBuffers - 1 - bufferReadIndex);


            UINT32 numMsPerBufferToRead = CodecHelper::GetNumMsecForNumBytes(codec, CodecHelper::GetPreferredFileReaderBufferSize(codec));
            // experiments has shown that 4/5 is a good number.
            if( timeLeftPerBuffer < (numMsPerBufferToRead *4/5) )
            {
              //we are late, try to catch up by submitting multiple buffers at
              //once. The more we are late reading the buffers, the more
              //buffers we will read during a ReadFileTask. This should help us
              //catch up.
              numBufferToRead = _numBufferNeededToCatchUp++;
            }
            else
            {
              //set back to default.
              _numBufferNeededToCatchUp = 2;
            }

            UAPI_INFO(fn,"Time left per buffer is %u, submitting %u buffers numFullBuffers %u "
                "bufferReadIndex %u\n", timeLeftPerBuffer, numBufferToRead, numFullBuffers, 
                bufferReadIndex);
          }

          while( numBufferToRead > 0 )
          {
            
            if (state == STOPPING)
            {
              //notify the listener that we are done with the stop
              onReadFileDone();
              return;
            }
            
            UINT32 nextReadDelay = 0;
            
            UINT16 bufferSize = CodecHelper::GetPreferredFileReaderBufferSize(codec);
            
            UINT32 bytesToRead = bufferSize;
            bool isLastBuffer = false;
            if (bufferReadIndex == (numFullBuffers - 1))
            {
              if (lastBuffNumBytes > 0)
                bytesToRead = lastBuffNumBytes;
              isLastBuffer = true;
            }
            
            unsigned char* buffer = (unsigned char*) malloc(bytesToRead);
            
            // read data as a block:
            file->read(buffer, sizeof(char), bytesToRead, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
              UAPI_ERROR(fn,"read from file returned %s. buffer 0x%p bytesToRead %u \n",
                         ReturnCode::toString(returnCode), buffer, bytesToRead);
              if (buffer)
                free(buffer);
              //unexpected error, send an error to the application.
              if (listener)
                listener->onError(returnCode);
              return;
            }
            
            audioQueue->addBuffer(buffer, bytesToRead, isLastBuffer, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
              UAPI_ERROR(fn,"Failed to AddBuffer pBuf %p size %d reason%s\n", buffer, 
                  bytesToRead, ReturnCode::toString(returnCode));
              free(buffer);
              //unexpected error, send an error to the application.
              if (listener)
                listener->onError(returnCode);
              return;
            }
            
            free(buffer);
            
            ++bufferReadIndex;
            
            if (isLastBuffer)
            {
              //notify the listener that we are done
              onReadFileDone();
              return;
            }
            else
            {
              --numBufferToRead;
              if( numBufferToRead > 0 )
                continue;

              //compute the next read delay
              if (mode == REAL_TIME)
              {
                TimeInstant now = TimeInstant::now(); 
                INT32 timeLeftToPlay = expireTime.minus(now, returnCode);
                if (returnCode)
                {
                  UAPI_INFO(fn,"TimeInstant.minus() error: %s\n", ReturnCode::toString(returnCode));
                  //unexpected error, send an error to the application.
                  if (listener)
                    listener->onError(returnCode);
                  return;
                }
                //here we will check how much time we have left per buffer
                UINT32 denom = (numFullBuffers - 1 - bufferReadIndex);

                if ( timeLeftToPlay <= 0 )
                {
                  nextReadDelay = 0;
                }
                else
                {
                  nextReadDelay = denom != 0 ? 
                    timeLeftToPlay / (numFullBuffers - 1 - bufferReadIndex) : 
                    timeLeftToPlay;
                }
              }

              //need to read some more... later
              MediaFileReaderImplProxy proxy(rootProxy);
              if (!proxy)
              {
                UAPI_ERROR(fn,"Could not create MediaFileReaderImplProxy: %s\n", ReturnCode::toString(returnCode));
                if (listener)
                  listener->onError(ReturnCode::INVALID_STATE);
                return;
              }
              pendingTask = new ReadFileTask(proxy, nextReadDelay);
              if (!pendingTask)
              {
                UAPI_ERROR(fn,"Could not create ReadFileTask\n");
                if (listener)
                  listener->onError(ReturnCode::OUT_OF_MEMORY);
                return;
              }
              workerQueue->enqueue(pendingTask, returnCode);
              if (returnCode != ReturnCode::SUCCESS)
              {
                UAPI_ERROR(fn,"Failed to enqueue ReadFileTask\n");
                if (listener)
                  listener->onError(returnCode);
                delete pendingTask;
                pendingTask = 0;
                return;
              }
            }
          }//while
        }
        
        void MediaFileReaderImpl::stop(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("MediaFileReader::stop");
            
          MediaFileReaderImplProxy proxy(rootProxy);
          if (!proxy)
          {
            UAPI_ERROR(fn,"Could not create MediaFileReaderImplProxy: %s\n", ReturnCode::toString(returnCode));
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          StopReadingFileTask* task = new StopReadingFileTask(proxy);
          if (!task)
          {
            UAPI_ERROR(fn,"Could not create ReadFileTask\n");
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          workerQueue->enqueue(task, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Failed to enqueue StopReadingFileTask\n");
            delete task;
            return;
          }
          returnCode = ReturnCode::SUCCESS;
        }
        
        void MediaFileReaderImpl::runStopReadingFileTask()
        {
          UAPI_FN_SCOPE("MediaFileReader::runStopReadingFileTask");
            
          //check the state.
          if (state == IDLE)
          {
            //already stopped. Do nothing.
            UAPI_INFO(fn,"MediaFileReader is already stopped.\n");
            return;
          }
          else if (state != READING)
          {
            //stop called multiple times, do nothing
            UAPI_WARN(fn,"MediaFileReaderImpl::stop was already called.\n");
            return;
          }
          
          //it will get stopped the next time runReadFileTask is called.
          state = STOPPING;
          ReturnCode::Type returnCode;

          if (pendingTask)
          {
            // If there any pending task then remove them
            workerQueue->remove(pendingTask, returnCode);
            if (returnCode != ReturnCode::SUCCESS && listener)
              listener->onError(returnCode);
            pendingTask = 0;
          }

          //we need to submit a buffer with the last buffer flag set to true.
          audioQueue->addBuffer(0, 0, true, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Failed to call AddBuffer buffer on last buffer ret: %s\n", ReturnCode::toString(returnCode));
            listener->onError(returnCode);
            onReadFileDone();
            return;
          }

          // Add to fix bug 4700 MediaFileReader lockup
          //notify the listener that we are done with the stop
          onReadFileDone();
        }
        
        
        AudioStreamProxy MediaFileReaderImpl::createAudio(ReturnCode::Type& returnCode)
        {
          return AudioStreamImpl::create(audioQueue, returnCode);
        }
        
        void MediaFileReaderImpl::onReadFileDone()
        {
          UAPI_FN_SCOPE("MediaFileReaderImpl::onReadFileDone");
            
          //we want to allow a user to re-use the MediaFileReader object. To do so,
          //we must create a new AudioQueue for each start() operation. To make sure
          //we are ready for the next operation, we get rid of the old one here and
          //we create a new one. NOTE that we do this before we send the callback.
          if (audioQueue != 0)
          {
            ReturnCode::Type returnCode;
            
            //NOTE: we don't call delete audioQueue. This class is ref counted.
            //Instead, we call Release on it and once no one is using the object, it
            //will get released.
            audioQueue->removeRef(returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
              UAPI_ERROR(fn,"Failed to release audio buffer\n");
              if (listener)
                listener->onError(returnCode);
              return;
            }
            
            audioQueue = AudioQueue::create(codec, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
              UAPI_ERROR(fn,"Failed to create AudioQueue\n");
              if (listener)
                listener->onError(returnCode);
              return;
            }
          }
          
          state = IDLE;
          
          // Adding this to fix fopen error
          // now we are sure to close the file
          if (file)
          {
            ReturnCode::Type temp;
            file->close(temp);
            if (temp)
                UAPI_WARN(fn,"onReadFileDone::Failed to close file\n");
          }

          if (listener)
            listener->onStopped();
        }
        
        UINT32 MediaFileReaderImpl::getFileLength(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("MediaFileReaderImpl::getFileLength");
            
          // get length of file:
          UINT32 fileLength = 0;
          file->seek(0, File::UAPI_SEEK_END, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could seek to the end of the file reason %s\n", ReturnCode::toString(returnCode));
            return 0;
          }
          
          file->getPosition(fileLength, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not get the file length reason %s\n", ReturnCode::toString(returnCode));
            return 0;
          }
          return fileLength;
        }
        
        ReadFileTask::ReadFileTask(MediaFileReaderImplProxy& _mediaFileReader, UINT32 timeout):
            ScheduledTask(timeout, "ReadFileTask"),
            mediaFileReader(_mediaFileReader)
        {}
        
        void ReadFileTask::run()
        {
          mediaFileReader->runReadFileTask();
        }
        
        StartReadingFileTask::StartReadingFileTask(MediaFileReaderImplProxy& _mediaFileReader):
            Task("StartReadingFileTask"),
            mediaFileReader(_mediaFileReader)
        {}
        
        void StartReadingFileTask::run()
        {
          mediaFileReader->runStartReadingFileTask();
        }
        
        StopReadingFileTask::StopReadingFileTask(MediaFileReaderImplProxy& _mediaFileReader):
            Task("StopReadingFileTask"),
            mediaFileReader(_mediaFileReader)
        {}
        
        void StopReadingFileTask::run()
        {
          mediaFileReader->runStopReadingFileTask();
        }
      }
    }
  }
}
