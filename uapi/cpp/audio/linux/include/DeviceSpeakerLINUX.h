/*---------------------------------------------------------------------------*
 *  DeviceSpeakerLINUX.h                                                     *
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

#ifndef __UAPI__DEVICESPEAKER_LINUX
#define __UAPI__DEVICESPEAKER_LINUX

#include "DeviceSpeaker.h"
#include "Task.h"
#include "Codec.h"
#include "AudioStreamImpl.h"
#include "DeviceSpeakerListener.h"

#define ENABLE_DUMMY_SPEAKER

#ifdef ENABLE_DUMMY_SPEAKER

/* Dummy code just to have speaker object that goes through the motions of playing audio. */
#ifdef __cplusplus
extern "C"
{
#endif

  typedef void VOID;
  typedef void* PVOID;
  typedef unsigned int UINT;
  typedef unsigned int WORD;
  typedef unsigned long DWORD;
  typedef DWORD DWORD_PTR;
  typedef char *LPSTR;
  
  typedef WORD HANDLE;
  
  typedef UINT  MMRESULT;   /* error return code, 0 means no error */
  
  
#define DECLARE_HANDLE(x) typedef WORD x
  
#define CALLBACK_FUNCTION   0x00030000l    /* dwCallback is a FARPROC */
  
#define WAVE_FORMAT_PCM 1
  
#define MMSYSERR_BASE          0
  
#define MMSYSERR_NOERROR 0
#define MMSYSERR_ERROR        (MMSYSERR_BASE + 1)  /* unspecified error */
#define MMSYSERR_BADDEVICEID  (MMSYSERR_BASE + 2)  /* device ID out of range */
#define MMSYSERR_NOTENABLED   (MMSYSERR_BASE + 3)  /* driver failed enable */
#define MMSYSERR_ALLOCATED    (MMSYSERR_BASE + 4)  /* device already allocated */
#define MMSYSERR_INVALHANDLE  (MMSYSERR_BASE + 5)  /* device handle is invalid */
#define MMSYSERR_NODRIVER     (MMSYSERR_BASE + 6)  /* no device driver present */
#define MMSYSERR_NOMEM        (MMSYSERR_BASE + 7)  /* memory allocation error */
#define MMSYSERR_NOTSUPPORTED (MMSYSERR_BASE + 8)  /* function isn't supported */
#define MMSYSERR_BADERRNUM    (MMSYSERR_BASE + 9)  /* error value out of range */
#define MMSYSERR_INVALFLAG    (MMSYSERR_BASE + 10) /* invalid flag passed */
#define MMSYSERR_INVALPARAM   (MMSYSERR_BASE + 11) /* invalid parameter passed */
#define MMSYSERR_HANDLEBUSY   (MMSYSERR_BASE + 12) /* handle being used */
  /* simultaneously on another */
  /* thread (eg callback) */
#define MMSYSERR_INVALIDALIAS (MMSYSERR_BASE + 13) /* specified alias not found */
#define MMSYSERR_BADDB        (MMSYSERR_BASE + 14) /* bad registry database */
#define MMSYSERR_KEYNOTFOUND  (MMSYSERR_BASE + 15) /* registry key not found */
#define MMSYSERR_READERROR    (MMSYSERR_BASE + 16) /* registry read error */
#define MMSYSERR_WRITEERROR   (MMSYSERR_BASE + 17) /* registry write error */
#define MMSYSERR_DELETEERROR  (MMSYSERR_BASE + 18) /* registry delete error */
#define MMSYSERR_VALNOTFOUND  (MMSYSERR_BASE + 19) /* registry value not found */
#define MMSYSERR_NODRIVERCB   (MMSYSERR_BASE + 20) /* driver does not call DriverCallback */
#define MMSYSERR_LASTERROR    (MMSYSERR_BASE + 20) /* last error in range */
  
#define MM_WOM_OPEN         0x3BB           /* waveform output */
#define MM_WOM_CLOSE        0x3BC
#define MM_WOM_DONE         0x3BD
  
#define MM_WIM_OPEN         0x3BE           /* waveform input */
#define MM_WIM_CLOSE        0x3BF
#define MM_WIM_DATA         0x3C0
  
  /* wave callback messages */
#define WOM_OPEN        MM_WOM_OPEN
#define WOM_CLOSE       MM_WOM_CLOSE
#define WOM_DONE        MM_WOM_DONE
#define WIM_OPEN        MM_WIM_OPEN
#define WIM_CLOSE       MM_WIM_CLOSE
#define WIM_DATA        MM_WIM_DATA
  
  /* flags for dwFlags field of WAVEHDR */
#define WHDR_DONE       0x00000001  /* done bit */
#define WHDR_PREPARED   0x00000002  /* set if this header has been prepared */
#define WHDR_BEGINLOOP  0x00000004  /* loop start block */
#define WHDR_ENDLOOP    0x00000008  /* loop end block */
#define WHDR_INQUEUE    0x00000010  /* reserved for driver */
  
  /* device ID for wave device mapper */
#ifndef WAVE_MAPPER
#define WAVE_MAPPER     ((UINT)-1)
#endif
  
#define INFINITE            0xFFFFFFFF  // Infinite timeout
  
  /* DUMMY */
#define CALLBACK
#define NEAR
#define FAR
  
  typedef struct
  {
    WORD  wFormatTag;
    WORD  nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD  nBlockAlign;
    WORD  wBitsPerSample;
    WORD  cbSize;
  }
  WAVEFORMATEX, *PWAVEFORMATEX, NEAR *NPWAVEFORMATEX, FAR *LPWAVEFORMATEX;
  
  /* wave data block header */
  typedef struct wavehdr_tag
  {
    LPSTR       lpData;                 /* pointer to locked data buffer */
    DWORD       dwBufferLength;         /* length of data buffer */
    DWORD       dwBytesRecorded;        /* used for input only */
    DWORD       dwUser;                 /* for client's use */
    DWORD       dwFlags;                /* assorted flags (see defines) */
    DWORD       dwLoops;                /* loop control counter */
    struct wavehdr_tag FAR *lpNext;     /* reserved for driver */
    DWORD       reserved;               /* reserved for driver */
  }
  WAVEHDR, *PWAVEHDR, NEAR *NPWAVEHDR, FAR *LPWAVEHDR;
  
  DECLARE_HANDLE(HWAVEOUT);
  
  typedef HWAVEOUT FAR *LPHWAVEOUT;
  
#ifdef __cplusplus
}
#endif
#endif  // #ifdef ENABLE_DUMMY_SPEAKER

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class Mutex;
        class WorkerQueue;
        class AudioBuffer;
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
        void CALLBACK __waveOutProc(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);
        /**
         * DeviceSpeaker used to playback audio samples on LINUX
         */
        class DeviceSpeakerLINUX: public DeviceSpeaker
        {
          public:
            /**
             * Start audio playback.
             *
             * @param source the audio source that contains the samples to play.
             * @param returnCode the return code
             */
            virtual void start(AudioStreamProxy& source, ReturnCode::Type& returnCode);
            /**
             * Stops audio playback.
             *
             * @return SUCCESS if stop in progress
             * @param returnCode the return code
             */
            virtual void stop(ReturnCode::Type& returnCode);
            
            /**
             * set the playback codec. This must be called before start is called.
             * @param playbackCodec the codec to use for the playback operation.
             * @param returnCode the return code.
             */
            virtual void setCodec(Codec::Type playbackCodec, ReturnCode::Type& returnCode);
            
            /**
             * set the microphone listener.
             * @param listener the device speaker listener.
             * @param returnCode INVALID_STATE if the microphone state isn't IDLE
             */
            virtual void setListener(DeviceSpeakerListenerProxy& listener, ReturnCode::Type& returnCode);
            
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
             * Prevent assignment.
             */
            DeviceSpeakerLINUX& operator=(DeviceSpeakerLINUX&);
            
            /**
             * Prevent construction.
             */
            DeviceSpeakerLINUX(ReturnCode::Type& returnCode);
            
            /**
             * Prevent destruction.
             */
            virtual ~DeviceSpeakerLINUX();
            
            enum State
            {
              IDLE,
              STARTING,
              PLAYING,
              STOPPING,
              DRAINING
            };
            
            /**
             * Task executed when setListener() is called.
             * @param _listener the proxy to the a device speaker listener.
             */
            void runSetDeviceSpeakerListenerTask(DeviceSpeakerListenerProxy& _listener);
            
            /**
             * Task executed when Start() is called. It will start the playback of
             * audio.
             */
            void runStartPlaybackTask();
            
            /**
             * Task executed when the waveOut is started. After we receive the WOM_OPEN
             * event.
             */
            void runDeviceSpeakerStartedTask();
            
            /**
             * Task executed when the device speaker needs new data to play and none is
             * available for now.
             */
            void runReadAudioBufferTask();
            
            /**
             * Task executed when a buffer has been played. At this point we must
             * realese the buffer and try to submit another one.
             * @param pwhdr a pointer to the WAVEHDR object that contains the played
             * samples.
             */
            void runBufferPlayedTask(WAVEHDR* pwhdr);
            
            /**
             * Task excuted when Stop() is called. It will stop the playback of audio.
             */
            void runStopPlaybackTask();
            
            /**
             * Task excuted the playback is done. After we receive the WOM_CLOSE event.
             */
            void runDeviceSpeakerStoppedTask();
            
            /**
             * internal function that calls waveOutClose.
             * @param returnCode the return code.
             */
            void closeWaveOut(ReturnCode::Type& returnCode);
            
            /**
             * internal function that tries to read AudioBuffer from the AudioStreamImpl
             * stream.
             */
            void readBuffersFromStream();
            
            /**
             * Function that converts AudioBuffer into WAVEHDR buffers. If needed it
             * will also do the encoding.
             * @param pBuf the audio buffer that contains the data to play.
             * @param iBuf the index of the buffer to fill with pBuf within the m_whs
             * array.
             * @param returnCode the returnCode.
             */
            void prepareBuffer(utilities::AudioBuffer* pBuf, int iBuf, ReturnCode::Type& returnCode);
            
            
            /**
             * calls waveOutWrite and check for errors
             * @param uiIndex buffer to add.
             * @param returnCode the returnCode.
             */
            void writeWaveOutBuffer(unsigned int uiIndex, ReturnCode::Type& returnCode);
            
            /**
             * Utility method to do cleanup and send the OnError callback to the
             * application.
             * @param returnCode the returnCode.
             * @param stopPlayback call stop playback if state == PLAYING || state == STARTING
             */
            void sendOnErrorToListener(ReturnCode::Type error, bool stopPlayback);
            
            /**
             * Utility method to do cleanup and send the OnStopped callback to the
             * application.
             */
            void sendOnStoppedToListener();
            
            /**
             * Utility function responsible for cleanup up the AudioStreamImplProxy code.
             */
            void cleanupAudioStreamProxy(ReturnCode::Type& returnCode);
            
            
            static ComponentInitializer componentInitializer;
            /**
             * maximum number of buffers that are queued at any time. You can view this as
             * a circular queue that contains NUM_BUFFERS_QUEUED buffers.
             */
            const static int NUM_BUFFERS_QUEUED = 8;
            /**
             * Collect BUFFERS_TO_COLLECT before we start playing.
             */
            const static int BUFFERS_TO_COLLECT = 4;
            /**
             * audio format in which we have to play.
             */
            Codec::Type codec;
            /**
             * the listener of Device Speaker events.
             */
            DeviceSpeakerListenerProxy listener;
            /**
             * worker queue used to run tasks.
             */
            utilities::WorkerQueue* workerQueue;
            /**
             * The AudioStream being played.
             */
            AudioStreamImplProxy audioStream;
            
            /**
             * Handle to the waveOut api
             */
            HWAVEOUT m_hwo;
            /**
             * Contains the audio format: encoding, bit rate, etc for the waveOut api.
             */
            WAVEFORMATEX m_wfx;
            /**
             * Index of the last buffer that was added to m_whs.
             */
            int firstSent;
            /**
             * Number of buffers that are currently queued into the waveOut api.
             */
            int numBuffers;
            /**
             * The circular queue of WAVEHDR buffers.
             */
            WAVEHDR m_whs[NUM_BUFFERS_QUEUED];
            /**
             * Flag that indicates if we have to collect buffers. Before the playback
             * starts, we have to collect a minimum of BUFFERS_TO_COLLECT buffers.
             */
            bool collectingBuffers;
            /**
             * current state
             */
            State state;
            static utilities::Mutex* mutex;
            
            SmartProxy::Root* rootProxy;
            
            
            
            friend void CALLBACK __waveOutProc(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);
            friend class SetDeviceSpeakerListenerTask;
            friend class StartPlaybackTask;
            friend class StopPlaybackTask;
            friend class BufferPlayedTask;
            friend class DeviceSpeakerStartedTask;
            friend class DeviceSpeakerStoppedTask;
            friend class ReadAudioBufferTask;
            friend class DeviceSpeaker;
            friend class DeviceSpeakerLINUXProxy;
        };
        
        /*
         * @see android::speech::recognition::SmartProxy
         */
        DECLARE_SMARTPROXY(UAPI_EXPORT, DeviceSpeakerLINUXProxy, DeviceSpeakerProxy, DeviceSpeakerLINUX)
        
        /**
         * Task executed when setListener() is called.
         */
        class SetDeviceSpeakerListenerTask: public utilities::Task
        {
          public:
            SetDeviceSpeakerListenerTask(DeviceSpeakerLINUXProxy& _deviceSpeaker,
                                         DeviceSpeakerListenerProxy & _listener);
            virtual void run();
          private:
            DeviceSpeakerLINUXProxy deviceSpeaker;
            DeviceSpeakerListenerProxy listener;
        };
        
        /**
         * Task executed when Start() is called. It will start the playback of audio.
         */
        class StartPlaybackTask: public utilities::Task
        {
          public:
            StartPlaybackTask(DeviceSpeakerLINUXProxy& deviceSpeaker);
            virtual void run();
          private:
            DeviceSpeakerLINUXProxy deviceSpeaker;
        };
        
        /**
         * Task excuted when Stop() is called. It will stop the playback of audio.
         */
        class StopPlaybackTask: public utilities::Task
        {
          public:
            StopPlaybackTask(DeviceSpeakerLINUXProxy& deviceSpeaker);
            virtual void run();
          private:
            DeviceSpeakerLINUXProxy deviceSpeaker;
        };
        
        /**
         * Task executed when a buffer has been played. At this point we must realese
         * the buffer and try to submit another one.
         */
        class BufferPlayedTask: public utilities::Task
        {
          public:
            BufferPlayedTask(DeviceSpeakerLINUXProxy& deviceSpeaker, WAVEHDR* waveHdr);
            virtual void run();
          private:
            DeviceSpeakerLINUXProxy deviceSpeaker;
            WAVEHDR* waveHdr;
        };
        
        /**
         * Task executed when the waveOut is started. After we receive the WOM_OPEN
         * event.
         */
        class DeviceSpeakerStartedTask: public utilities::Task
        {
          public:
            DeviceSpeakerStartedTask(DeviceSpeakerLINUXProxy& deviceSpeaker);
            virtual void run();
          private:
            DeviceSpeakerLINUXProxy deviceSpeaker;
        };
        
        /**
         * Task excuted the playback is done. After we receive the WOM_CLOSE event.
         */
        class DeviceSpeakerStoppedTask: public utilities::Task
        {
          public:
            DeviceSpeakerStoppedTask(DeviceSpeakerLINUXProxy& deviceSpeaker);
            virtual void run();
          private:
            DeviceSpeakerLINUXProxy deviceSpeaker;
        };
        
        /**
         * Task executed when the device speaker needs new data to play and none is
         * available for now.
         */
        class ReadAudioBufferTask: public utilities::ScheduledTask
        {
          public:
            ReadAudioBufferTask(DeviceSpeakerLINUXProxy& deviceSpeaker,  UINT32 timeout);
            virtual void run();
          private:
            DeviceSpeakerLINUXProxy deviceSpeaker;
        };
      }
    }
  }
}

#endif
