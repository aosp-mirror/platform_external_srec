/*---------------------------------------------------------------------------*
 *  MicrophoneLINUX.cpp                                                      *
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

#include "MicrophoneLINUX.h"
#include "MicrophoneListener.h"
#include "WorkerQueueFactory.h"
#include "WorkerQueue.h"
#include "CodecHelper.h"
#include "AudioStreamImpl.h"
#include "AudioQueue.h"
#include "LoggerImpl.h"
#include "Codec.h"
#include "System.h"
#include "Runnable.h"

using namespace android::speech::recognition::utilities;


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      Microphone* Microphone::instance = 0;
      
      MicrophoneProxy Microphone::getInstance(ReturnCode::Type& returnCode)
      {
        UAPI_FN_SCOPE("Microphone::getInstance");
          
        if (impl::MicrophoneLINUX::componentInitializer.returnCode)
        {
          returnCode = impl::MicrophoneLINUX::componentInitializer.returnCode;
          return MicrophoneProxy();
        }
        //we have to protect the construction of "instance". Make sure we don't have
        //multiple threads calling getInstance() while "instance" is equal to 0.
        LockScope lock(impl::MicrophoneLINUX::mutex, returnCode);

        if (returnCode != ReturnCode::SUCCESS)
        {
          UAPI_ERROR(fn,"Failed to create LockScope of MicrophoneLINUX::mutex\n");
          return MicrophoneProxy();
        }
        //it's now safe to check if instance == 0
        if (instance == 0)
        {
          instance = new impl::MicrophoneLINUX(returnCode);
          if (returnCode)
          {
            delete instance;
            instance = 0;
            return MicrophoneProxy();
          }
          if (instance == 0)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return MicrophoneProxy();
          }
          impl::MicrophoneLINUX* instanceImpl = (impl::MicrophoneLINUX*) instance;
          MicrophoneProxy result(instanceImpl);
          if (!result)
          {
            delete instance;
            instance = 0;
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return MicrophoneProxy();
          }
          instanceImpl->rootProxy = result.getRoot();
          
          //register for auto cleanup.
          System* system = System::getInstance(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            delete instance;
            instance = 0;
            return MicrophoneProxy();
          }
          
          //has to be called after rootProxy is assigned.
          system->add(instance, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            delete instance;
            instance = 0;
            return MicrophoneProxy();
          }
          return result;
        }
        impl::MicrophoneLINUX* instanceImpl = (impl::MicrophoneLINUX*) instance;
        return MicrophoneProxy(instanceImpl->rootProxy);
      }



      namespace impl
      {
        DEFINE_SMARTPROXY(impl, MicrophoneLINUXProxy, MicrophoneProxy, MicrophoneLINUX);
        
        Mutex* MicrophoneLINUX::mutex = 0;
        MicrophoneLINUX::ComponentInitializer MicrophoneLINUX::componentInitializer;
        
        MicrophoneLINUX::ComponentInitializer::ComponentInitializer()
        {
          mutex = Mutex::create(false, returnCode);
          if (returnCode)
          {
            fprintf(stderr, "Could not create MicrophoneLINUX::mutex\n");
            return;
          }
        }

        struct WaveInHelperStruct
        {
          WorkerQueue* workerQueue;
          MicrophoneLINUXProxy microphone;
        };

        MicrophoneLINUX::ComponentInitializer::~ComponentInitializer()
        {
          delete mutex;
          returnCode = ReturnCode::UNKNOWN;
        }


        
        void CALLBACK __waveInProc(HWAVEIN, AUDIOIN_MSG uMsg, DWORD_PTR dwInstance,
                                   DWORD_PTR dwParam1, DWORD_PTR)
        {
          ReturnCode::Type returnCode;
          
          WaveInHelperStruct* helper = ((WaveInHelperStruct*) dwInstance);
          MicrophoneLINUXProxy& microphone  = helper->microphone;

          UAPI_FN_SCOPE("Microphone::__waveInProc");

          //NOTE: we are not allowed to lock any mutex in this functions. This is
          //part of the contract when using the waveIn api.

          WorkerQueue* workerQueue = helper->workerQueue;

          switch (uMsg)
          {
            case AUDIOIN_MSG_OPEN:
              UAPI_INFO(fn,"AUDIOIN_MSG_OPEN\n");
              break;

            case AUDIOIN_MSG_START:
            {
              UAPI_INFO(fn,"AUDIOIN_MSG_START\n");
              MicrophoneStartedTask* task = new MicrophoneStartedTask(microphone);
              if (!task)
              {
                microphone->m_state = MicrophoneLINUX::ERROR;
                UAPI_ERROR(fn,"Could not create MicrophoneStartedTask\n");
                microphone->sendOnErrorToListener(ReturnCode::OUT_OF_MEMORY,true);
                delete helper;
                return;
              }
              workerQueue->enqueue(task, returnCode);
              if (returnCode)
              {
                microphone->m_state = MicrophoneLINUX::ERROR;
                UAPI_ERROR(fn,"Failed to enqueue MicrophoneStartedTask: %d\n",
                           ReturnCode::Type(returnCode));
                microphone->sendOnErrorToListener(returnCode,true);
                delete helper;
                delete task;
                return;
              }
              UAPI_INFO(fn,"enqueue(new MicrophoneStartedTask)\n");
              break;
            }

            case AUDIOIN_MSG_DATA:
              UAPI_INFO(fn,"AUDIOIN_MSG_DATA\n");
// Rather than put the data in to a task and then into a worker queue, it seems better
// to just put it into the audio queue directly. The only danger will be doing more in
// the callback, but I don't believe that will be an issue. SteveR
// I have left the old code in so that it can be easily restored if needed.
// It should be deleted once this is in production. SteveR
              microphone->runBufferRecordedTask ( (WAVEHDR*) dwParam1);
              break;

            case AUDIOIN_MSG_STOP:
              UAPI_INFO(fn,"AUDIOIN_MSG_STOP\n");
              break;

            case AUDIOIN_MSG_CLOSE:
            {
              UAPI_INFO(fn,"AUDIOIN_MSG_CLOSE\n");
              MicrophoneStoppedTask* task = new MicrophoneStoppedTask(microphone);
              if (!task)
              {
                microphone->m_state = MicrophoneLINUX::ERROR;
                UAPI_ERROR(fn,"Could not create MicrophoneStoppedTask\n");
                microphone->sendOnErrorToListener(ReturnCode::OUT_OF_MEMORY,true);
                delete helper;
                return;
              }
              workerQueue->enqueue(task, returnCode);
              if (returnCode)
              {
                UAPI_ERROR(fn,"Failed to enqueue MicrophoneStoppedTask: %d\n", ReturnCode::Type(returnCode));
                microphone->m_state = MicrophoneLINUX::ERROR;
                microphone->sendOnErrorToListener(returnCode,true);
                delete helper;
                delete task;
                return;
              }
              UAPI_INFO(fn,"enqueue(new MicrophoneStoppedTask)\n");
              delete helper;
              break;
            }

            default:
              UAPI_ERROR(fn,"Invalid audioin message %d\n", uMsg);
              break;
          }
        }



        MicrophoneLINUX::MicrophoneLINUX(ReturnCode::Type& returnCode):
            m_pWorkerQueue(0),
            m_codec(Codec::PCM_16BIT_11K),
            m_pAudioQueue(0),
            m_hwi(0),
            m_nSamplesPerSec(0),
            m_state(IDLE),
            rootProxy(0)
        {
          UAPI_FN_SCOPE("MicrophoneLINUX::MicrophoneLINUX");
          UAPI_TRACE(fn,"this=%p\n", this);
          
          m_nSamplesPerSec = Codec::getSampleRate(m_codec, returnCode);
          if (returnCode)
            return;
          init(returnCode);
        }



        void MicrophoneLINUX::setCodec(Codec::Type recordingCodec, ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("MicrophoneLINUX::setCodec");
   
          //need to protect when we are assigning m_state (done in WorkerQueue
          //thread) and when we are calling SetCodec (done in the app
          //thread).
          LockScope ls(mutex, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Failed to create LockScope\n");
            return;
          }

          if (m_state != IDLE)
          {
            UAPI_ERROR(fn,"Must be in IDLE state to call setCodec\n");
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          m_codec = recordingCodec;
          {
            ReturnCode::Type rc;
            m_nSamplesPerSec = Codec::getSampleRate(m_codec, rc);
            if (rc)
            {
              returnCode = rc;
              return;
            }
          }
          //NOTE: we don't call delete m_pAudioQueue. This class is ref counted.
          //Instead, we call Release on it and once no one is using the object, it
          //will get released.
          m_pAudioQueue->removeRef(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not release ref count on AudioQueue\n");
            return;
          }
          m_pAudioQueue = 0;
          m_pAudioQueue = AudioQueue::create(m_codec, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            m_pAudioQueue = 0;
            UAPI_ERROR(fn,"Could not create AudioQueue\n");
            return;
          }
          returnCode = ReturnCode::SUCCESS;
        }



        void MicrophoneLINUX::setListener(AudioSourceListenerProxy& _listener,
                                          ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("MicrophoneLINUX::setListener");

          if (!_listener)
          {
            //not accepting NULL listener
            UAPI_WARN(fn,"Listener cannot be NULL\n");
            returnCode = ReturnCode::ILLEGAL_ARGUMENT;
            return;
          }
          MicrophoneLINUXProxy proxy(rootProxy);
          if (!proxy)
          {
            UAPI_ERROR(fn,"Could not create proxy\n");
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          SetMicrophoneListenerTask* task = new SetMicrophoneListenerTask(proxy, _listener);

          if (!task)
          {
            UAPI_ERROR(fn,"Could not create SetMicrophoneListenerTask\n");
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          m_pWorkerQueue->enqueue(task, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            delete task;
            UAPI_ERROR(fn,"Failed to enqueue SetMicrophoneListenerTask\n");
            return;
          }
          returnCode = ReturnCode::SUCCESS;
        }



        void MicrophoneLINUX::runSetMicrophoneListenerTask(AudioSourceListenerProxy& _listener)
        {
          UAPI_FN_SCOPE("MicrophoneLINUX::runSetMicrophoneListenerTask");

          listener = _listener;
        }



        void MicrophoneLINUX::shutdown(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("MicrophoneLINUX::shutdown");

          stop(returnCode);
        }



        void MicrophoneLINUX::init(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("MicrophoneLINUX::init");
            
          m_pAudioQueue = AudioQueue::create(m_codec, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not create AudioQueue\n");
            return;
          }
          WorkerQueueFactory* workerFactory = WorkerQueueFactory::getInstance(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not create the worker queue factory\n");
            return;
          }
          m_pWorkerQueue = workerFactory->getWorkerQueue(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not create the worker queue\n");
            return;
          }
          returnCode = ReturnCode::SUCCESS;
        }



        MicrophoneLINUX::~MicrophoneLINUX()
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("MicrophoneLINUX::~MicrophoneLINUX");
          UAPI_TRACE(fn,"this=%p\n", this);

          {
            LockScope ls(mutex, returnCode);

            if (returnCode)
            {
              UAPI_ERROR(fn,"Failed to create LockScope\n");
            }
            if ( ( m_state != IDLE ) && ( m_state != ERROR_STOPPED ) )
              UAPI_WARN(fn,"Microphone Did Not Shut Down Cleanly\n");
          }
          if (m_pAudioQueue != 0)
          {
            //NOTE: we don't call delete m_pAudioQueue. This class is ref counted.
            //Instead, we call Release on it and once no one is using the object, it
            //will get released. The object is created by the MicrophoneLINUX class, but
            //it is shared with the AudioStreamImpl class.
            ReturnCode::Type rc;
            m_pAudioQueue->removeRef(rc);
            if (rc != ReturnCode::SUCCESS)
              UAPI_ERROR(fn,"Could not release ref count on AudioQueue\n");
          }
          if (m_hwi != 0)
          {
            //Stop was not called or playback did not complete normally, do our best to
            //cleanup.
            LHS_AUDIOIN_ERROR lhsErr;

            lhsErr = lhs_audioinStop(m_hwi);
            if (lhsErr != LHS_AUDIOIN_OK)
            {
              UAPI_ERROR(fn,"lhs_audioinStop failed %ld\n", lhsErr);
            }
            lhsErr = lhs_audioinClose(&m_hwi);
            if (lhsErr != LHS_AUDIOIN_OK)
            {
              UAPI_ERROR(fn,"lhs_audioinClose failed %ld\n", lhsErr);
            }
            m_hwi = 0;
          }
          instance = 0;
        }



        void MicrophoneLINUX::start(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("MicrophoneLINUX::start");
            
          if (m_pAudioQueue == 0 || m_pWorkerQueue == 0)
          {
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          MicrophoneLINUXProxy proxy(rootProxy);
          if (!proxy)
          {
            UAPI_ERROR(fn,"Could not create proxy\n");
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          StartMicrophoneTask* task = new StartMicrophoneTask(proxy);
          if (!task)
          {
            UAPI_ERROR(fn,"Could not create StartMicrophoneTask\n");
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          m_pWorkerQueue->enqueue(task, returnCode);
          if (returnCode)
          {
            UAPI_ERROR(fn,"Could not enqueue StartMicrophoneTask\n");
            delete task;
            return;
          }
          UAPI_INFO(fn,"enqueue(new StartMicrophoneTask)\n");
          returnCode = ReturnCode::SUCCESS;
        }



        void MicrophoneLINUX::runStartMicrophoneTask()
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("MicrophoneLINUX::runStartMicrophoneTask");

          LockScope ls(mutex, returnCode);
          if (returnCode)
          {
            m_state = ERROR;
            UAPI_ERROR(fn,"Failed to create LockScope\n");
            sendOnErrorToListener(returnCode,true);
            return;
          }
          if (m_state != IDLE)
          {
            UAPI_WARN(fn,"Must be in IDLE state to call start\n");
            sendOnErrorToListener(ReturnCode::INVALID_STATE,true);
            return;
          }
          m_state = STARTING;
          LHS_AUDIOIN_ERROR lhsErr;
          //if everything is as expected, helper gets deleted when we get the WIM_CLOSE
          //message back.
          MicrophoneProxy microphone = Microphone::getInstance(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            m_state = ERROR;
            UAPI_ERROR(fn,"Could not get an instance of the microphone\n");
            sendOnErrorToListener(ReturnCode::INVALID_STATE,true);
            return;
          }
          WaveInHelperStruct* helper = new WaveInHelperStruct();
          if (helper == 0)
          {
            m_state = ERROR;
            UAPI_ERROR(fn,"Could not allocated memory for WaveInHelperStruct\n");
            sendOnErrorToListener(ReturnCode::OUT_OF_MEMORY,true);
            return;
          }
          helper->workerQueue = m_pWorkerQueue;
          MicrophoneLINUXProxy proxy(microphone);
          helper->microphone = proxy;
          lhsErr = lhs_audioinOpenCallback(WAVE_MAPPER, m_nSamplesPerSec, CodecHelper::GetPreferredBufferSize(m_codec) / sizeof(audioinSample), __waveInProc, helper, &m_hwi);
          if (lhsErr != LHS_AUDIOIN_OK)
          {
            m_state = ERROR;
            UAPI_ERROR(fn,"lhs_audioinOpenCallback failed %ld\n", lhsErr);
            sendOnErrorToListener(ReturnCode::AUDIO_DRIVER_ERROR,true);
            delete helper;
            return;
          }
          lhsErr = lhs_audioinStart(m_hwi);
          if (lhsErr != LHS_AUDIOIN_OK)
          {
            m_state = ERROR;
            UAPI_ERROR(fn,"lhs_audioinStart failed %ld\n", lhsErr);
            sendOnErrorToListener(ReturnCode::AUDIO_DRIVER_ERROR,true);
            delete helper;
            return;
          }
        }



        void MicrophoneLINUX::runMicrophoneStartedTask()
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("MicrophoneLINUX::runMicrophoneStartedTask");

          {
            LockScope ls(mutex, returnCode);
            if (returnCode)
            {
              m_state = ERROR;
              UAPI_ERROR(fn,"Failed to create LockScope\n");
              sendOnErrorToListener(returnCode,true);
              return;
            }
            if (m_state != STARTING)
            {
// No error state here, this seems recoverable
              UAPI_WARN(fn,"MicrophoneLINUX::Start was already called, state is %d.\n", m_state);
              return;
            }
            m_state = RECORDING;
          }
          //recording was successfully started! Send callback to the application
          if (listener)
            listener->onStarted();
        }



        void MicrophoneLINUX::runBufferRecordedTask(WAVEHDR* pwhdr)
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("MicrophoneLINUX::runBufferRecordedTask");

          //a buffer was filled.
          ARRAY_LIMIT nBytesRecorded = (ARRAY_LIMIT) pwhdr->nBytesRecorded;
          unsigned char* pbData = (unsigned char*)(pwhdr->pData);

          if (nBytesRecorded <= 0)
            pbData = 0;
          if (pbData)
          {
            m_pAudioQueue->addBuffer(pbData, nBytesRecorded, false, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
              UAPI_ERROR(fn,"Failed to call AddBuffer buffer %p, size %d ret: %s\n", pbData,
                         nBytesRecorded, ReturnCode::toString(returnCode));
              m_state = ERROR;
              sendOnErrorToListener(returnCode,true);
              return;
            }
          }
#ifdef ANDROID
// For the ANDROID platform we are no longer allocating these buffers. The correct
// solution would have the audio in module having a dispose function so that this
// "knowledge" is not exposed across modules. SteveR
#else	  
          free(pwhdr->pData);
          free(pwhdr);
#endif
        }



        void MicrophoneLINUX::stop(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("MicrophoneLINUX::stop");

          MicrophoneLINUXProxy proxy(rootProxy);
          if (!proxy)
          {
            m_state = ERROR;
            UAPI_ERROR(fn,"Could not create proxy\n");
            returnCode = ReturnCode::INVALID_STATE;
            sendOnErrorToListener(returnCode,true);
            return;
          }
          StopMicrophoneTask* task = new StopMicrophoneTask(proxy);
          if (!task)
          {
            m_state = ERROR;
            UAPI_ERROR(fn,"Could not create StartMicrophoneTask\n");
            returnCode = ReturnCode::OUT_OF_MEMORY;
            sendOnErrorToListener(returnCode,true);
            return;
          }
          m_pWorkerQueue->enqueue(task, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            m_state = ERROR;
            UAPI_ERROR(fn,"Failed to enqueue StopMicrophoneTask\n");
            delete task;
            returnCode = ReturnCode::INVALID_STATE;
            sendOnErrorToListener(returnCode,true);
            return;
          }
          UAPI_INFO(fn,"enqueue(new StopMicrophoneTask)\n");

          returnCode = ReturnCode::SUCCESS;
        }



        void MicrophoneLINUX::runStopMicrophoneTask()
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("MicrophoneLINUX::runStopMicrophoneTask");

          {
            LockScope ls(mutex, returnCode);
            if (returnCode)
            {
              m_state = ERROR;
              UAPI_ERROR(fn,"Failed to create LockScope\n");
              sendOnErrorToListener(returnCode,true);
              return;
            }
            if ( ( m_state == IDLE ) || ( m_state == ERROR_STOPPED ) )
            {
              UAPI_WARN(fn,"Microphone is already stopped, state is %d.\n", m_state );
              return;
            }
            else if ( ( m_state == STOPPING )  && ( m_state == ERROR_STOPPING ) )
            {
              UAPI_WARN(fn,"Microphone is already being stopped, state is %d.\n", m_state);
              return;
            }
            else if ( ( m_state == STARTING )  || ( m_state == RECORDING ) )
            {
              m_state = STOPPING;
            }
            else if ( m_state == ERROR )
            {
              m_state = ERROR_STOPPING;
            }
            else 	// Unknown State
            {
              UAPI_ERROR(fn,"Microphone Cannot Stop in Unknown State %d.\n", m_state);
              return;
            }
          }
          // Submit a buffer with the last buffer flag set to true to mark the end of the audio
          // stream.  The Windows version does not need to do this since it keeps track of the number
          // of queued buffers (m_nNumQueuedBuf == 0) so runBufferRecordedTask()
          // knows when to set the last buffer flag.
          m_pAudioQueue->addBuffer(0, 0, true, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
// I'm not going into the error state here, since I'm not sure if this is fatal.
            UAPI_ERROR(fn,"Failed to call AddBuffer buffer on last buffer ret: %s\n", ReturnCode::toString(returnCode));
            onStopDone();
            return;
          }
          if (m_hwi == NULL)
          {
            //already stopped.
            onStopDone();
            return;
          }
          // The Windows version calls waveInReset() which stops audio, then all
          // remaining buffers are still returned to application via WIM_DATA messages
          // and will be processed in runBufferRecordedTask().  For LINUX version,
          // we simply stop here.
          LHS_AUDIOIN_ERROR lhsErr = lhs_audioinStop(m_hwi);
          if (lhsErr != LHS_AUDIOIN_OK)
          {
            UAPI_ERROR(fn,"lhs_audioinStop failed %ld\n", lhsErr);
            //notify the error to the application.
            if (listener)
              listener->onError(ReturnCode::AUDIO_DRIVER_ERROR);
          }
          // The Windows version closes the audio device via waveInClose() once
          // the last buffer is processed by runBufferRecordedTask().
          // For LINUX version, close here after stopping audio.
          lhsErr = lhs_audioinClose(&m_hwi);
          if (lhsErr != LHS_AUDIOIN_OK)
          {
            UAPI_ERROR(fn,"lhs_audioinClose failed %ld\n", lhsErr);
            //notify the error to the application.
             if (listener)
              listener->onError(ReturnCode::AUDIO_DRIVER_ERROR);
          }
          m_hwi = 0;
        }



        void MicrophoneLINUX::runMicrophoneStoppedTask()
        {
          UAPI_FN_SCOPE("MicrophoneLINUX::runMicrophoneStoppedTask");

          if ( ( m_state == STOPPING ) || ( m_state == ERROR_STOPPING ) )
            onStopDone();
          else
            UAPI_ERROR(fn,"Cannot Complete Stop In State %d\n", m_state);
        }



        void MicrophoneLINUX::sendOnErrorToListener(ReturnCode::Type error, bool stopRecording)
        {
          UAPI_FN_SCOPE("MicrophoneLINUX::sendOnErrorToListener");
          
          //try our best to perform cleanup.
          if ( ( m_state == ERROR ) && ( stopRecording == true ) )
          {
            runStopMicrophoneTask();
          }
          if (listener)
            listener->onError(error);
        }



        AudioStreamProxy MicrophoneLINUX::createAudio(ReturnCode::Type & returnCode)
        {
          UAPI_FN_SCOPE("MicrophoneLINUX::createAudio");
           
          //need to protect when we are assigning m_state (done in WorkerQueue
          //thread) and when we are calling SetCodec (done in the app
          //thread).
          LockScope ls(mutex, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Failed to create LockScope\n");
             return AudioStreamProxy();
          }
          if ( ( m_state == STOPPING ) || ( m_state == ERROR ) ||
          ( m_state == ERROR_STOPPING ) || ( m_state == ERROR_STOPPED ) )
          {
            UAPI_WARN(fn,"Must not be in state %d to call createAudio\n", m_state);
            returnCode = ReturnCode::INVALID_STATE;
            return AudioStreamProxy();
          }
          return AudioStreamImpl::create(m_pAudioQueue, returnCode);
        }



        void MicrophoneLINUX::onStopDone()
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("MicrophoneLINUX::onStopDone");
           
          {
            //need to protect when we are assigning m_state (done in WorkerQueue
            //thread) and when we are calling SetCodec (done in the app
            //thread).
            LockScope ls(mutex, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
              UAPI_WARN(fn,"Failed to create LockScope\n");
              sendOnErrorToListener(returnCode,false);
              return;
            }
            if ( ( m_state != STOPPING ) && ( m_state != ERROR_STOPPING ) )
            {
              UAPI_ERROR(fn,"Cannot Complete Stop In State %d\n", m_state);
              sendOnErrorToListener(returnCode,false);
              return;
            }
            //we want to allow a user to re-use the MicrophoneLINUX object. To do so,
            //we must create a new AudioQueue for each start() operation. To make sure
            //we are ready for the next operation, we get rid of the old one here and
            //we create a new one. NOTE that we do this before we send the callback.
            if (m_pAudioQueue != 0)
            {
              //NOTE: we don't call delete m_pAudioQueue. This class is ref counted.
              //Instead, we call Release on it and once no one is using the object, it
              //will get released.
              m_pAudioQueue->removeRef(returnCode);
              if (returnCode != ReturnCode::SUCCESS)
              {
                UAPI_ERROR(fn,"Could not release ref count on AudioQueue\n");
                m_state = ERROR_STOPPED;
                sendOnErrorToListener(returnCode,false);
                return;
              }
              m_pAudioQueue = AudioQueue::create(m_codec, returnCode);
              if (returnCode != ReturnCode::SUCCESS)
              {
                UAPI_ERROR(fn,"Could not create AudioQueue\n");
                m_state = ERROR_STOPPED;
                sendOnErrorToListener(returnCode,false);
                return;
              }
            }
            if ( m_state == STOPPING )
              m_state = IDLE;
            else 
              m_state = ERROR_STOPPED;
          }
#ifdef ANDROID_AUDIODRIVER_WORKAROUND
          // Wait after we stop the microphone to work around a Q audio device driver bug.
          // If we put this in the apps instead, there's no risk of forgetting to disable this
          // workaround once the audio driver works the way it should.
          const unsigned short delay_ms = 1800;  // 340 for 44.1 kHz, about 1400 for 11.025 kHz, and 1800 for 8 kHz
          struct timeval sleep_time_struct;
          sleep_time_struct.tv_sec = 0;
          sleep_time_struct.tv_usec = delay_ms*1000; // microseconds
          select(0, NULL, NULL, NULL, &sleep_time_struct);
#endif
          if (listener)
            listener->onStopped();
        }



        SmartProxy::Root* MicrophoneLINUX::getRoot()
        {
          return rootProxy;
        }



        SetMicrophoneListenerTask::SetMicrophoneListenerTask(MicrophoneLINUXProxy& _microphone,
            AudioSourceListenerProxy& _listener):
            Task("SetMicrophoneListenerTask"),
            microphone(_microphone),
            listener(_listener)
        {}



        void SetMicrophoneListenerTask::run()
        {
          microphone->runSetMicrophoneListenerTask(listener);
        }



        StartMicrophoneTask::StartMicrophoneTask(MicrophoneLINUXProxy& _microphone):
            Task("StartMicrophoneTask"),
            microphone(_microphone)
        {}



        void StartMicrophoneTask::run()
        {
          microphone->runStartMicrophoneTask();
        }



        MicrophoneStartedTask::MicrophoneStartedTask(MicrophoneLINUXProxy& _microphone):
            Task("MicrophoneStartedTask"),
            microphone(_microphone)
        {}



        void MicrophoneStartedTask::run()
        {
          microphone->runMicrophoneStartedTask();
        }



        BufferRecordedTask::BufferRecordedTask(MicrophoneLINUXProxy& _microphone, WAVEHDR* _pwhdr):
            Task("BufferRecordedTask"),
            microphone(_microphone),
            pwhdr(_pwhdr)
        {}



        void BufferRecordedTask::run()
        {
          microphone->runBufferRecordedTask(pwhdr);
          pwhdr = 0;
        }



        BufferRecordedTask::~BufferRecordedTask()
        {
          if (pwhdr)
          {
            //buffer was not consumed, i.e. run was not called.
            free(pwhdr->pData);
            free(pwhdr);
          }
        }



        StopMicrophoneTask::StopMicrophoneTask(MicrophoneLINUXProxy& _microphone):
            Task("StopMicrophoneTask"),
            microphone(_microphone)
        {}



        void StopMicrophoneTask::run()
        {
          microphone->runStopMicrophoneTask();
        }



        MicrophoneStoppedTask::MicrophoneStoppedTask(MicrophoneLINUXProxy& _microphone):
            Task("MicrophoneStoppedTask"),
            microphone(_microphone)
        {}



        void MicrophoneStoppedTask::run()
        {
          microphone->runMicrophoneStoppedTask();
        }
      }
    }
  }
}
