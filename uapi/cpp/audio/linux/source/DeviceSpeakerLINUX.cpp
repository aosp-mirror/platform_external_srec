/*---------------------------------------------------------------------------*
 *  DeviceSpeakerLINUX.cpp                                                   *
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
#include <assert.h>
#include <stdio.h>
#include "DeviceSpeakerLINUX.h"
#include "WorkerQueue.h"
#include "WorkerQueueFactory.h"
#include "AudioBuffer.h"
#include "Codec.h"
#include "AudioStreamImpl.h"
#include "DeviceSpeakerListener.h"
#include "LoggerImpl.h"
#include "System.h"

using namespace android::speech::recognition::utilities;


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      DeviceSpeaker* DeviceSpeaker::instance = 0;
      
      DeviceSpeakerProxy DeviceSpeaker::getInstance(ReturnCode::Type& returnCode)
      {
        UAPI_FN_SCOPE("DeviceSpeakerLINUX::getInstance");
          
        if (impl::DeviceSpeakerLINUX::componentInitializer.returnCode)
        {
          returnCode = impl::DeviceSpeakerLINUX::componentInitializer.returnCode;
          return DeviceSpeakerProxy();
        }
        
        //we have to protect the construction of "instance". Make sure we don't have
        //multiple threads calling getInstance() while "instance" is equal to 0.
        LockScope lock(impl::DeviceSpeakerLINUX::mutex, returnCode);
        if (returnCode != ReturnCode::SUCCESS)
        {
          UAPI_ERROR(fn,"Failed to create LockScope of DeviceSpeakerLINUX::mutex\n");
          return DeviceSpeakerProxy();
        }
        
        //it's now safe to check if instance == 0
        if (instance == 0)
        {
          instance = new impl::DeviceSpeakerLINUX(returnCode);
          if (returnCode || instance == 0)
          {
            delete instance;
            instance = 0;
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return DeviceSpeakerProxy();
          }
          
          // register for auto cleanup.
          impl::DeviceSpeakerLINUX* instanceImpl = (impl::DeviceSpeakerLINUX*) instance;
          DeviceSpeakerProxy result(instanceImpl);
          if (!result)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return DeviceSpeakerProxy();
          }
          instanceImpl->rootProxy = result.getRoot();
          
          System* system = System::getInstance(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            delete instance;
            return DeviceSpeakerProxy();
          }
          
          // has to be called after rootProxy is assigned.
          system->add(instance, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            delete instance;
            return DeviceSpeakerProxy();
          }
          return result;
        }
        returnCode = ReturnCode::SUCCESS;
        impl::DeviceSpeakerLINUX* instanceImpl = (impl::DeviceSpeakerLINUX*) instance;
        return DeviceSpeakerProxy(instanceImpl->rootProxy);
      }
      
      namespace impl
      {
        /* if ENABLE_DUMMY_SPEAKER_CALLBACK_THREAD is defined, invoke callback function from another thread */
        
