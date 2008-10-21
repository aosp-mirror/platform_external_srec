/*---------------------------------------------------------------------------*
 *  SrecRecognizerImpl.cpp                                                   *
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

#include "SrecRecognizerImpl.h"
#include "EmbeddedGrammar.h"
#include "AudioStream.h"
#include "AudioStreamImpl.h"
#include "SrecGrammarImpl.h"
#include "SrecGrammar.h"
#include "RecognizerListener.h"
#include "SrecHelper.h"
#include "SrecRecognitionResultImpl.h"
#include "RecognitionResult.h"
#include "AudioSource.h"
#include "AudioBuffer.h"
#include "GrammarListener.h"
#include "WorkerQueue.h"
#include "Logger.h"
#include "ConditionVariable.h"
#include "WorkerQueueFactory.h"
#include "Queue.h"
#include "System.h"


#include "ESR_ReturnCode.h"
#include "PFileSystem.h"
#include "plog.h"
#include "pmemory.h"
#include "PANSIFileSystem.h"
#include "SR_Session.h"
#include "SR_Recognizer.h"
#include "ESR_Session.h"
#include "SR_AcousticState.h"
#include "ptypes.h"

// Adding a counter to report failed consecutive requests for audio to prevent the recognizer from hanging.
// s also errors out if we hit the max. SteveR
#define NO_AUDIO_BUFFERS_WARNING_COUNT	90
#define NO_AUDIO_BUFFERS_ERROR_MARGIN	10
#define NO_AUDIO_BUFFERS_ERROR_COUNT	( NO_AUDIO_BUFFERS_WARNING_COUNT + NO_AUDIO_BUFFERS_ERROR_MARGIN )

using namespace android::speech::recognition;
using namespace android::speech::recognition::impl;
using namespace android::speech::recognition::utilities;


EmbeddedRecognizerProxy* ConfigureEmbeddedRecognizer(const char* config, ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("ConfigureEmbeddedRecognizer");
    
  // Ensure that the function signature matches its associated typedef
  ::android::speech::recognition::ConfigureEmbeddedRecognizer test = ::ConfigureEmbeddedRecognizer;
  
  // Nonsense instruction to suppress warning C4189: "local variable is
  // initialized but not referenced"
  if (test)
    test = 0;
    
  if (config == 0)
  {
    returnCode = ReturnCode::ILLEGAL_ARGUMENT;
    return 0;
  }
  returnCode = ReturnCode::SUCCESS;
  srec::SrecRecognizerImplProxy recognizer = srec::SrecRecognizerImpl::create(returnCode);
  if (returnCode)
    return 0;
  srec::SrecRecognizerImplProxy* result = new srec::SrecRecognizerImplProxy(recognizer);
  if (!result || !*result)
  {
    delete result;
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return 0;
  }
  recognizer->configure(config, returnCode);
  if (returnCode)
  {
    delete result;
    return 0;
  }
  return result;
}

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace srec
      {
        DEFINE_SMARTPROXY(srec, SrecRecognizerImplProxy, EmbeddedRecognizerProxy, SrecRecognizerImpl)
        
        SrecRecognizerImpl::SrecRecognizerImpl(ReturnCode::Type& returnCode):
            recognizer(0),
            vocabulary(0),
            workerQueue(0),
            grammars(0),
            state(IDLE),
            rootProxy(0),
            pendingTask(0)
        {
          UAPI_FN_NAME("SrecRecognizerImpl::SrecRecognizerImpl");
            
          UAPI_TRACE(fn,"this=%p\n", this);
          
          grammars = new Queue();
          if (grammars == 0)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          mutex = Mutex::create(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Failed to create mutex\n");
            return;
          }
          
          returnCode = ReturnCode::SUCCESS;
        }
        
        SrecRecognizerImplProxy SrecRecognizerImpl::create(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::create");
            
          SrecRecognizerImpl* object = new SrecRecognizerImpl(returnCode);
          if (!object)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return SrecRecognizerImplProxy();
          }
          else if (returnCode)
          {
            delete object;
            return SrecRecognizerImplProxy();
          }
          SrecRecognizerImplProxy result(object);
          if (!result)
            returnCode = ReturnCode::OUT_OF_MEMORY;
          object->rootProxy = result.getRoot();
          return result;
        }
        
        SrecRecognizerImpl::~SrecRecognizerImpl()
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::~SrecRecognizerImpl");
            
          UAPI_TRACE(fn,"this=%p\n", this);
          
          stopSREC();
          delete grammars;
          delete mutex;
        }
        
        void SrecRecognizerImpl::configure(const char* config, ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::configure");
            
          WorkerQueueFactory* workerFactory = 0;
          if (config == 0)
          {
            returnCode = ReturnCode::ILLEGAL_ARGUMENT;
            return;
          }
          
          //need to protect when we are assigning _state (done in WorkerQueue
          //thread) and when we are calling configure (done in the app
          //thread).
          LockScope ls(mutex, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Failed to create LockScope\n");
            return;
          }
          
          if (state != IDLE)
          {
            UAPI_ERROR(fn,"Must be in IDLE state to call configure\n");
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          
          // Shut down the engine in case it is already loaded
          stopSREC();
          
          //
          // Initialize portable library.
          // Can't use CHKLOG() before plogInit, so use non-portable methods instead.
          //
          ESR_ReturnCode rc;
          size_t len;
          ESR_BOOL bValue;
          
          ESR_SessionExists(&bValue);
          if (!bValue)
          {
            // Initialize the portable library
            PFile* outputLog;
            PFile* memFile;
            
            CHKLOG(rc, PMemInit());
            PLogger* logger;
            outputLog = PSTDOUT;
            CHKLOG(rc, PLogCreateFileLogger(outputLog, &logger));
            CHKLOG(rc, PLogInit(logger, 0));
            memFile = PSTDOUT;
            CHKLOG(rc, PMemSetLogFile(memFile));
            CHKLOG(rc, PLogSetFormat(LOG_OUTPUT_FORMAT_DATE_TIME | LOG_OUTPUT_FORMAT_THREAD_ID | LOG_OUTPUT_FORMAT_MODULE_NAME));
            
            CHKLOG(rc, SR_SessionCreate(config));
          }
          CHKLOG(rc, SR_RecognizerCreate(&recognizer));
          
          // Load acoustic models
          LCHAR filenames[P_PATH_MAX];
          len = P_PATH_MAX;
          CHKLOG(rc, ESR_SessionGetLCHAR(L("cmdline.modelfiles"), filenames, &len));
          CHKLOG(rc, SR_RecognizerSetup(recognizer));
          
          // Create vocabulary object and associate with grammar
          len = P_PATH_MAX;
          LCHAR filename[P_PATH_MAX];
          CHKLOG(rc, ESR_SessionGetLCHAR(L("cmdline.vocabulary"), filename, &len));
          CHKLOG(rc, SR_VocabularyLoad(filename, &vocabulary));
          
          // Load acoustic state
          //len = P_PATH_MAX;
          //CHKLOG(rc, ESR_SessionGetLCHAR(L("cmdline.state"), filename, &len));
          //CHKLOG(rc, SR_AcousticStateLoad(recognizer, filename));
          
          // start a new log session
          CHKLOG(rc, SR_RecognizerLogSessionStart(recognizer, L("SRecTest.session1")));
          
          workerFactory = WorkerQueueFactory::getInstance(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not get a WorkerQueueFactory instance\n");
            goto CLEANUP;
          }
          
          workerQueue = workerFactory->getWorkerQueue(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not create the worker queue\n");
            goto CLEANUP;
          }
          return;
CLEANUP:
          returnCode = SrecHelper::toUAPI(rc);
          return;
        }
        
        void SrecRecognizerImpl::stopSREC()
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::stopSREC");
            
          if (recognizer)
          {
            SR_RecognizerStop(recognizer);
            SR_RecognizerLogSessionEnd(recognizer);
            SR_RecognizerUnsetup(recognizer);
            SR_RecognizerDestroy(recognizer);
            recognizer = 0;
          }
          if (vocabulary)
          {
            SR_VocabularyDestroy(vocabulary);
            vocabulary = 0;
          }
          
          // Save acoustic state
          //LCHAR filename[P_PATH_MAX];
          //len = P_PATH_MAX;
          //CHKLOG(rc, ESR_SessionGetLCHAR(L("cmdline.state"), filename, &len));
          //CHKLOG(rc, SR_AcousticStateSave(recognizer, filename));
          
          ESR_BOOL bValue;
          ESR_SessionExists(&bValue);
          if (bValue)
            SR_SessionDestroy();
            
          // Shutdown portable library
          PLogShutdown();
          PMemShutdown();
        }
        
        GrammarProxy SrecRecognizerImpl::createGrammar(
              const char* value, GrammarListenerProxy& listener, ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::createGrammar");
            
          if ( value == 0 || 
               (strstr(value, "http://") != 0) )
          {
            returnCode = ReturnCode::ILLEGAL_ARGUMENT;
            return GrammarProxy();
          }
          SrecRecognizerImplProxy proxy(rootProxy);
          if (!proxy)
            return GrammarProxy();
            
          return SrecGrammarImpl::create(value, listener, proxy,
                                         workerQueue, returnCode);
        }
        
        void SrecRecognizerImpl::handleRecogStatus(GrammarListenerProxy* grammarListeners, ARRAY_LIMIT listenerCount)
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("SrecRecognizerImpl::handleRecogStatus");
            
          UAPI_INFO(fn,"recogStatus %d\n", recogStatus);
          
          switch (recogStatus)
          {
            case SR_RECOGNIZER_EVENT_INVALID:
            {
              // this should never happen
              assert(false);
              break;
            }
            case SR_RECOGNIZER_EVENT_INCOMPLETE:
            {
              // do nothing
              break;
            }
            case SR_RECOGNIZER_EVENT_NEED_MORE_AUDIO:
            {
              // do nothing
              break;
            }
            case SR_RECOGNIZER_EVENT_STARTED:
            {
              if (listener)
                listener->onStarted();
              break;
            }
            case SR_RECOGNIZER_EVENT_STOPPED:
            {
              onStopped(returnCode);
              if (returnCode != ReturnCode::SUCCESS && listener)
                listener->onError(returnCode);
              break;
            }
            case SR_RECOGNIZER_EVENT_START_OF_VOICING:
            {
              if (listener)
                listener->onBeginningOfSpeech();
              break;
            }
            case SR_RECOGNIZER_EVENT_END_OF_VOICING:
            {
              if (listener)
                listener->onEndOfSpeech();
              break;
            }
            case SR_RECOGNIZER_EVENT_RECOGNITION_RESULT:
            {
              SrecRecognitionResultImplProxy uapiResult = SrecRecognitionResultImpl::create(result, returnCode);
              if (returnCode != ReturnCode::SUCCESS)
              {
                if (listener)
                  listener->onError(returnCode);
                break;
              }
              if (listener)
                listener->onRecognitionSuccess(uapiResult);
              onStopped(returnCode);
              if (returnCode != ReturnCode::SUCCESS && listener)
                listener->onError(returnCode);
              break;
            }
            case SR_RECOGNIZER_EVENT_NO_MATCH:
            {
              if (listener)
                listener->onRecognitionFailure(RecognizerListener::NO_MATCH);
              onStopped(returnCode);
              if (returnCode != ReturnCode::SUCCESS && listener)
                listener->onError(returnCode);
              break;
            }
            case SR_RECOGNIZER_EVENT_SPOKE_TOO_SOON:
            {
              if (listener)
                listener->onRecognitionFailure(RecognizerListener::SPOKE_TOO_SOON);
              onStopped(returnCode);
              if (returnCode != ReturnCode::SUCCESS && listener)
                listener->onError(returnCode);
              break;
            }
            case SR_RECOGNIZER_EVENT_START_OF_UTTERANCE_TIMEOUT:
            {
              if (listener)
                listener->onRecognitionFailure(RecognizerListener::BEGINNING_OF_SPEECH_TIMEOUT);
              onStopped(returnCode);
              if (returnCode != ReturnCode::SUCCESS && listener)
                listener->onError(returnCode);
              break;
            }
            case SR_RECOGNIZER_EVENT_RECOGNITION_TIMEOUT:
            {
              if (listener)
                listener->onRecognitionFailure(RecognizerListener::RECOGNITION_TIMEOUT);
              onStopped(returnCode);
              if (returnCode != ReturnCode::SUCCESS && listener)
                listener->onError(returnCode);
              break;
            }
            case SR_RECOGNIZER_EVENT_MAX_SPEECH:
            {
              if (listener)
                listener->onRecognitionFailure(RecognizerListener::TOO_MUCH_SPEECH);
              onStopped(returnCode);
              if (returnCode != ReturnCode::SUCCESS && listener)
                listener->onError(returnCode);
              break;
            }
            default:
            {
              // this should never happen
              assert(false);
              break;
            }
          }
        }
        
        void SrecRecognizerImpl::onStopped(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::onStopped");
            
          RecognizerStopAndCleanup(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
            return;
            
          if (listener != 0)
            listener->onStopped();
            
          returnCode = ReturnCode::SUCCESS;
        }
        
        void SrecRecognizerImpl::RecognizerStopAndCleanup(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::RecognizerStopAndCleanup");
            

          
          audioStream->unlock();
          audioStream = AudioStreamImplProxy();
          
          {
            LockScope ls(mutex, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
              UAPI_WARN(fn,"Failed to create LockScope\n");
              return;
            }
            state = IDLE;
          }
          
          ESR_ReturnCode rc = SR_RecognizerStop(recognizer);
          if (rc != ESR_SUCCESS)
          {
            returnCode = SrecHelper::toUAPI(rc);
            return;
          }
          
          FwdIterator iter(grammars->begin(), grammars->end());
          while (iter.hasNext())
          {
            SrecGrammarImplProxy* grammar = (SrecGrammarImplProxy*) iter.next();
            
            unbind(*grammar, returnCode);
            delete grammar;
            if (returnCode != ReturnCode::SUCCESS)
              return;
          }

          returnCode = ReturnCode::SUCCESS;
        }
        
        void SrecRecognizerImpl::onReadAudio(GrammarListenerProxy* grammarListeners,
                                             ARRAY_LIMIT listenerCount)
        {
          ReturnCode::Type returnCode;
	  static int num_consecutive_no_audio_msgs = 0;	// Count to prevent hang on no audio.
          UAPI_FN_SCOPE("SrecRecognizerImpl::onReadAudio");
            
          ESR_ReturnCode rc;
          AudioBuffer* buffer;
          
          pendingTask = 0;
          buffer = audioStream->read(returnCode);
          if (buffer)
          {
	    num_consecutive_no_audio_msgs = 0;	// Reset counter, we only care about consecutive failures.
            UAPI_INFO(fn,"Srec got buffer %p\n", buffer);
            assert(buffer->size % 2 == 0);
            size_t samplesLeft = buffer->size / 2;
            do
            {
              if (samplesLeft > 0)
              {
                size_t samplesRead = samplesLeft;
                rc = SR_RecognizerPutAudio(recognizer, (asr_int16_t*) buffer->buffer, &samplesRead,
                                           ESR_FALSE);
                if (rc != ESR_BUFFER_OVERFLOW && rc != ESR_SUCCESS)
                {
                  audioStream->release(buffer, returnCode);
                  if (returnCode != ReturnCode::SUCCESS)
                    UAPI_ERROR(fn,"Failed to release audio buffer\n");
                  returnCode = SrecHelper::toUAPI(rc);
                  goto CLEANUP;
                }
                else
                  samplesLeft -= samplesRead;
              }
              
              rc = SR_RecognizerAdvance(recognizer, &recogStatus, &resultType, &result);
              if (rc != ESR_SUCCESS)
              {
                PLogError(ESR_rc2str(rc));
                recogStatus = SR_RECOGNIZER_EVENT_STOPPED;
                resultType = SR_RECOGNIZER_RESULT_TYPE_COMPLETE;
                returnCode = SrecHelper::toUAPI(rc);
                goto CLEANUP;
              }
              
              if (recogStatus != SR_RECOGNIZER_EVENT_INCOMPLETE)
              {
                // We are done with the audio buffer, Release it.
                audioStream->release(buffer, returnCode);
                if (returnCode != ReturnCode::SUCCESS)
                {
                  UAPI_ERROR(fn,"Failed to release audio buffer\n");
                  goto CLEANUP;
                }
              }
              handleRecogStatus(grammarListeners, listenerCount);
            }
            while (recogStatus == SR_RECOGNIZER_EVENT_INCOMPLETE);
            
            // Try reading again as soon as possible
            if (resultType != SR_RECOGNIZER_RESULT_TYPE_COMPLETE)
            {
              GrammarListenerProxy* grammarListenersCopy = new GrammarListenerProxy[listenerCount];
              for (ARRAY_LIMIT i = 0; i < listenerCount; ++i)
                grammarListenersCopy[i] = grammarListeners[i];
                
              SrecRecognizerImplProxy proxy(rootProxy);
              if (!proxy)
              {
                UAPI_ERROR(fn,"Failed to create proxy\n");
                returnCode = ReturnCode::INVALID_STATE;
                delete[] grammarListenersCopy;
                goto CLEANUP;
              }
              pendingTask = new ReadAudioTask(proxy, 0, grammarListenersCopy, listenerCount);
              if (!pendingTask)
              {
                UAPI_ERROR(fn,"Could not create ReadAudioTask\n");
                returnCode = ReturnCode::OUT_OF_MEMORY;
                delete[] grammarListenersCopy;
                goto CLEANUP;
              }
              workerQueue->enqueue(pendingTask, returnCode);
              if (returnCode != ReturnCode::SUCCESS)
              {
                UAPI_ERROR(fn,"Could not enqueue ReadAudioTask\n");
                delete pendingTask; //grammarListenersCopy is deleted here as well.
                pendingTask = 0;
                goto CLEANUP;
              }
              
            }
          }
          else
          {
            if (returnCode == ReturnCode::PENDING_DATA)
            {
	    num_consecutive_no_audio_msgs++;

            if ( num_consecutive_no_audio_msgs == NO_AUDIO_BUFFERS_ERROR_COUNT )
              {
                returnCode = ReturnCode::INVALID_STATE;
                UAPI_ERROR(fn,"Did Not Receive Audio For %d Requests\n", num_consecutive_no_audio_msgs );
                if ( listener )
                  listener->onRecognitionFailure ( RecognizerListener::BEGINNING_OF_SPEECH_TIMEOUT );
                onStopped(returnCode);
                if (returnCode != ReturnCode::SUCCESS && listener)
                   listener->onError(returnCode);
                goto CLEANUP_2;
              }
            else if ( num_consecutive_no_audio_msgs >= NO_AUDIO_BUFFERS_WARNING_COUNT )
              {
                UAPI_WARN(fn,"Did Not Receive Audio For %d Requests\n", num_consecutive_no_audio_msgs );
              }
              // Try reading a bit later
              GrammarListenerProxy* grammarListenersCopy = new GrammarListenerProxy[listenerCount];
              for (ARRAY_LIMIT i = 0; i < listenerCount; ++i)
                grammarListenersCopy[i] = grammarListeners[i];
                
              SrecRecognizerImplProxy proxy(rootProxy);
              if (!proxy)
              {
                returnCode = ReturnCode::INVALID_STATE;
                delete[] grammarListenersCopy;
                goto CLEANUP;
              }
// The time value has been changed from 100 milliseconds down to 10 milliseconds to
// improve latency. The slight increase in work is negligible compared to the huge
// gain in reducing latency. This change is tied to the modified size of the audio
// buffers to 256 samples in the Google audio in code.  SteveR
// Changed value to 25 because buffers are larger and we're getting time outs. SteveR
              pendingTask = new ReadAudioTask(proxy, 25, grammarListenersCopy, listenerCount);
              if (!pendingTask)
              {
                UAPI_ERROR(fn,"Could not create ReadAudioTask\n");
                returnCode = ReturnCode::OUT_OF_MEMORY;
                delete[] grammarListenersCopy;
                goto CLEANUP;
              }
              workerQueue->enqueue(pendingTask, returnCode);
              if (returnCode != ReturnCode::SUCCESS)
              {
                UAPI_ERROR(fn,"Could not enqueue ReadAudioTask\n");
                delete pendingTask; //grammarListenersCopy is deleted here as well.
                pendingTask = 0;
                goto CLEANUP;
              }
            }
            else if (returnCode == ReturnCode::END_OF_STREAM)
            {
              size_t samplesRead = 0;
              rc = SR_RecognizerPutAudio(recognizer, 0, &samplesRead, ESR_TRUE);
              if (rc != ESR_SUCCESS)
              {
                returnCode = SrecHelper::toUAPI(rc);
                goto CLEANUP;
              }
              do
              {
                rc = SR_RecognizerAdvance(recognizer, &recogStatus, &resultType, &result);
                if (rc != ESR_SUCCESS)
                {
                  PLogError(ESR_rc2str(rc));
                  returnCode = SrecHelper::toUAPI(rc);
                  goto CLEANUP;
                }
                handleRecogStatus(grammarListeners, listenerCount);
              }
              while (resultType != SR_RECOGNIZER_RESULT_TYPE_COMPLETE);
            }
          }
          return;
CLEANUP:
          ReturnCode::Type dummy;
          RecognizerStopAndCleanup(dummy);
          //don't care about rc, we already failed. Only report returnCode
          
          if (listener)
          {
            listener->onError(returnCode);
          }
CLEANUP_2:
// Just a place holder, because all work will be done above. SteveR
          recogStatus = SR_RECOGNIZER_EVENT_STOPPED;
          resultType = SR_RECOGNIZER_RESULT_TYPE_COMPLETE;
        }
        
        void SrecRecognizerImpl::RecognizerIsActiveRule(SR_Grammar* grammar, ESR_BOOL* wasActive,
            ReturnCode::Type& returnCode)
        {
          ESR_ReturnCode rc;
          rc = SR_RecognizerIsActiveRule(recognizer, grammar, 0, wasActive);
          if (rc != ESR_SUCCESS)
          {
            returnCode = SrecHelper::toUAPI(rc);
            return;
          }
          returnCode = ReturnCode::SUCCESS;
        }
        
        bool SrecRecognizerImpl::isSRBoolParameter(const char *id)
        {
            ESR_ReturnCode  rc = ESR_SUCCESS;
            ESR_BOOL val= ESR_FALSE;
            rc = SR_RecognizerGetBoolParameter(recognizer, id, &val );
            if (rc != ESR_SUCCESS)
                return false;
            return (val==ESR_TRUE)?true:false;
        }

        bool SrecRecognizerImpl::isRecognizing()
        {
            return (state == RECOGNIZING);
        }
        void SrecRecognizerImpl::recognize(AudioStreamProxy& audio, GrammarProxy* grammars,
                                           ARRAY_LIMIT grammarCount,
                                           ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::recognize");
            
          AudioStreamImplProxy& audioStreamImpl = (AudioStreamImplProxy&) audio;
          
          if (!audioStreamImpl || grammars == 0 || grammarCount < 1)
          {
            returnCode = ReturnCode::ILLEGAL_ARGUMENT;
            return;
          }
          else if (grammarCount > 1)
          {
            returnCode = ReturnCode::NOT_SUPPORTED;
            return;
          }
          size_t recognizerSampleRate;
          ESR_ReturnCode rc;
          rc = ESR_SessionGetSize_t(L("CREC.Frontend.samplerate"), &recognizerSampleRate);
          if (rc != ESR_SUCCESS)
          {
            returnCode = SrecHelper::toUAPI(rc);
            return;
          }
          UINT16 audioSampleRate = Codec::getSampleRate(audioStreamImpl->getCodec(), returnCode);
          if (returnCode != ReturnCode::SUCCESS)
            return;
            
          if (recognizerSampleRate != audioSampleRate)
          {
            UAPI_ERROR(fn, "Invalid sample rate! Recognizer: %d, Audio: %d\n", recognizerSampleRate, audioSampleRate);
            returnCode = ReturnCode::ILLEGAL_ARGUMENT;
            return;
          }
          
          //Flag this AudioStreamImpl as being locked. This means that this audio cannot be
          //passed to other modules.
          audioStreamImpl->lock(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not lock audio stream\n");
            return;
          }
          
          // Copy the grammars argument, we want to use SrecGrammarImplProxy to
          // make sure we don't use a grammar that was deleted.
          SrecGrammarImplProxy* grammarsCopy = new SrecGrammarImplProxy[grammarCount];
          for (ARRAY_LIMIT i = 0; i < grammarCount; ++i)
            grammarsCopy[i] = (SrecGrammarImplProxy) grammars[i];
            
          SrecRecognizerImplProxy proxy(rootProxy);
          if (!proxy)
          {
            UAPI_ERROR(fn,"Could not create proxy\n");
            returnCode = ReturnCode::INVALID_STATE;
            delete[] grammarsCopy;
            audioStreamImpl->unlock();
            return;
          }
          RecognizeTask* task = new RecognizeTask(proxy, audioStreamImpl, grammarsCopy, grammarCount);
          if (!task)
          {
            UAPI_ERROR(fn,"Could not create RecognizeTask\n");
            returnCode = ReturnCode::OUT_OF_MEMORY;
            delete[] grammarsCopy;
            audioStreamImpl->unlock();
            return;
          }
          
          workerQueue->enqueue(task, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not enqueue RecognizeTask\n");
            delete task; // will delete grammarsCopy too
            audioStreamImpl->unlock();
            return;
          }
          
          return;
        }
        
        void SrecRecognizerImpl::recognize(AudioStreamProxy& audio, GrammarProxy& grammar,
                                           ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::recognize2");
            
          GrammarProxy* grammars = new GrammarProxy[1];
          if (grammars == 0)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          grammars[0] = grammar;
          recognize(audio, grammars, 1, returnCode);
          delete[] grammars;
        }
        
        void SrecRecognizerImpl::onRecognize(AudioStreamImplProxy& _audioStream,
                                             SrecGrammarImplProxy* _grammars,
                                             ARRAY_LIMIT grammarCount)
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("SrecRecognizerImpl::onRecognize");
            
          ESR_ReturnCode rc;
          
          GrammarListenerProxy* grammarListeners = new GrammarListenerProxy[grammarCount];
          grammars->clear();
          
          if (state != IDLE)
          {
            UAPI_ERROR(fn,"Must be in IDLE state to call recognize\n");
            returnCode = ReturnCode::INVALID_STATE;
            goto CLEANUP;
          }
          
          state = RECOGNIZING;
          audioStream = _audioStream;
          
          for (ARRAY_LIMIT i = 0; i < grammarCount; ++i)
          {
            SrecGrammarImplProxy& grammar = _grammars[i];
            grammarListeners[i] = grammar->_listener;
            if (!grammar->isLoaded())
            {
              returnCode = ReturnCode::INVALID_STATE;
              goto CLEANUP;
            }
            bind(grammar, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
              goto CLEANUP;
            SrecGrammarImplProxy* temp = new SrecGrammarImplProxy(grammar);
            if (returnCode)
            {
              delete temp;
              goto CLEANUP;
            }
            grammars->push(temp, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
              goto CLEANUP;
          }
          CHKLOG(rc, SR_RecognizerStart(recognizer));
          
          // Try reading as soon as possible
          {
            SrecRecognizerImplProxy proxy(rootProxy);
            if (!proxy)
            {
              UAPI_ERROR(fn,"Could not create proxy\n");
              returnCode = ReturnCode::INVALID_STATE;
              goto CLEANUP;
            }
            pendingTask = new ReadAudioTask(proxy, 0, grammarListeners, grammarCount);
            if (!pendingTask)
            {
              UAPI_ERROR(fn,"Could not create ReadAudioTask\n");
              returnCode = ReturnCode::OUT_OF_MEMORY;
              goto CLEANUP;
            }
            workerQueue->enqueue(pendingTask, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
              UAPI_ERROR(fn,"Could not enqueue RecognizeTask\n");
              delete pendingTask;
              pendingTask = 0;
              goto CLEANUP;
            }
          }
          return;
CLEANUP:
          ReturnCode::Type temp;
          RecognizerStopAndCleanup(temp);
          
          delete[] grammarListeners;
          if (listener)
          {
            listener->onError(returnCode);
          }
          return;
        }
        
        void SrecRecognizerImpl::onStop()
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("SrecRecognizerImpl::onStop");
            
          //check the state.
          if (state == IDLE)
          {
            //already stopped. Do nothing.
            UAPI_WARN(fn,"Recognizer is already stopped, nothing to do.\n");
            return;
          }
          
          onStopped(returnCode);
          if (returnCode != ReturnCode::SUCCESS && listener)
            listener->onError(returnCode);
          if (pendingTask)
          {
            workerQueue->remove(pendingTask, returnCode);
            if (returnCode != ReturnCode::SUCCESS && listener)
              listener->onError(returnCode);
            pendingTask = 0;
          }
        }
        
        void SrecRecognizerImpl::stop(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::stop");
            
          SrecRecognizerImplProxy proxy(rootProxy);
          if (!proxy)
          {
            UAPI_ERROR(fn,"Could not create proxy\n");
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          StopTask* task = new StopTask(proxy);
          if (!task)
          {
            UAPI_ERROR(fn,"Could not create StopTask\n");
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          workerQueue->enqueue(task, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not enqueue StopTask\n");
            delete task;
            return;
          }
        }
        
        void SrecRecognizerImpl::setParameters(const char** keys, const char** values,
                                               ARRAY_LIMIT count, ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::setParameters");
            
          SetParametersTask* task = 0;
          
          SrecRecognizerImplProxy proxy(rootProxy);
          if (!proxy)
          {
            UAPI_ERROR(fn,"Could not create proxy\n");
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          char** keysCopy = new char*[count];
          if (!keysCopy)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          char** valuesCopy = new char*[count];
          if (!valuesCopy)
          {
            delete[] keysCopy;
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          for (ARRAY_LIMIT i = 0; i < count; ++i)
          {
            keysCopy[i] = 0;
            valuesCopy[i] = 0;
          }
          for (ARRAY_LIMIT i = 0; i < count; ++i)
          {
            keysCopy[i] = new char[strlen(keys[i]) + 1];
            if (!keysCopy[i])
            {
              returnCode = ReturnCode::OUT_OF_MEMORY;
              goto CLEANUP;
            }
            strcpy(keysCopy[i], keys[i]);
            
            valuesCopy[i] = new char[strlen(values[i]) + 1];
            if (!valuesCopy[i])
            {
              returnCode = ReturnCode::OUT_OF_MEMORY;
              goto CLEANUP;
            }
            strcpy(valuesCopy[i], values[i]);
          }
          
          task = new SetParametersTask(proxy, keysCopy, valuesCopy, count);
          if (!task)
          {
            UAPI_ERROR(fn,"Could not create SetParametersTask\n");
            returnCode = ReturnCode::OUT_OF_MEMORY;
            goto CLEANUP;
          }
          workerQueue->enqueue(task, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not enqueue SetParametersTask\n");
            delete task;
            goto CLEANUP;
          }
          return;
CLEANUP:
          for (ARRAY_LIMIT i = 0; i < count; ++i)
          {
            delete[] keysCopy[i];
            delete[] valuesCopy[i];
          }
          delete[] keysCopy;
          delete[] valuesCopy;
        }
        
        void SrecRecognizerImpl::onSetParameters(const char** keys, const char** values, ARRAY_LIMIT count)
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("SrecRecognizerImpl::onSetParameters");
            
          ARRAY_LIMIT invalidIndex = 0;
          ARRAY_LIMIT validIndex = 0;
          
          // NOTE: validKeys, validValues, invalidKeys, invalidValues point to elements of keys and values
          // variables, no copies are made.
          const char** validKeys = 0;
          const char** validValues = 0;
          const char** invalidKeys = 0;
          const char** invalidValues = 0;
          
          if (state != IDLE)
          {
            UAPI_ERROR(fn,"Must be in IDLE state to call SetParameters\n");
            returnCode = ReturnCode::INVALID_STATE;
            goto CLEANUP;
          }
          validKeys = new const char*[count];
          if (!validKeys)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            goto CLEANUP;
          }
          validValues = new const char*[count];
          if (!validValues)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            goto CLEANUP;
          }
          invalidKeys = new const char*[count];
          if (!invalidKeys)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            goto CLEANUP;
          }
          invalidValues = new const char*[count];
          if (!invalidValues)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            goto CLEANUP;
          }
          
          for (ARRAY_LIMIT i = 0; i < count; ++i)
          {
            ESR_ReturnCode rc;
            if (strcmp(keys[i], "SREC.Recognizer.utterance_timeout") == 0 ||
                strcmp(keys[i], "CREC.Frontend.samplerate") == 0 ||
                strcmp(keys[i], "CREC.Recognizer.terminal_timeout") == 0 ||
                strcmp(keys[i], "CREC.Recognizer.optional_terminal_timeout") == 0 ||
                strcmp(keys[i], "CREC.Recognizer.non_terminal_timeout") == 0 ||
                strcmp(keys[i], "CREC.Recognizer.eou_threshold") == 0 ||
                strcmp(keys[i], "enableGetWaveform") == 0)
            {
              char* endptr;
              long convertedValue = strtol(values[i], &endptr, 10);
              if (strlen(values[i]) == 0 || endptr != values[i] + strlen(values[i]) ||
                  convertedValue == LONG_MAX || convertedValue == LONG_MIN)
              {
                returnCode = ReturnCode::ILLEGAL_ARGUMENT;
                UAPI_ERROR(fn,"Value must be an integer. Key=%s, value=%s\n", keys[i], values[i]);
                invalidKeys[invalidIndex] = keys[i];
                invalidValues[invalidIndex] = values[i];
                ++invalidIndex;
                continue;
              }
              if (strcmp(keys[i], "enableGetWaveform") == 0)
              {
                  ESR_BOOL val= (ESR_BOOL) convertedValue;
                  rc = SR_RecognizerSetBoolParameter(recognizer, keys[i], val);
              }
              else
              {
                 rc = SR_RecognizerSetSize_tParameter(recognizer, keys[i], convertedValue);
              }
              if (rc != ESR_SUCCESS)
              {
                returnCode = SrecHelper::toUAPI(rc);
                invalidKeys[invalidIndex] = keys[i];
                invalidValues[invalidIndex] = values[i];
                ++invalidIndex;
              }
              else
              {
                validKeys[validIndex] = keys[i];
                validValues[validIndex] = values[i];
                ++validIndex;
              }
            }
	    else if ( strcmp(keys[i], "CREC.Frontend.swicms.cmn") == 0 )
            {
              rc = SR_AcousticStateSet ( recognizer, values [i] );

              if (rc != ESR_SUCCESS)
              {
                returnCode = SrecHelper::toUAPI(rc);
                invalidKeys[invalidIndex] = keys[i];
                invalidValues[invalidIndex] = values[i];
                ++invalidIndex;
              }
              else
              {
                validKeys[validIndex] = keys[i];
                validValues[validIndex] = values[i];
                ++validIndex;
              }
            }
            else
            {
              UAPI_ERROR(fn,"Unsupported key: %s\n", keys[i]);
              invalidKeys[invalidIndex] = keys[i];
              invalidValues[invalidIndex] = values[i];
              ++invalidIndex;
            }
          }
          if (listener)
          {
            if (invalidIndex > 0)
              listener->onParametersSetError(invalidKeys, invalidValues, invalidIndex, ReturnCode::NOT_SUPPORTED);
            if (invalidIndex > 0 || validIndex > 0)
              listener->onParametersSet(validKeys, validValues, validIndex);
          }
          returnCode = ReturnCode::SUCCESS;
CLEANUP:
          if (validKeys)
            delete[] validKeys;
          if (validValues)
            delete[] validValues;
          if (invalidKeys)
            delete[] invalidKeys;
          if (invalidValues)
            delete[] invalidValues;
          if (returnCode && listener)
            listener->onParametersSetError(keys, values, count, returnCode);
        }
        
        void SrecRecognizerImpl::getParameters(const char** keys, ARRAY_LIMIT count,
                                               ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::getParameters");
            
          GetParametersTask* task = 0;
          
          SrecRecognizerImplProxy proxy(rootProxy);
          if (!proxy)
          {
            UAPI_ERROR(fn,"Could not create proxy\n");
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          char** keysCopy = new char*[count];
          if (!keysCopy)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          for (ARRAY_LIMIT i = 0; i < count; ++i)
            keysCopy[i] = 0;
          for (ARRAY_LIMIT i = 0; i < count; ++i)
          {
            keysCopy[i] = new char[strlen(keys[i]) + 1];
            if (!keysCopy[i])
            {
              returnCode = ReturnCode::OUT_OF_MEMORY;
              goto CLEANUP;
            }
            strcpy(keysCopy[i], keys[i]);
          }
          
          task = new GetParametersTask(proxy, keysCopy, count);
          if (!task)
          {
            UAPI_ERROR(fn,"Could not create GetParametersTask\n");
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          workerQueue->enqueue(task, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not enqueue GetParametersTask\n");
            delete task;
            return;
          }
          return;
CLEANUP:
          for (ARRAY_LIMIT i = 0; i < count; ++i)
            delete[] keysCopy[i];
          delete[] keysCopy;
        }
        
        void SrecRecognizerImpl::onGetParameters(const char** keys, ARRAY_LIMIT count)
        {
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("SrecRecognizerImpl::onGetParameters");
            
          ARRAY_LIMIT invalidIndex = 0;
          ARRAY_LIMIT validIndex = 0;
          
          // NOTE: validKeys, invalidKeys point to elements of 'keys' variable, no copies are made
          const char** validKeys = 0;
          const char** invalidKeys = 0;
          
          char** validValues = 0;
          
          validKeys = new const char*[count];
          if (!validKeys)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            goto CLEANUP;
          }
          invalidKeys = new const char*[count];
          if (!invalidKeys)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            goto CLEANUP;
          }
          validValues = new char*[count];
          if (!validValues)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            goto CLEANUP;
          }
          for (ARRAY_LIMIT i = 0; i < count; ++i)
            validValues[i] = 0;
            
          for (ARRAY_LIMIT i = 0; i < count; ++i)
          {
            ESR_ReturnCode rc;

            if (strcmp(keys[i], "SREC.Recognizer.utterance_timeout") == 0 ||
                strcmp(keys[i], "CREC.Frontend.samplerate") == 0 ||
                strcmp(keys[i], "CREC.Recognizer.terminal_timeout") == 0 ||
                strcmp(keys[i], "CREC.Recognizer.optional_terminal_timeout") == 0 ||
                strcmp(keys[i], "CREC.Recognizer.non_terminal_timeout") == 0 ||
                strcmp(keys[i], "CREC.Recognizer.eou_threshold") == 0 ||
                strcmp(keys[i], "enableGetWaveform") == 0)
            {
              size_t value;
              if (strcmp(keys[i], "enableGetWaveform") == 0)
              {
                  ESR_BOOL val = ESR_FALSE;
                  rc = SR_RecognizerGetBoolParameter(recognizer, keys[i], &val);
                  value = (size_t) val;
              }
              else
                  rc = SR_RecognizerGetSize_tParameter(recognizer, keys[i], &value);
              if (rc != ESR_SUCCESS)
              {
                returnCode = SrecHelper::toUAPI(rc);
                invalidKeys[invalidIndex] = keys[i];
                ++invalidIndex;
              }
              else
              {
                validKeys[validIndex] = keys[i];
                validValues[validIndex] = new char[UINT32_DIGITS];
                if (!validValues[validIndex])
                {
                  returnCode = ReturnCode::OUT_OF_MEMORY;
                  goto CLEANUP;
                }
#ifdef UAPI_WIN32
                _snprintf(validValues[validIndex], UINT32_DIGITS, "%lu", value);
#else
                snprintf(validValues[validIndex], UINT32_DIGITS, "%lu", (unsigned long)value);
#endif
                ++validIndex;
              }
            }
	    else if ( strcmp(keys[i], "CREC.Frontend.swicms.cmn") == 0 )
            {
              char *returned_value;
	      int  value_len;

              rc = SR_AcousticStateGet ( recognizer, (const LCHAR **)&returned_value );

              if (rc != ESR_SUCCESS)
              {
                returnCode = SrecHelper::toUAPI(rc);
                invalidKeys[invalidIndex] = keys[i];
                ++invalidIndex;
              }
              else
              {
                value_len = strlen ( returned_value ) + 1;
                validKeys[validIndex] = keys[i];
                validValues[validIndex] = new char[value_len];

                if (!validValues[validIndex])
                {
                  returnCode = ReturnCode::OUT_OF_MEMORY;
                  goto CLEANUP;
                }
                memcpy ( validValues[validIndex], returned_value, value_len );
                ++validIndex;
              }
            }
            else
            {
              UAPI_ERROR(fn,"Unsupported key: %s\n", keys[i]);
              invalidKeys[invalidIndex] = keys[i];
              ++invalidIndex;
            }
          }
          if (listener)
          {
            if (invalidIndex > 0)
              listener->onParametersGetError(invalidKeys, invalidIndex, ReturnCode::NOT_SUPPORTED);
            if (invalidIndex > 0 || validIndex > 0)
              listener->onParametersGet(validKeys, (const char**) validValues, validIndex);
          }
          returnCode = ReturnCode::SUCCESS;
CLEANUP:
          if (validKeys)
            delete[] validKeys;
          if (invalidKeys)
            delete[] invalidKeys;
          if (validValues)
          {
            for (ARRAY_LIMIT i = 0; i < count; ++i)
              delete[] validValues[i];
            delete[] validValues;
          }
          if (returnCode && listener)
            listener->onParametersGetError(invalidKeys, count, returnCode);
        }
        
        void SrecRecognizerImpl::bind(SrecGrammarImplProxy& grammar, ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::bind");
            
          ESR_ReturnCode rc = ESR_SUCCESS;
          SR_Grammar* srecGrammar;
          
          // TODO: We happens if we invoke this multiple times per grammar?
          srecGrammar = grammar->grammar;
          CHKLOG(rc, SR_GrammarSetupVocabulary(srecGrammar, vocabulary));
          
          CHKLOG(rc, SR_GrammarSetupRecognizer(srecGrammar, recognizer));
          // -------------------------------
          
          CHKLOG(rc, SR_RecognizerActivateRule(recognizer, srecGrammar, 0, 0));
          returnCode = ReturnCode::SUCCESS;
          return;
CLEANUP:
          returnCode = SrecHelper::toUAPI(rc);
        }
        
        void SrecRecognizerImpl::unbind(SrecGrammarImplProxy& grammar, ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::unbind");
            
          ESR_ReturnCode rc;
          
          SR_Grammar* srecGrammar = grammar->grammar;
          CHKLOG(rc, SR_RecognizerDeactivateRule(recognizer, srecGrammar, 0));
          returnCode = ReturnCode::SUCCESS;
          return;
CLEANUP:
          returnCode = SrecHelper::toUAPI(rc);
        }
        
        void SrecRecognizerImpl::setListener(RecognizerListenerProxy& listener,
                                             ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::setListener");
            
          if (!listener)
          {
            //not accepting NULL listener
            UAPI_WARN(fn,"Listener cannot be NULL\n");
            returnCode = ReturnCode::ILLEGAL_ARGUMENT;
            return;
          }
          
          SrecRecognizerImplProxy proxy(rootProxy);
          if (!proxy)
          {
            UAPI_ERROR(fn,"Could not create proxy\n");
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          
          SetListenerTask* task = new SetListenerTask(proxy, listener);
          if (!task)
          {
            UAPI_ERROR(fn,"Could not create SetListenerTask\n");
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          
          workerQueue->enqueue(task, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not enqueue SetListenerTask\n");
            delete task;
            return;
          }
          
          returnCode = ReturnCode::SUCCESS;
        }
        
        void SrecRecognizerImpl::resetAcousticState(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::resetAcousticState");
            
          SrecRecognizerImplProxy proxy(rootProxy);
          if (!proxy)
          {
            UAPI_ERROR(fn,"Could not create proxy\n");
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          
          ResetAcousticStateTask* task = new ResetAcousticStateTask(proxy);
          if (!task)
          {
            UAPI_ERROR(fn,"Could not create ResetAcousticStateTask\n");
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          
          workerQueue->enqueue(task, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not enqueue ResetAcousticStateTask\n");
            delete task;
            return;
          }
          
          returnCode = ReturnCode::SUCCESS;
        }
        
        void SrecRecognizerImpl::onResetAcousticState()
        {
          ESR_ReturnCode rc;
          ReturnCode::Type returnCode;
          UAPI_FN_SCOPE("SrecRecognizerImpl::onResetAcousticState");

	  if (state != IDLE)
          {
            UAPI_ERROR(fn,"Must be in IDLE state to call ResetAcousticState\n");
            returnCode = ReturnCode::INVALID_STATE;
          }
	  else
          {
            rc = SR_AcousticStateReset(recognizer);
            returnCode = SrecHelper::toUAPI(rc);
          }
          if (listener)
          {
            if (returnCode)
              listener->onError(returnCode);
            else
              listener->onAcousticStateReset();
          }
        }
        
        void SrecRecognizerImpl::onSetListener(RecognizerListenerProxy& _listener)
        {
          UAPI_FN_SCOPE("SrecRecognizerImpl::onSetListener");
            
	  if (state != IDLE)
            UAPI_WARN(fn,"Should be in IDLE state to call SetListener\n");
          listener = _listener;
        }
        
        SetListenerTask::SetListenerTask(SrecRecognizerImplProxy& _recognizer,
                                         RecognizerListenerProxy & _listener):
            Task("SetListenerTask"),
            recognizer(_recognizer),
            listener(_listener)
        {}
        
        void SetListenerTask::run()
        {
          recognizer->onSetListener(listener);
        }
        
        RecognizeTask::RecognizeTask(SrecRecognizerImplProxy& _recognizer,
                                     AudioStreamImplProxy & _audioStream,
                                     SrecGrammarImplProxy* _grammars,
                                     ARRAY_LIMIT _grammarCount):
            Task("RecognizeTask"),
            recognizer(_recognizer),
            audioStream(_audioStream),
            grammars(_grammars),
            grammarCount(_grammarCount)
        {}
        
        RecognizeTask::~RecognizeTask()
        {
          delete[] grammars;
        }
        
        void RecognizeTask::run()
        {
          recognizer->onRecognize(audioStream, grammars, grammarCount);
        }
        
        ReadAudioTask::ReadAudioTask(SrecRecognizerImplProxy& _recognizer,
                                     UINT32 timeout, GrammarListenerProxy* _grammarListeners,
                                     ARRAY_LIMIT _listenerCount):
            ScheduledTask(timeout, "ReadAudioTask"),
            recognizer(_recognizer),
            grammarListeners(_grammarListeners),
            listenerCount(_listenerCount)
        {}
        
        ReadAudioTask::~ReadAudioTask()
        {
          delete[] grammarListeners;
        }
        
        void ReadAudioTask::run()
        {
          recognizer->onReadAudio(grammarListeners, listenerCount);
        }
        
        StopTask::StopTask(SrecRecognizerImplProxy& _recognizer):
            Task("StopTask"),
            recognizer(_recognizer)
        {}
        
        void StopTask::run()
        {
          recognizer->onStop();
        }
        
        ResetAcousticStateTask::ResetAcousticStateTask(SrecRecognizerImplProxy& _recognizer):
            Task("ResetAcousticStateTask"),
            recognizer(_recognizer)
        {}
        
        void ResetAcousticStateTask::run()
        {
          recognizer->onResetAcousticState();
        }
        
        SetParametersTask::SetParametersTask(SrecRecognizerImplProxy& _recognizer, char** _keys,
                                             char** _values, ARRAY_LIMIT _count):
            Task("SetParametersTask"),
            recognizer(_recognizer),
            keys(_keys),
            values(_values),
            count(_count)
        {}
        
        void SetParametersTask::run()
        {
          // Reasoning for (const char**) cast: http://coding.derkeiler.com/Archive/C_CPP/comp.lang.cpp/2004-10/1514.html
          recognizer->onSetParameters((const char**) keys, (const char**) values, count);
        }
        
        SetParametersTask::~SetParametersTask()
        {
          for (ARRAY_LIMIT i = 0; i < count; ++i)
            delete [] keys[i];
          delete [] keys;
          for (ARRAY_LIMIT i = 0; i < count; ++i)
            delete [] values[i];
          delete [] values;
        }
        
        GetParametersTask::GetParametersTask(SrecRecognizerImplProxy& _recognizer, char** _keys,
                                             ARRAY_LIMIT _count):
            Task("GetParametersTask"),
            recognizer(_recognizer),
            keys(_keys),
            count(_count)
        {}
        
        void GetParametersTask::run()
        {
          // Reasoning for (const char**) cast: http://coding.derkeiler.com/Archive/C_CPP/comp.lang.cpp/2004-10/1514.html
          recognizer->onGetParameters((const char**) keys, count);
        }
        
        GetParametersTask::~GetParametersTask()
        {
          for (ARRAY_LIMIT i = 0; i < count; ++i)
            delete [] keys[i];
          delete [] keys;
        }
      }
    }
  }
}
