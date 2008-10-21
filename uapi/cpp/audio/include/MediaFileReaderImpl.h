/*---------------------------------------------------------------------------*
 *  MediaFileReaderImpl.h                                                    *
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

#ifndef __UAPI__MEDIAFILEREADERIMPL_
#define __UAPI__MEDIAFILEREADERIMPL_

#include "exports.h"
#include "types.h"
#include "MediaFileReader.h"
#include "MediaFileReaderListener.h"
#include "Task.h"
#include "Codec.h"
#include "TimeInstant.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class AudioStreamProxy;
      namespace utilities
      {
        class File;
        class WorkerQueue;
        class AudioQueue;
      }
    }
  }
}


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace impl
      {
        class MediaFileReaderImplProxy;
        /**
         * Class responsible for reading a file from disk, read its content and store
         * that into and AudioQueue as AudioBuffer(s).
         */
        class MediaFileReaderImpl: public MediaFileReader
        {
          public:
            //----------------------------------------------------------------
            //          MediaFileReader implementation
            //----------------------------------------------------------------
            
            /**
             * Set the reading mode
             *
             * @param mode the reading mode
             * @param returnCode the return code
             */
            virtual void setReadingMode(ReadingMode mode, ReturnCode::Type& returnCode);
            
            /**
             * Tells the audio source to start collecting audio samples.
             *
             * @param returnCode the return code
             */
            virtual void start(ReturnCode::Type& returnCode);
            
            /**
             * Stops this source from collecting audio samples.
             *
             * @param returnCode the return code
             */
            virtual void stop(ReturnCode::Type& returnCode);
            
            /**
             * Returns an object that contains the audio samples. It can be passed to
             * a Recognizer or a DeviceSpeaker. Internally a position is associated
             * with Audio object. This means that if someone wants to do two
             * simultaneous recognitions on the same audio stream, then two Audio
             * objects should be created. After crateAudio was called, the application
             * is now responsible for releasing the memory. It should do so only once
             * no other resource are using it. For example, if we do a createAudio on a
             * Microphone, and we pass that audio to a recognizer, then the audio
             * should be deleted only once the Microphone was stopped and once the
             * recognition is completed.
             *
             * @param returnCode the return code.
             * @return a pointer to an Audio object.
             */
            virtual AudioStreamProxy createAudio(ReturnCode::Type& returnCode);
            
          private:
           /**
            * Creates an instance of this class.
            *
            * @param pszFileName the name of the file in which we will read the
            * audio samples. Note: The file MUST be of type Microsoft WAVE RIFF
            * format (PCM 16 bits 8000 Hz or PCM 16 bits 11025 Hz).
            * @param listener listens for MediaFileReader events
            * @param logger the logger
            * @param returnCode the return code
            */
            MediaFileReaderImpl(const char* pszFileName,
                                AudioSourceListenerProxy& listener,
                                ReturnCode::Type& returnCode);
            /**
             * Prevent destruction.
             */
            virtual ~MediaFileReaderImpl();
            
            /**
             * Called when done reading the file or after Stop is done
             */
            void onReadFileDone();
            
            /**
             * Called when the ReadFileTask is dequeued and executed
             */
            void runReadFileTask();
            
            /**
             * Called when the StartReadingFileTask is dequeued and executed
             */
            void runStartReadingFileTask();
            
            /**
             * Called when the StopReadingFileTask is dequeued and executed
             */
            void runStopReadingFileTask();
            
          private:
            enum State
            {
              IDLE,
              READING,
              STOPPING
            };
            
            /**
             * Prevent assignment.
             */
            MediaFileReaderImpl& operator=(MediaFileReaderImpl&);
            
            /**
             * returns the length of the file
             * @param returnCode the return code.
             * @return the lenght of the file.
             */
            UINT32 getFileLength(ReturnCode::Type& returnCode);
            
            /**
             * File that contains the data to read
             */
            utilities::File* file;
            /**
             * The mode, REAL_TIME or ALL_AT_ONCE
             */
            ReadingMode mode;
            /**
             * Header size of the file. Ex: 44 K for RIFF, 1024 for SPHERE
             */
            UINT16 headerOffset;
            /**
             * The listener of MediaFileReader events
             */
            AudioSourceListenerProxy listener;
            /**
             * codec contained in the file to read
             */
            Codec::Type codec;
            /**
             * Worker queue used run Tasks
             */
            utilities::WorkerQueue* workerQueue;
            /**
             * The queue in which we insert the read audio samples.
             */
            utilities::AudioQueue* audioQueue;
            /**
             * Number of "full" buffers we have to read
             */
            UINT32 numFullBuffers;
            /**
             * Number of bytes in the last buffer to read
             */
            UINT16 lastBuffNumBytes;
            /**
             * The current buffer we have to read
             */
            UINT32 bufferReadIndex;
            /**
             * current state
             */
            State state;
            /**
             * If Recognizer.stop() is invoked we need to cancel any pending tasks related to the
             * current recognition.
             */
            android::speech::recognition::utilities::Task* pendingTask;
           
            /**
             * time at which we should be done playing this file
             */
            android::speech::recognition::utilities::TimeInstant expireTime;
           
            /**
             * Number of buffers we will read if we are late reading the audio in real time mode.
             */
            UINT32 _numBufferNeededToCatchUp;
            
            SmartProxy::Root* rootProxy;
            
            friend class ReadFileTask;
            friend class StartReadingFileTask;
            friend class StopReadingFileTask;
            friend class MediaFileReader;
            friend class MediaFileReaderImplProxy;
        };
        
        /*
         * @see android::speech::recognition::SmartProxy
         */
        DECLARE_SMARTPROXY(UAPI_EXPORT, MediaFileReaderImplProxy, MediaFileReaderProxy, MediaFileReaderImpl)
        
        /**
         * Task responsible for reading the file and adding the file content into the
         * AudioQueue.
         */
        class ReadFileTask: public utilities::ScheduledTask
        {
          public:
            ReadFileTask(MediaFileReaderImplProxy& mediaFileReader, UINT32 timeout);
            virtual void run();
          private:
            MediaFileReaderImplProxy mediaFileReader;
        };
        
        /**
         * Task responsible for starting reading process.
         */
        class StartReadingFileTask: public utilities::Task
        {
          public:
            StartReadingFileTask(MediaFileReaderImplProxy& mediaFileReader);
            virtual void run();
          private:
            MediaFileReaderImplProxy mediaFileReader;
        };
        
        /**
         * Task responsible for stopping the reading process.
         */
        class StopReadingFileTask: public utilities::Task
        {
          public:
            StopReadingFileTask(MediaFileReaderImplProxy& mediaFileReader);
            virtual void run();
          private:
            MediaFileReaderImplProxy mediaFileReader;
        };
      }
    }
  }
}

#endif