#ifdef ENABLE_DUMMY_SPEAKER
        
        /* Dummy code just to have speaker object that goes through the motions of playing audio. */
        
        typedef void CALLBACK(*pCallback)(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
        
        static pCallback g_OutCallback = NULL;
        static DWORD g_dwCallbackInstance = 0;
        
#ifdef ENABLE_DUMMY_SPEAKER_CALLBACK_THREAD
        typedef struct
        {
          /* parameters for callback function */
          HWAVEOUT  hwo;
          UINT      uMsg;
          DWORD_PTR dwInstance;
          DWORD_PTR dwParam1;
          DWORD_PTR dwParam2;
        } CallbackOptions;
        
        static void* CallbackThread(void *pParam)
        {
          CallbackOptions *pOptions = (CallbackOptions *) pParam;
          
          g_OutCallback(pOptions->hwo, pOptions->uMsg, pOptions->dwInstance, (DWORD_PTR)pOptions->dwParam1, (DWORD_PTR)pOptions->dwParam2);
          
          delete pOptions;
          
          return NULL;
        }
#endif
        
        static MMRESULT waveOutOpen(LPHWAVEOUT phwo, UINT uDeviceID, LPWAVEFORMATEX pwfx, DWORD dwCallback, DWORD dwCallbackInstance, DWORD fdwOpen)
        {
          g_OutCallback = (pCallback) dwCallback;
          g_dwCallbackInstance = dwCallbackInstance;
          
          *phwo = (WORD)10;
          
#ifdef ENABLE_DUMMY_SPEAKER_CALLBACK_THREAD
          int              rc;
          pthread_t        threadId;
          CallbackOptions *pOptions = new CallbackOptions;
          
          if (pOptions == 0)
            return MMSYSERR_NOMEM;
            
          pOptions->hwo        = *phwo;
          pOptions->uMsg       = WOM_OPEN;
          pOptions->dwInstance = g_dwCallbackInstance;
          pOptions->dwParam1   = (DWORD_PTR)NULL;
          pOptions->dwParam2   = (DWORD_PTR)NULL;
          
          rc = pthread_create(&threadId, NULL, &CallbackThread, pOptions);
          if (rc != 0)
          {
            UAPI_ERROR(fn,"error creating thread in DeviceSpeakerLINUX dummy callback.  pthread_create() returns %d\n", rc);
            return MMSYSERR_ERROR;
          }
#else
          g_OutCallback(*phwo, WOM_OPEN, g_dwCallbackInstance, (DWORD_PTR)NULL, (DWORD_PTR)NULL);
#endif
          
          return MMSYSERR_NOERROR;
        }
        
        static MMRESULT waveOutClose(HWAVEOUT hwo)
        {
#ifdef ENABLE_DUMMY_SPEAKER_CALLBACK_THREAD
          int              rc;
          pthread_t        threadId;
          CallbackOptions *pOptions = new CallbackOptions;
          
          if (pOptions == 0)
            return MMSYSERR_NOMEM;
            
          pOptions->hwo        = hwo;
          pOptions->uMsg       = WOM_CLOSE;
          pOptions->dwInstance = g_dwCallbackInstance;
          pOptions->dwParam1   = (DWORD_PTR)NULL;
          pOptions->dwParam2   = (DWORD_PTR)NULL;
          
          rc = pthread_create(&threadId, NULL, &CallbackThread, pOptions);
          if (rc != 0)
          {
            UAPI_ERROR(fn,"error creating thread in DeviceSpeakerLINUX dummy callback.  pthread_create() returns %d\n", rc);
            return MMSYSERR_ERROR;
          }
#else
          g_OutCallback(hwo, WOM_CLOSE, g_dwCallbackInstance, (DWORD_PTR)NULL, (DWORD_PTR)NULL);
#endif
          
          return MMSYSERR_NOERROR;
        }
        
        static MMRESULT waveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
        {
#ifdef ENABLE_DUMMY_SPEAKER_CALLBACK_THREAD
          int              rc;
          pthread_t        threadId;
          CallbackOptions *pOptions = new CallbackOptions;
          
          if (pOptions == 0)
            return MMSYSERR_NOMEM;
            
          pOptions->hwo        = hwo;
          pOptions->uMsg       = WOM_DONE;
          pOptions->dwInstance = g_dwCallbackInstance;
          pOptions->dwParam1   = (DWORD_PTR)pwh;
          pOptions->dwParam2   = (DWORD_PTR)NULL;
          
          rc = pthread_create(&threadId, NULL, &CallbackThread, pOptions);
          if (rc != 0)
          {
            UAPI_ERROR(fn,"error creating thread in DeviceSpeakerLINUX dummy callback.  pthread_create() returns %d\n", rc);
            return MMSYSERR_ERROR;
          }
#else
          g_OutCallback(hwo, WOM_DONE, g_dwCallbackInstance, (DWORD_PTR)pwh, (DWORD_PTR)NULL);
#endif
          
          return MMSYSERR_NOERROR;
        }
        
        static MMRESULT waveOutReset(HWAVEOUT hwo)
        {
          return MMSYSERR_NOERROR;
        }
        
        MMRESULT waveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
        {
          return MMSYSERR_NOERROR;
        }
        
        MMRESULT waveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
        {
          return MMSYSERR_NOERROR;
        }
        
        DWORD WaitForSingleObject(HANDLE hHandle, WORD dwMilliseconds)
        {
          return 0;
        }
        
        VOID ZeroMemory(PVOID Destination, DWORD Length)
        {
          memset(Destination, 0, Length);
        }
        
#endif  /* #ifdef ENABLE_DUMMY_SPEAKER */
        
        
        
        DEFINE_SMARTPROXY(impl, DeviceSpeakerLINUXProxy, DeviceSpeakerProxy, DeviceSpeakerLINUX);
        
        Mutex* DeviceSpeakerLINUX::mutex = 0;
        DeviceSpeakerLINUX::ComponentInitializer DeviceSpeakerLINUX::componentInitializer;
        
        DeviceSpeakerLINUX::ComponentInitializer::ComponentInitializer()
        {
          mutex = Mutex::create(false, returnCode);
          if (returnCode)
          {
            fprintf(stderr, "Could not create DeviceSpeakerLINUX::mutex\n");
            return;
          }
        }
        
        DeviceSpeakerLINUX::ComponentInitializer::~ComponentInitializer()
        {
          delete mutex;
          returnCode = ReturnCode::UNKNOWN;
        }
        
        struct WaveOutHelperStruct
        {
          WorkerQueue* workerQueue;
          DeviceSpeakerLINUXProxy deviceSpeaker;
        };
        
        void CALLBACK __waveOutProc(HWAVEOUT, UINT uMsg,
                                    DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR)
        {
          //NOTE: we are not allowed to lock any mutex in this functions. This is
          //part of the contract when using the waveIn api.
          ReturnCode::Type returnCode;
          
          WaveOutHelperStruct* helper = ((WaveOutHelperStruct*) dwInstance);
          DeviceSpeakerLINUXProxy& deviceSpeaker = helper->deviceSpeaker;
          
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::__waveOutProc");
          
          WorkerQueue* workerQueue     = helper->workerQueue;
          switch (uMsg)
          {
            case WOM_OPEN:
            {
              UAPI_INFO(fn,"WOM_OPEN\n");
              DeviceSpeakerStartedTask* task = new DeviceSpeakerStartedTask(deviceSpeaker);
              if (!task)
              {
                UAPI_ERROR(fn,"Could not create DeviceSpeakerStartedTask\n");
                deviceSpeaker->sendOnErrorToListener(ReturnCode::OUT_OF_MEMORY,true);
                delete helper;
                return;
              }
              workerQueue->enqueue(task, returnCode);
              if (returnCode != ReturnCode::SUCCESS)
              {
                UAPI_ERROR(fn,"could not enqueue DeviceSpeakerStartedTask %s\n",
                           ReturnCode::toString(returnCode));
                deviceSpeaker->sendOnErrorToListener(returnCode,true);
                delete task;
                delete helper;
                return;
              }
              UAPI_INFO(fn,"enqueue(new DeviceSpeakerStartedTask)\n");
              break;
            }
            case WOM_DONE:
            {
              UAPI_INFO(fn,"WOM_DONE\n");
              BufferPlayedTask* task = new BufferPlayedTask(deviceSpeaker, (WAVEHDR*) dwParam1);
              if (!task)
              {
                UAPI_ERROR(fn,"Could not create BufferPlayedTask\n");
                deviceSpeaker->sendOnErrorToListener(ReturnCode::OUT_OF_MEMORY,true);
                delete helper;
                return;
              }
              workerQueue->enqueue(task, returnCode);
              if (returnCode != ReturnCode::SUCCESS)
              {
                UAPI_ERROR(fn,"could not enqueue BufferPlayedTask %s\n",
                           ReturnCode::toString(returnCode));
                deviceSpeaker->sendOnErrorToListener(returnCode,true);
                delete task;
                delete helper;
                return;
              }
              UAPI_INFO(fn,"enqueue(new BufferPlayedTask)\n");
              break;
            }
            case WOM_CLOSE:
            {
              UAPI_INFO(fn,"WOM_CLOSE\n");
              DeviceSpeakerStoppedTask* task = new DeviceSpeakerStoppedTask(deviceSpeaker);
              if (!task)
              {
                UAPI_ERROR(fn,"Could not create DeviceSpeakerStoppedTask\n");
                deviceSpeaker->sendOnErrorToListener(ReturnCode::OUT_OF_MEMORY,true);
                delete helper;
                return;
              }
              workerQueue->enqueue(task, returnCode);
              if (returnCode != ReturnCode::SUCCESS)
              {
                UAPI_ERROR(fn,"could not enqueue DeviceSpeakerStoppedTask %s\n",
                           ReturnCode::toString(returnCode));
                deviceSpeaker->sendOnErrorToListener(returnCode,true);
                delete task;
                delete helper;
                return;
              }
              UAPI_INFO(fn,"enqueue(new DeviceSpeakerStoppedTask)\n");
              
              delete helper;
              break;
            }
          }
        }
        
        DeviceSpeakerLINUX::DeviceSpeakerLINUX(ReturnCode::Type& returnCode):
            codec(Codec::PCM_16BIT_8K),
            workerQueue(0),
            m_hwo(0),
            collectingBuffers(true),
            state(IDLE),
            rootProxy(0)
        {
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::DeviceSpeakerLINUX");
          UAPI_TRACE(fn,"this=%p\n", this);
          
          m_wfx.cbSize             = 0;
          m_wfx.nBlockAlign        = 2;
          m_wfx.nChannels          = 1;
          m_wfx.wBitsPerSample     = 16;
          
          ReturnCode::Type rc;
          m_wfx.nSamplesPerSec = Codec::getSampleRate(codec, rc);
          if (rc)
            return;
          m_wfx.nAvgBytesPerSec    = m_wfx.nSamplesPerSec * m_wfx.wBitsPerSample / 8;
          m_wfx.wFormatTag         = WAVE_FORMAT_PCM;
          
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
          
          returnCode = ReturnCode::SUCCESS;
        }
        
        void DeviceSpeakerLINUX::setCodec(Codec::Type playbackCodec, ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::setCodec");
            
          //need to protect when we are assigning state (done in WorkerQueue
          //thread) and when we are calling SetCodec (done in the app
          //thread).
          LockScope ls(mutex, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_WARN(fn,"Failed to create LockScope\n");
            return;
          }
          
          if (state != IDLE)
          {
            UAPI_ERROR(fn,"Must be in IDLE state to call SetCodec\n");
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          
          codec = playbackCodec;
          
          m_wfx.nSamplesPerSec = Codec::getSampleRate(codec, returnCode);
          if (returnCode)
            return;
            
          m_wfx.nAvgBytesPerSec = m_wfx.nSamplesPerSec * m_wfx.wBitsPerSample / 8;
          returnCode = ReturnCode::SUCCESS;
        }
        
        void DeviceSpeakerLINUX::setListener(DeviceSpeakerListenerProxy& _listener,
                                             ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::setListener");
            
          if (!_listener)
          {
            //not accepting NULL listener
            UAPI_WARN(fn,"Listener cannot be NULL\n");
            returnCode = ReturnCode::ILLEGAL_ARGUMENT;
            return;
          }
          
          DeviceSpeakerLINUXProxy proxy(rootProxy);
          if (!proxy)
          {
            UAPI_ERROR(fn,"Could not create proxy\n");
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          
          SetDeviceSpeakerListenerTask* task = new SetDeviceSpeakerListenerTask(proxy, _listener);
          if (!task)
          {
            UAPI_ERROR(fn,"Could not create SetDeviceSpeakerListenerTask\n");
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          
          workerQueue->enqueue(task, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            delete task;
            UAPI_ERROR(fn,"Failed to enqueue SetDeviceSpeakerListenerTask\n");
            return;
          }
          returnCode = ReturnCode::SUCCESS;
        }
        
        void DeviceSpeakerLINUX::runSetDeviceSpeakerListenerTask(DeviceSpeakerListenerProxy& _listener)
        {
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::runSetDeviceSpeakerListenerTask");
          
          listener = _listener;
        }
        
        void DeviceSpeakerLINUX::shutdown(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::shutdown");
            
          stop(returnCode);
        }
        
        DeviceSpeakerLINUX::~DeviceSpeakerLINUX()
        {
          UAPI_FN_NAME("DeviceSpeakerLINUX::~DeviceSpeakerLINUX");
          UAPI_TRACE(fn,"this=%p\n", this);
          
          if (m_hwo != 0)
          {
            //Stop was not called, do our best to cleanup.
            waveOutReset(m_hwo);
            waveOutClose(m_hwo);
          }
          
          instance = 0;
        }
        
        void DeviceSpeakerLINUX::start(AudioStreamProxy& audio, ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::start");
            
          ReturnCode::Type dummy;
          
          if (workerQueue == 0)
          {
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          
          if (!audio)
          {
            returnCode = ReturnCode::ILLEGAL_ARGUMENT;
            return;
          }
          
          audioStream = (AudioStreamImplProxy&) audio;
          
          //Flag this AudioStreamImpl as being locked. This means that this audio cannot be
          //passed to other modules.
          audioStream->lock(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Cannot lock the audio stream: %s\n", ReturnCode::toString(returnCode));
            return;
          }
          
          DeviceSpeakerLINUXProxy proxy(rootProxy);
          if (!proxy)
          {
            UAPI_ERROR(fn,"Could not create proxy\n");
            cleanupAudioStreamProxy(dummy);
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          
          StartPlaybackTask* task = new StartPlaybackTask(proxy);
          if (!task)
          {
            UAPI_ERROR(fn,"Could not create StartPlaybackTask\n");
            cleanupAudioStreamProxy(dummy);
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          workerQueue->enqueue(task, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            delete task;
            cleanupAudioStreamProxy(dummy);
            UAPI_ERROR(fn,"Failed to enqueue StartPlaybackTask\n");
            return;
          }
          UAPI_INFO(fn,"enqueue(new StartPlaybackTask)\n");
          
          returnCode = ReturnCode::SUCCESS;
        }
        
        void DeviceSpeakerLINUX::runStartPlaybackTask()
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::runStartPlaybackTask");
          
          if (state != IDLE)
          {
            UAPI_ERROR(fn,"Must be in IDLE state to call start\n");
            sendOnErrorToListener(ReturnCode::INVALID_STATE,true);
            return;
          }
          
          {
            //need to protect when we are assigning state (done in WorkerQueue
            //thread) and when we are calling SetCodec (done in the app
            //thread).
            LockScope ls(mutex, returnCode);
            if (returnCode)
            {
              UAPI_WARN(fn,"Failed to create LockScope\n");
              sendOnErrorToListener(returnCode,false);
              return;
            }
            state = STARTING;
          }
          
          
          WaveOutHelperStruct* helper = new WaveOutHelperStruct();
          if (helper == 0)
          {
            UAPI_ERROR(fn,"Could not allocated memory for WaveOutHelperStruct\n");
            sendOnErrorToListener(ReturnCode::OUT_OF_MEMORY,true);
            return;
          }
          helper->workerQueue = workerQueue;
          
          DeviceSpeakerLINUXProxy proxy(rootProxy);
          if (!proxy)
          {
            UAPI_ERROR(fn,"Could not create proxy\n");
            sendOnErrorToListener(ReturnCode::OUT_OF_MEMORY,true);
            delete helper;
            return;
          }
          helper->deviceSpeaker = proxy;
          
          MMRESULT mmr;
          mmr = waveOutOpen(&m_hwo, WAVE_MAPPER, &m_wfx, (DWORD_PTR)__waveOutProc,
                            (DWORD_PTR) helper, CALLBACK_FUNCTION);
          if (mmr != MMSYSERR_NOERROR)
          {
            UAPI_ERROR(fn,"waveOutOpen failed %d\n", mmr);
            sendOnErrorToListener(ReturnCode::AUDIO_DRIVER_ERROR,true);
            delete helper;
            return;
          }
          
          collectingBuffers = true;
          firstSent = -1;
          numBuffers = 0;
          
          //initialize the memory of the WAVEHDR.
          ZeroMemory(m_whs, NUM_BUFFERS_QUEUED*sizeof(WAVEHDR));
          
          //work will continue in RunDeviceSpeakerStartedTask which is triggered
          //after we have received the WOM_OPEN event.
        }
        
        void DeviceSpeakerLINUX::runDeviceSpeakerStartedTask()
        {
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::runDeviceSpeakerStartedTask");
          
          if (state != STARTING)
          {
            //someone called stop.
            UAPI_INFO(fn,"Start recording was aborted, current state is %d\n", state);
            return;
          }
          
          state = PLAYING;
          
          //send callback to the application
          if (listener)
            listener->onStarted();
            
          //time to read data from the audio stream.
          readBuffersFromStream();
        }
        
        void DeviceSpeakerLINUX::runBufferPlayedTask(WAVEHDR* pWaveHdr)
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::runBufferPlayedTask");
          
          //FROM MSDN WEB SITE: The WOM_DONE message is sent to a waveform-audio output
          //callback function when the given output buffer is being returned to the
          //application. Buffers are returned to the application when they have been
          //played, or as the result of a call to the waveOutReset function.
          
          //buffer index
          size_t bufferIndex = pWaveHdr - m_whs;
          
          firstSent++;
          firstSent %= NUM_BUFFERS_QUEUED;
          numBuffers--;
          
          //now that the audio has been played, we can Release the AudioBuffer.
          AudioBuffer* pAudioBuff = (AudioBuffer*) m_whs[bufferIndex].dwUser;
          
          //the play can end in two different ways. When ended, we have to close the
          //waveOut.
          // 1) all the samples were played
          //  or
          // 2) Stop() was called and all the samples were returned.
          // or
          // 3) all the samples were read and all the samples were returned.
          if (/* 1) */ (m_whs[bufferIndex].dwFlags& WHDR_ENDLOOP) ||
                       /* 2) */ (state == STOPPING && numBuffers == 0) ||
                       /* 3) */ (state == DRAINING && numBuffers == 0))
          {
            // last buffer was played (WHDR_ENDLOOP)
            // now close the audio player.
            MMRESULT mmr = waveOutUnprepareHeader(m_hwo, &(m_whs[bufferIndex]), sizeof(WAVEHDR));
            if (mmr != MMSYSERR_NOERROR)
            {
              //for now, not considered as a fatal error.
              UAPI_WARN(fn,"waveOutUnprepareHeader failed %d\n", mmr);
            }
            
            closeWaveOut(returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
              UAPI_ERROR(fn,"closeWaveOut returned an error\n");
              sendOnErrorToListener(returnCode,true);
              return;
            }
          }
          else
          {
            //we are not stopping, we need to read more data.
            MMRESULT mmr = waveOutUnprepareHeader(m_hwo, &(m_whs[bufferIndex]), sizeof(WAVEHDR));
            if (mmr != MMSYSERR_NOERROR)
            {
              UAPI_WARN(fn,"waveOutUnprepareHeader failed %d\n", mmr);
            }
            readBuffersFromStream();
          }
          
          //release the buffer
          if (audioStream)
          {
            audioStream->release(pAudioBuff, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
              UAPI_ERROR(fn,"Failed to release audio buffer %p\n", pAudioBuff);
              sendOnErrorToListener(returnCode,true);
              return;
            }
          }
        }
        
        void DeviceSpeakerLINUX::runReadAudioBufferTask()
        {
          readBuffersFromStream();
        }
        
        void DeviceSpeakerLINUX::readBuffersFromStream()
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::readBuffersFromStream");
          
          //stop reading buffers if the stop is in progress.
          if (state != PLAYING)
          {
            UAPI_INFO(fn,"Not reading a new buffer, state is %d\n", state);
            return;
          }
          
          //we try to fill up to NUM_BUFFERS_QUEUED at a time.
          while (numBuffers < NUM_BUFFERS_QUEUED && returnCode != ReturnCode::END_OF_STREAM)
          {
            //read from the stream. If 0 is returned, it means that no buffers are
            //ready.
            AudioBuffer* buffer = audioStream->read(returnCode);
            if (buffer)
            {
              if (buffer->size == 0)
              {
                audioStream->release(buffer, returnCode);
                if (returnCode != ReturnCode::SUCCESS)
                {
                  UAPI_ERROR(fn,"Failed to release audio buffer %p\n", buffer);
                  sendOnErrorToListener(returnCode,true);
                  return;
                }
                continue;
              }
              
              int iBuf;
              if (!numBuffers)
              {
                iBuf = 0;
                firstSent = 0;
              }
              else
              {
                iBuf = firstSent + numBuffers;
                iBuf %= NUM_BUFFERS_QUEUED;
              }
              ++numBuffers;
              
              //
              //NOTE: buffer will get Released once the buffer has been played.
              //By doing this, we will avoid having to copy all the samples.
              //
              prepareBuffer(buffer, iBuf, returnCode);
              if (returnCode != ReturnCode::SUCCESS)
              {
                UAPI_ERROR(fn,"Failed to prepare buffer index %d\n", iBuf);
                sendOnErrorToListener(returnCode,true);
                return;
              }
              
              if (!collectingBuffers)
              {
                writeWaveOutBuffer(iBuf, returnCode);
                if (returnCode != ReturnCode::SUCCESS)
                {
                  UAPI_ERROR(fn,"Failed to write buffer index %d\n", iBuf);
                  sendOnErrorToListener(returnCode,true);
                  return;
                }
              }
              else if ((numBuffers >= BUFFERS_TO_COLLECT))
              {
                collectingBuffers = false;
                for (int i = 0; i < numBuffers; i++)
                {
                  writeWaveOutBuffer(i, returnCode);
                  if (returnCode != ReturnCode::SUCCESS)
                  {
                    UAPI_ERROR(fn,"Failed to write buffer index %d\n", i);
                    sendOnErrorToListener(returnCode,true);
                    return;
                  }
                }
              }
            }
            else
            {
              if (returnCode == ReturnCode::END_OF_STREAM)
              {
                //nothing else to read.
                if (collectingBuffers && numBuffers < BUFFERS_TO_COLLECT)
                {
                  //we were collecting buffer, make sure we submit them.
                  collectingBuffers = false;
                  for (int i = 0; i < numBuffers; i++)
                  {
                    writeWaveOutBuffer(i, returnCode);
                    if (returnCode != ReturnCode::SUCCESS)
                    {
                      UAPI_ERROR(fn,"Failed to write buffer index %d\n", i);
                      sendOnErrorToListener(returnCode,true);
                      return;
                    }
                  }
                }
                //all the samples were read, waiting for them to be played.
                state = DRAINING;
                
                if (numBuffers == 0)
                {
                  //nothing is queued for playback, we must stop now.
                  closeWaveOut(returnCode);
                  if (returnCode != ReturnCode::SUCCESS)
                  {
                    UAPI_ERROR(fn,"closeWaveOut returned an error\n");
                    sendOnErrorToListener(returnCode,true);
                    return;
                  }
                }
              }
              else if (returnCode == ReturnCode::PENDING_DATA)
              {
                //the last read failed, this means that no data is ready. If we
                //were able to read at least one buffer then we are ok. If nothing
                //could be read, or that we are collecting buffer, then we have to
                //register a callback to read later.
                if (numBuffers == 0 || collectingBuffers)
                {
                  //we were not able to read a single buffer. We will have to try
                  //again a little later (20 ms).
                  
                  ReturnCode::Type returnCode;
                  DeviceSpeakerLINUXProxy proxy(rootProxy);
                  if (!proxy)
                  {
                    UAPI_ERROR(fn,"Could not create proxy\n");
                    sendOnErrorToListener(ReturnCode::INVALID_STATE,true);
                    return;
                  }
                  ReadAudioBufferTask* task = new ReadAudioBufferTask(proxy, 20);
                  if (!task)
                  {
                    UAPI_ERROR(fn,"Could not create ReadAudioBufferTask\n");
                    sendOnErrorToListener(ReturnCode::OUT_OF_MEMORY,true);
                    return;
                  }
                  workerQueue->enqueue(task, returnCode);
                  if (returnCode != ReturnCode::SUCCESS)
                  {
                    UAPI_ERROR(fn,"could not enqueue ReadAudioBufferTask %s\n", ReturnCode::toString(returnCode));
                    sendOnErrorToListener(returnCode,true);
                    delete task;
                    return;
                  }
                  UAPI_INFO(fn,"enqueue(new ReadAudioBufferTask)\n");
                }
              }
              else
              {
                UAPI_ERROR(fn,"AudioStream::read returned an unexpected error %s\n", ReturnCode::toString(returnCode));
                sendOnErrorToListener(returnCode,true);
                return;
              }
              break; //get out of the loop
            }
          }//while
        }
        
        void DeviceSpeakerLINUX::stop(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::stop");
            
          DeviceSpeakerLINUXProxy proxy(rootProxy);
          if (!proxy)
          {
            UAPI_ERROR(fn,"Could not create proxy\n");
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          StopPlaybackTask* task = new StopPlaybackTask(proxy);
          if (!task)
          {
            UAPI_ERROR(fn,"Could not create StopPlaybackTask\n");
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          workerQueue->enqueue(task, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Failed to enqueue StopPlaybackTask\n");
            delete task;
            return;
          }
          UAPI_INFO(fn,"enqueue(new StopPlaybackTask)\n");
          
          returnCode = ReturnCode::SUCCESS;
        }
        
        void DeviceSpeakerLINUX::runStopPlaybackTask()
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::runStopPlaybackTask");
          
          //check the state.
          if (state == IDLE)
          {
            //already stopped. Do nothing.
            UAPI_INFO(fn,"DeviceSpeaker is already stopped.\n");
            return;
          }
          else if (state == STOPPING)
          {
            //stop called multiple times, do nothing
            UAPI_WARN(fn,"stop was already called.\n");
            return;
          }
          
          state = STOPPING;
          
          if (m_hwo == 0)
          {
            //already stopped, all the buffers were played and callbacks were called.
            UAPI_INFO(fn,"m_hwo is null, stop was already called for DeviceSpeakerLINUX\n");
            return;
          }
          
          //we now call waveOutReset which will cause the audio driver to return all
          //the queued buffers. Only do so if we have queue buffers.
          if (numBuffers != 0 && collectingBuffers == false)
          {
            MMRESULT mmr = waveOutReset(m_hwo);
            if (mmr != MMSYSERR_NOERROR)
            {
              UAPI_ERROR(fn,"waveOutReset failed %d\n", mmr);
              //don't report the error, try to close anyway and hope of the best.
              closeWaveOut(returnCode);
              if (returnCode != ReturnCode::SUCCESS)
              {
                UAPI_ERROR(fn,"closeWaveOut returned an error\n");
                sendOnErrorToListener(returnCode,false);
                return;
              }
            }
          }
          else
          {
            //now close the audio player.
            closeWaveOut(returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
              UAPI_ERROR(fn,"closeWaveOut returned an error\n");
              sendOnErrorToListener(returnCode,false);
              return;
            }
          }
        }
        
        
        void DeviceSpeakerLINUX::runDeviceSpeakerStoppedTask()
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::runDeviceSpeakerStoppedTask");
          
          //need to protect when we are assigning state (done in WorkerQueue
          //thread) and when we are calling SetCodec (done in the app
          //thread).
          {
            LockScope ls(mutex, returnCode);
            if (returnCode)
            {
              UAPI_WARN(fn,"Failed to create LockScope\n");
              sendOnErrorToListener(returnCode,true);
              return;
            }
            state = IDLE;
          }
          
          sendOnStoppedToListener();
        }
        
        void DeviceSpeakerLINUX::prepareBuffer(AudioBuffer* pBuf, int iBuf, ReturnCode::Type & returnCode)
        {
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::prepareBuffer");
            
          if (codec == Codec::PCM_16BIT_8K ||
              codec == Codec::PCM_16BIT_11K ||
              codec == Codec::PCM_16BIT_22K)
          {
            m_whs[iBuf].lpData = (LPSTR) pBuf->buffer;
            m_whs[iBuf].dwBufferLength = pBuf->size;
            
            //to avoid copying memory, we save the address of pBuf. We will call
            //Release on it once it's been played.
            m_whs[iBuf].dwUser = (DWORD_PTR)pBuf;
          }
          else
          {
            //TODO check all the codecs
            assert(false);
          }
          
          //first buffer
          if (collectingBuffers && numBuffers == 0)
            m_whs[iBuf].dwFlags = WHDR_BEGINLOOP;
            
          //last buffer
          if (pBuf->isLastBuffer)
            m_whs[iBuf].dwFlags = WHDR_ENDLOOP;
            
          MMRESULT  mmr = waveOutPrepareHeader(m_hwo, &(m_whs[iBuf]), sizeof(WAVEHDR));
          if (mmr != MMSYSERR_NOERROR)
          {
            UAPI_ERROR(fn,"waveOutPrepareHeader failed %d\n", mmr);
            returnCode = ReturnCode::AUDIO_DRIVER_ERROR;
            return;
          }
          
          returnCode = ReturnCode::SUCCESS;
        }
        
        void DeviceSpeakerLINUX::closeWaveOut(ReturnCode::Type & returnCode)
        {
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::closeWaveOut");
            
          if (m_hwo == 0)
          {
            returnCode = ReturnCode::SUCCESS;
            return;
          }
          
          MMRESULT mmr = waveOutClose(m_hwo);
          if (mmr != MMSYSERR_NOERROR)
          {
            UAPI_ERROR(fn,"waveOutClose failed %d\n", mmr);
            m_hwo = 0;
            returnCode = ReturnCode::AUDIO_DRIVER_ERROR;
            return;
          }
          WaitForSingleObject(m_hwo, INFINITE);
          m_hwo = 0;
          
          returnCode = ReturnCode::SUCCESS;
        }
        
        void DeviceSpeakerLINUX::writeWaveOutBuffer(unsigned int uiIndex, ReturnCode::Type & returnCode)
        {
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::writeWaveOutBuffer");
            
          MMRESULT mmr = waveOutWrite(m_hwo, &(m_whs[uiIndex]), sizeof(WAVEHDR));
          if (mmr != MMSYSERR_NOERROR)
          {
            UAPI_ERROR(fn,"waveOutWrite failed %d\n", mmr);
            returnCode = ReturnCode::AUDIO_DRIVER_ERROR;
            return;
          }
          
          returnCode = ReturnCode::SUCCESS;
        }
        
        SetDeviceSpeakerListenerTask::SetDeviceSpeakerListenerTask(DeviceSpeakerLINUXProxy& _deviceSpeaker,
            DeviceSpeakerListenerProxy& _listener):
            Task("SetDeviceSpeakerListenerTask"),
            deviceSpeaker(_deviceSpeaker),
            listener(_listener)
        {}
        
        void SetDeviceSpeakerListenerTask::run()
        {
          deviceSpeaker->runSetDeviceSpeakerListenerTask(listener);
        }
        
        StartPlaybackTask::StartPlaybackTask(DeviceSpeakerLINUXProxy& _deviceSpeaker):
            Task("StartPlaybackTask"),
            deviceSpeaker(_deviceSpeaker)
        {}
        
        void StartPlaybackTask::run()
        {
          deviceSpeaker->runStartPlaybackTask();
        }
        
        DeviceSpeakerStartedTask::DeviceSpeakerStartedTask(DeviceSpeakerLINUXProxy& _deviceSpeaker):
            Task("DeviceSpeakerStartedTask"),
            deviceSpeaker(_deviceSpeaker)
        {}
        
        void DeviceSpeakerStartedTask::run()
        {
          deviceSpeaker->runDeviceSpeakerStartedTask();
        }
        
        ReadAudioBufferTask::ReadAudioBufferTask(DeviceSpeakerLINUXProxy& _deviceSpeaker, UINT32 timeout):
            ScheduledTask(timeout, "ReadAudioBufferTask"),
            deviceSpeaker(_deviceSpeaker)
        {}
        
        void ReadAudioBufferTask::run()
        {
          deviceSpeaker->runReadAudioBufferTask();
        }
        
        BufferPlayedTask::BufferPlayedTask(DeviceSpeakerLINUXProxy& _deviceSpeaker, WAVEHDR* _waveHdr):
            Task("BufferPlayedTask"),
            deviceSpeaker(_deviceSpeaker),
            waveHdr(_waveHdr)
        {}
        
        void BufferPlayedTask::run()
        {
          deviceSpeaker->runBufferPlayedTask(waveHdr);
        }
        
        StopPlaybackTask::StopPlaybackTask(DeviceSpeakerLINUXProxy& _deviceSpeaker):
            Task("StopPlaybackTask"),
            deviceSpeaker(_deviceSpeaker)
        {}
        
        void StopPlaybackTask::run()
        {
          deviceSpeaker->runStopPlaybackTask();
        }
        
        
        DeviceSpeakerStoppedTask::DeviceSpeakerStoppedTask(DeviceSpeakerLINUXProxy& _deviceSpeaker):
            Task("DeviceSpeakerStoppedTask"),
            deviceSpeaker(_deviceSpeaker)
        {}
        
        void DeviceSpeakerStoppedTask::run()
        {
          deviceSpeaker->runDeviceSpeakerStoppedTask();
        }
        
        void DeviceSpeakerLINUX::sendOnErrorToListener(ReturnCode::Type error, bool stopPlayback)
        {
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::sendOnErrorToListener");
            
          ReturnCode::Type rc;
          cleanupAudioStreamProxy(rc);
          //don't care about rc, we are already sending an error.
          
          //try our best to perform cleanup.
          if ((state == PLAYING || state == STARTING)&& stopPlayback)
          {
            runStopPlaybackTask();
          }
          
          if (listener)
            listener->onError(error);
        }
        
        void DeviceSpeakerLINUX::sendOnStoppedToListener()
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::sendOnStoppedToListener");
          
          cleanupAudioStreamProxy(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
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
        
        void DeviceSpeakerLINUX::cleanupAudioStreamProxy(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("DeviceSpeakerLINUX::cleanupAudioStreamProxy");
            
          if (audioStream)
          {
            audioStream->unlock();
            
            //if we are collecting buffer, it means that they were not yet submitted to
            //waveOutWrite. In this case we have to make sure we release the memory.
            if (collectingBuffers)
            {
              for (int i = 0; i < numBuffers; i++)
              {
                AudioBuffer* pAudioBuff = (AudioBuffer*) m_whs[i].dwUser;
                audioStream->release(pAudioBuff, returnCode);
                if (returnCode != ReturnCode::SUCCESS)
                  UAPI_ERROR(fn,"Failed to release audio buffer %p\n", pAudioBuff);
              }
            }
            
            audioStream = AudioStreamImplProxy();
          }
          returnCode = ReturnCode::SUCCESS;
        }
        
        SmartProxy::Root* DeviceSpeakerLINUX::getRoot()
        {
          return rootProxy;
        }
      }
    }
  }
}
