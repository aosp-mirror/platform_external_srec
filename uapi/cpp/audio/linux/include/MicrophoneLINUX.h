/*---------------------------------------------------------------------------*
 *  MicrophoneLINUX.h                                                        *
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

#ifndef __UAPI__MICROPHONELINUX_H
#define __UAPI__MICROPHONELINUX_H

#include "exports.h"
#include "Microphone.h"
#include "MicrophoneListener.h"
#include "Task.h"
#include "Codec.h"
#include "audioin.h"

/* define these so code can look more like Windows version (for ease of support) */
#define CALLBACK
typedef AUDIOIN_WAVEHDR WAVEHDR;
typedef AUDIOIN_H       HWAVEIN;
typedef void*           DWORD_PTR;

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class AudioStream;
      namespace utilities
      {
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
        void CALLBACK __waveInProc(HWAVEIN, AUDIOIN_MSG, DWORD_PTR, DWORD_PTR, DWORD_PTR);
        /**
         * Microphone (audio acquisition)
         */
        class MicrophoneLINUX: public Microphone
        {
          public:
            //-------------------------------------------------------------
            //  Microphone IMPLEMENTATION
            //-------------------------------------------------------------
            
            /**
             * Tells the microphone to start collecting audio samples.
             *
             * @param returnCode the return code
             */
            virtual void start(ReturnCode::Type& returnCode);
            
            /**
             * Stops the microphone from collecting audio samples.
             *
             * @param returnCode the return code
             */
            virtual void stop(ReturnCode::Type& returnCode);
            
            /**
             * Returns an object that contains the audio samples. It can be passed to
             * a Recognizer a DeviceSpeaker or a MediaFileWriter. Internally a position
             * is associated with Audio object. This means that if someone wants to do
             * two simultaneous recognitions on the same audio stream, then two Audio
             * objects should be created. After createAudio is called, the application
             * is now responsible for releasing the memory. It should do so only once
             * no other resources are using it. For example, if we do a createAudio on a
             * Microphone, and we pass that audio to a Recognizer, then the audio
             * should be deleted only once the Microphone was stopped and once the
             * recognition is completed.
             *
             * @param returnCode the return code.
             * @return a pointer to an Audio object.
             */
            virtual AudioStreamProxy createAudio(ReturnCode::Type& returnCode);
            
            /**
             * set the recording codec. This must be called before Start is called.
             * @param recordingCodec the codec in which the samples will be recorded.
             * @param returnCode the return code.
             */
            virtual void setCodec(Codec::Type recordingCodec, ReturnCode::Type& returnCode);
            
            /**
             * Sets the microphone listener.
             * @param listener the microphone listener.
             * @param returnCode the return code.
             */
            virtual void setListener(AudioSourceListenerProxy& listener, ReturnCode::Type& returnCode);
          private:
            /**
             * Initializes the component when the library is loaded.
             */
            class ComponentInitializer
            {
              public:
                ComponentInitializer();
                ~ComponentInitializer();
                
                ReturnCode::Type returnCode;
            };
            
            //Singleton implementation
            /**
             * Returns the root of this proxy, i.e. the "real object".
             *
             * @return Root of the proxy, the "real object".
             */
            virtual SmartProxy::Root* getRoot();
            
            /**
             * Invoked when the singleton has to shutdown.
             *
             * @param returnCode the return code.
             */
            virtual void shutdown(ReturnCode::Type& returnCode);
            
            /**
             * Creates a new Microphone.
             *
             * @param returnCode the return code
             */
            MicrophoneLINUX(ReturnCode::Type& returnCode);
            /**
             * Prevent destruction.
             */
            virtual ~MicrophoneLINUX();
            
            enum State
            {
              IDLE,
              STARTING,
              RECORDING,
              STOPPING,
              ERROR,
              ERROR_STOPPING,
              ERROR_STOPPED
            };
            
            /**
             * Initialize this object. Internally called by createMicrophoneLINUX.
             * @param returnCode the return code
             */
            void init(ReturnCode::Type& returnCode);
            
            /**
             * Task executed when setListener() is called.
             * @param _listener the proxy to the a microphone listener.
             */
            void runSetMicrophoneListenerTask(AudioSourceListenerProxy& _listener);
            
            /**
             * Task that is executed when start() is called. It will start the
             * recorder.
             */
            void runStartMicrophoneTask();
            
            /**
             * Task that handles the things to do when the recording is successfully
             * started.
             */
            void runMicrophoneStartedTask();
            
            /**
             * Task invoked every time a buffer is recorded
             * @param pwhdr a pointer to the WAVEHDR object that contains the recorded
             * samples.
             */
            void runBufferRecordedTask(WAVEHDR* pwhdr);
            
            /**
             * Task that is executed when stop() is called. It will stop the
             * recorder.
             */
            void runStopMicrophoneTask();
            
            /**
             * Task that handles the things to do when the recording is successfully
             * stopped.
             */
            void runMicrophoneStoppedTask();
            
            /**
             * Utility method to do cleanup and send the OnError callback to the
             * application.
             */
            void sendOnErrorToListener(ReturnCode::Type error, bool stopRecording);
            
            /**
             * stop() has not completed. This function is used to reset the AudioQueue
             * and send the onStopped() callback to the listener.
             */
            void onStopDone();
            
            static ComponentInitializer componentInitializer;
            
            /**
             * Worker queue used to run tasks.
             */
            utilities::WorkerQueue* m_pWorkerQueue;
            /**
             * The listener of Microphone events.
             */
            AudioSourceListenerProxy listener;
            /**
             * The audio format in which we have to record.
             */
            Codec::Type m_codec;
            /**
             * AudioQueue in which we will store (AddBuffer) the audio samples.
             */
            utilities::AudioQueue* m_pAudioQueue;
            /**
             * a WAVEIN handle. Needed to call any waveIn functions.
             */
            HWAVEIN m_hwi;
            /**
             * sampling rate
             */
            unsigned int m_nSamplesPerSec;
            
            /**
             * current state
             */
            State m_state;
            static utilities::Mutex* mutex;
            
            
            SmartProxy::Root* rootProxy;
            
            friend void CALLBACK __waveInProc(HWAVEIN, AUDIOIN_MSG, DWORD_PTR, DWORD_PTR, DWORD_PTR);
            friend class SetMicrophoneListenerTask;
            friend class StartMicrophoneTask;
            friend class MicrophoneStartedTask;
            friend class BufferRecordedTask;
            friend class StopMicrophoneTask;
            friend class MicrophoneStoppedTask;
            friend class Microphone;
            friend class MicrophoneLINUXProxy;
        };
        
        /*
         * @see android::speech::recognition::SmartProxy
         */
        DECLARE_SMARTPROXY(UAPI_EXPORT, MicrophoneLINUXProxy, MicrophoneProxy, MicrophoneLINUX)
        
        /**
         * Task executed when setListener() is called.
         */
        class SetMicrophoneListenerTask: public utilities::Task
        {
          public:
            SetMicrophoneListenerTask(MicrophoneLINUXProxy& _microphone,
                                      AudioSourceListenerProxy & _listener);
            virtual void run();
          private:
            MicrophoneLINUXProxy     microphone;
            AudioSourceListenerProxy  listener;
        };
        
        /**
         * Task that is executed when start() is called. It will start the recorder.
         */
        class StartMicrophoneTask: public utilities::Task
        {
          public:
            StartMicrophoneTask(MicrophoneLINUXProxy& microphone);
            virtual void run();
          private:
            MicrophoneLINUXProxy microphone;
        };
        
        
        /**
         * Task that handles the things to do when the recording is successfully
         * started.
         */
        class MicrophoneStartedTask: public utilities::Task
        {
          public:
            MicrophoneStartedTask(MicrophoneLINUXProxy& microphone);
            virtual void run();
          private:
            MicrophoneLINUXProxy microphone;
        };
        
        /**
         * Task invoked every time a buffer is recorded
         */
        class BufferRecordedTask: public utilities::Task
        {
          public:
            BufferRecordedTask(MicrophoneLINUXProxy& microphone, WAVEHDR* pwhdr);
            virtual ~BufferRecordedTask();
            virtual void run();
          private:
            MicrophoneLINUXProxy microphone;
            WAVEHDR* pwhdr;
        };
        
        /**
         * Task that is executed when stop() is called. It will start the recorder.
         */
        class StopMicrophoneTask: public utilities::Task
        {
          public:
            StopMicrophoneTask(MicrophoneLINUXProxy& microphone);
            virtual void run();
          private:
            MicrophoneLINUXProxy microphone;
        };
        
        /**
         * Task that handles the things to do when the recording is successfully
         * stopped.
         */
        class MicrophoneStoppedTask: public utilities::Task
        {
          public:
            MicrophoneStoppedTask(MicrophoneLINUXProxy& _microphone);
            virtual void run();
          private:
            MicrophoneLINUXProxy microphone;
        };
      }
    }
  }
}

#endif
