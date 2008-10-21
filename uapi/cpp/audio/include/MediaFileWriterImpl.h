/*---------------------------------------------------------------------------*
 *  MediaFileWriterImpl.h                                                    *
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

#ifndef __UAPI__MEDIAFILEWRITERIMPL_
#define __UAPI__MEDIAFILEWRITERIMPL_

#include "exports.h"
#include "MediaFileWriter.h"
#include "Task.h"
#include "AudioStreamImpl.h"
#include "MediaFileWriterListener.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class File;
        class WorkerQueue;
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
        class MediaFileWriterImplProxy;
        /**
         * Class responsible for saving a collection of AudioBuffer(s) into an audio
         * file on disk. AudioBuffer are read from an Audio class.
         */
        class MediaFileWriterImpl: public MediaFileWriter
        {
          public:
            /**
             * Call this function to save the data contained into an audio source.
             * MediaFileWriterListener::onStopped() will be called once the operation
             * completes.
             * @param source the audio source that contains the samples.
             * @param fileName the filename to save the audio to.
             * @param returnCode the return code
             */
            virtual void save(AudioStreamProxy& source, const char* filename, ReturnCode::Type& returnCode);
            
          private:
            enum State
            {
              IDLE,
              SAVING
            };
            
            /**
             * Prevent assignment.
             */
            MediaFileWriterImpl& operator=(MediaFileWriterImpl&);
            /**
             * Prevent destruction.
             */
            virtual ~MediaFileWriterImpl();
            
            /**
             * constructor
             * @param listener the listener that will receive the event when the save
             * completes.
             */
            MediaFileWriterImpl(MediaFileWriterListenerProxy& listener);
            
            /**
             * internally called by createMediaFileWriterImpl. Initialize some components.
             * @param pszFileName the name of the file in which the audio samples
             * will get saved.
             * @param returnCode the return code
             */
            void init(const char* pszFileName, ReturnCode::Type& returnCode);
            
            /**
             * Called when the StartWriteFileTask is dequeued and executed.
             */
            void runStartWriteFileTask(impl::AudioStreamImplProxy& audioStream);
            
            /**
             * Called when the WriteFileTask is dequeued and executed.
             */
            void runWriteFileTask(impl::AudioStreamImplProxy& audioStream);
            
            /**
             * Utility method to do cleanup and send the OnError callback to the
             * application.
             */
            void sendOnErrorToListener(impl::AudioStreamImplProxy& audioStream, ReturnCode::Type error);
            
            /**
             * Utility method to do cleanup and send the OnStopped callback to the
             * application.
             */
            void sendOnStoppedToListener(impl::AudioStreamImplProxy& audioStream);
            
            /**
             * Utility function responsible for cleanup up the AudioStreamImplProxy code.
             */
            void cleanupAudioStreamProxy(impl::AudioStreamImplProxy& audioStream, ReturnCode::Type& returnCode);
            
            /**
            * Close the file
            */
            void closeFile(ReturnCode::Type& returnCode);

            /**
             * File in which we will save the data
             */
            utilities::File* file;
            /**
             * Worker queue used to run Tasks
             */
            utilities::WorkerQueue* workerQueue;
            /**
             * The listener for MediaFileWriter events
             */
            MediaFileWriterListenerProxy listener;
            
            /**
             * state of the resource
             */
            State state;
            
            SmartProxy::Root* rootProxy;
            /**
             * true if the RIFF header was saved in the file.
             */
            bool bHeaderSaved;
            
            friend class StartWriteFileTask;
            friend class WriteFileTask;
            friend class MediaFileWriter;
            friend class MediaFileWriterImplProxy;
        };
        
        /**
         * @see android::speech::recognition::SmartProxy
         */
        DECLARE_SMARTPROXY(UAPI_EXPORT, MediaFileWriterImplProxy, MediaFileWriterProxy, MediaFileWriterImpl)
        
        /**
         * Task that takes care of executing the "save" operation
         */
        class StartWriteFileTask: public utilities::Task
        {
          public:
            StartWriteFileTask(MediaFileWriterImplProxy& mediaFileWriter,
                               AudioStreamImplProxy& audioStream);
            virtual void run();
          private:
            MediaFileWriterImplProxy mediaFileWriter;
            AudioStreamImplProxy audioStream;
        };
        
        /**
         * Task that takes care of writing the audio samples into a file.
         */
        class WriteFileTask: public utilities::ScheduledTask
        {
          public:
            WriteFileTask(MediaFileWriterImplProxy& mediaFileWriter, AudioStreamImplProxy& audioStream,
                          UINT32 timeout);
            virtual void run();
          private:
            MediaFileWriterImplProxy mediaFileWriter;
            AudioStreamImplProxy audioStream;
        };
      }
    }
  }
}

#endif
