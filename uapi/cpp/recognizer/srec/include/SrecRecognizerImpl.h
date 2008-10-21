/*---------------------------------------------------------------------------*
 *  SrecRecognizerImpl.h                                                     *
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

#ifndef __UAPI__SRECRECOGNIZERIMPL
#define __UAPI__SRECRECOGNIZERIMPL

#include "exports.h"
#include "EmbeddedRecognizer.h"
#include "Task.h"
#include "AudioStreamImpl.h"
#include "RecognizerListener.h"

#include "SR_Recognizer.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class AudioSource;
      class GrammarListener;
      class GrammarListenerProxy;
      namespace utilities
      {
        class WorkerQueue;
        class Queue;
        class AudioBuffer;
      }
      namespace impl
      {
        class VoicetagItemImpl;
      }
      namespace srec
      {
        class SrecGrammarImpl;
        class SrecGrammarImplProxy;
      }
    }
  }
}

//typedef struct PFile_t PFile;
typedef struct SR_Recognizer_t SR_Recognizer;
typedef struct SR_AcousticModels_t SR_AcousticModels;
typedef struct SR_Vocabulary_t SR_Vocabulary;
typedef struct SR_Grammar_t SR_Grammar;
typedef struct SR_RecognizerResult_t SR_RecognizerResult;


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace srec
      {
        class SrecRecognizerImplProxy;
        /**
         * SREC speech recognizer.
         */
        class SrecRecognizerImpl: public EmbeddedRecognizer
        {
          public:
            /**
             * Creates a new SREC recognizer.
             *
             * @param returnCode the return code
             */
            static SrecRecognizerImplProxy create(ReturnCode::Type& returnCode);
            /**
             * Configures the embedded recognizer.
             *
             * @param config recognizer configuration file
             * @param params optional parameters
             * @param returnCode ILLEGAL_ARGUMENT if config is null. OPEN_ERROR, or
             * READ_ERROR if the recognizer configuration, acoustic model, or vocabulary files could not
             * be opened or read.
             */
            virtual void configure(const char* config, ReturnCode::Type& returnCode);
            
            /**
             * Sets the recognizer listener.
             *
             * @param listener recognizer listener
             * @param returnCode returns SUCCESS unless a fatal error occurs
             */
            virtual void setListener(RecognizerListenerProxy& listener,
                                     ReturnCode::Type& returnCode);
                                     
          /**
           * Creates a grammar.
           *
           * @param value the contents of the grammar
           * @param listener the grammar listener
           * @param returnCode ILLEGAL_ARGUMENT if value is null
           */
          virtual GrammarProxy createGrammar(
              const char* value, GrammarListenerProxy& listener, ReturnCode::Type& returnCode);


                                               
            /**
             * Begins speech recognition.
             *
             * @param audio the audio to recognizer
             * @param grammars a collection of grammar sets to recognize against
             * @param grammarCount the number of grammar sets
             * @param returnCode ILLEGAL_ARGUMENT if audio is null or is being used by another component,
             * or if grammars is null or if grammarCount < 1. NOT_SUPPORTED if grammarCount > 1
             */
            virtual void recognize(AudioStreamProxy& audio, GrammarProxy* grammars,
                                   ARRAY_LIMIT grammarCount,
                                   ReturnCode::Type& returnCode);
                                   
            /**
             * This convenience method is equivilent to invoking
             * recognize(audio, grammars, grammarCount, returnCode) with a single grammar.
             *
             * @param audio the audio to recognizer
             * @param grammar a grammar to recognize against
             * @param returnCode ILLEGAL_ARGUMENT if audio is null or is being used by another component,
             * or if grammar is null.
             * @see recognize(audio, grammars, grammarCount, returnCode)
             */
            virtual void recognize(AudioStreamProxy& audio, GrammarProxy& grammar,
                                   ReturnCode::Type& returnCode);
                                   
            /**
             * Terminates a recognition if one is in-progress.
             *
             * @param returnCode the return code
             */
            virtual void stop(ReturnCode::Type& returnCode);
            
            /**
             * Set parameter(s).
             *
             * @param keys parameter keys
             * @param values parameter values
             * @param count the number of parameters
             * @param returnCode the return code
             */
            virtual void setParameters(const char** keys, const char** values,
                                       ARRAY_LIMIT count, ReturnCode::Type& returnCode);
                                       
            /**
             * Get one or more parameter(s).
             *
             * @param keys parameter keys
             * @param count the number of parameters
             * @param returnCode the return code
             */
            virtual void getParameters(const char** keys, ARRAY_LIMIT count,
                                       ReturnCode::Type& returnCode);
                                       
            /**
             * The recognition accuracy improves over time as the recognizer adapts to the surrounding
             * environment. This method enables developers to reset the adaptation when the environment
             * is known to have changed.
             *
             * @param returnCode the return code
             */
            virtual void resetAcousticState(ReturnCode::Type& returnCode);
            /**
            * isSRBoolParameter() 
            * @param id of the parameter
            * @return bool true if the parameter is set and false otherwise
            */
            bool isSRBoolParameter(const char *id);

            /**
            * isRecognizing() 
            * @return bool true if State == RECOGNIZING, false otherwise
            */
            bool isRecognizing();
          private:
            enum State
            {
              IDLE,
              RECOGNIZING
            };
            
            
            /**
             * Prevent destruction.
             */
            virtual ~SrecRecognizerImpl();
            /**
             * Prevent construction.
             */
            SrecRecognizerImpl(ReturnCode::Type& returnCode);
            /**
             * Prevent assignment.
             */
            SrecRecognizerImpl& operator=(SrecRecognizerImpl&);
            /**
             * Binds a grammar to the recognizer.
             *
             * @param grammar the grammar
             * @param returnCode returns SUCCESS unless a fatal error occurs
             */
            void bind(SrecGrammarImplProxy& grammar, ReturnCode::Type& returnCode);
            
            /**
             * Unbinds the grammar from the associated recognizer.
             *
             * @param grammar the grammar
             * @param returnCode NO_MATCH if the specified grammar is not bound to the recognizer
             */
            void unbind(SrecGrammarImplProxy& grammar, ReturnCode::Type& returnCode);
            
            /**
             * Invoked by SetListenerTask
             */
            void onSetListener(RecognizerListenerProxy& listener);
            
            /**
             * Invoked by RecognizeTask.
             */
            void onRecognize(impl::AudioStreamImplProxy& _audioStream,
                             SrecGrammarImplProxy* grammarsProxy, ARRAY_LIMIT grammarCount);
                             
            /**
             * Invoked by StopTask.
             */
            void onStop();
            
            /**
             * Invoked by ReadAudioTask.
             */
            void onReadAudio(GrammarListenerProxy* grammarListeners,
                             ARRAY_LIMIT listenerCount);
                             
            /**
             * Invoked by ResetAcousticStateTask.
             */
            void onResetAcousticState();
            
            /**
             * Invoked by SetParametersTask.
             */
            void onSetParameters(const char** keys, const char** values, ARRAY_LIMIT count);
            
            /**
             * Invoked by GetParametersTask.
             */
            void onGetParameters(const char** keys, ARRAY_LIMIT count);
            
            /**
             * proxy call to SR_RecognizerIsActiveRule
             */
            void RecognizerIsActiveRule(SR_Grammar* grammar, ESR_BOOL* wasActive,
                                        ReturnCode::Type& returnCode);
                                        
            /**
             * Shuts down srec.
             */
            void stopSREC();
            /**
             * Fires the necessary events whenever recogStatus changes values.
             */
            void handleRecogStatus(GrammarListenerProxy* grammarListeners, ARRAY_LIMIT listenerCount);
            
            /**
             * Invoked after the recognizer shuts down. Contains
             * RecognizerStopAndCleanup + onStopped callback.
             */
            void onStopped(ReturnCode::Type& returnCode);
            
            /**
             * Invoked when the recognition is done and we want to cleanup
             * resources and Stop the recognizer. No callback.
             */
            void RecognizerStopAndCleanup(ReturnCode::Type& returnCode);
            
            /**
             * The event listener.
             */
            RecognizerListenerProxy listener;
            /**
             * The underlying recognizer.
             */
            SR_Recognizer* recognizer;
            /**
             * The vocabulary.
             */
            SR_Vocabulary* vocabulary;
            /**
             * The thread and the queue used to process aysnc tasks.
             */
            utilities::WorkerQueue* workerQueue;
            /**
             * All grammars associated with the recognizer.
             */
            utilities::Queue* grammars;
            /**
             * The result type returned by the last recognition step.
             */
            SR_RecognizerResultType resultType;
            /**
             * The recognizer status returned by the last recognition step.
             */
            SR_RecognizerStatus recogStatus;
            /**
             * The recognition result returned by the last recognition step.
             */
            SR_RecognizerResult* result;
            /**
             * State of the recognizer
             */
            State state;
            /**
             * protect the state variable
             */
            utilities::Mutex* mutex;
            
            android::speech::recognition::impl::AudioStreamImplProxy audioStream;
            SmartProxy::Root* rootProxy;
            
            /**
             * If Recognizer.stop() is invoked we need to cancel any pending tasks related to the
             * current recognition.
             */
            android::speech::recognition::utilities::Task* pendingTask;
            
            friend class android::speech::recognition::impl::VoicetagItemImpl;
            friend class android::speech::recognition::srec::SrecGrammarImpl;
            friend class SetListenerTask;
            friend class RecognizeTask;
            friend class AudioReadyTask;
            friend class ReadAudioTask;
            friend class StopTask;
            friend class ResetAcousticStateTask;
            friend class SetParametersTask;
            friend class GetParametersTask;
            friend class SrecRecognizerImplProxy;
        };
        
        /**
         * @see android::speech::recognition::SmartProxy
         */
        DECLARE_SMARTPROXY(SREC_EXPORT, SrecRecognizerImplProxy, EmbeddedRecognizerProxy, SrecRecognizerImpl)
        
        class SetListenerTask: public utilities::Task
        {
          public:
            SetListenerTask(SrecRecognizerImplProxy& _recognizer, RecognizerListenerProxy & _listener);
            virtual void run();
          private:
            SrecRecognizerImplProxy recognizer;
            RecognizerListenerProxy listener;
        };
        
        class RecognizeTask: public utilities::Task
        {
          public:
            RecognizeTask(SrecRecognizerImplProxy& recognizer,
                          impl::AudioStreamImplProxy& audioStream,
                          SrecGrammarImplProxy* grammars,
                          ARRAY_LIMIT grammarCount);
                          
            virtual ~RecognizeTask();
            virtual void run();
          private:
            SrecRecognizerImplProxy recognizer;
            impl::AudioStreamImplProxy audioStream;
            SrecGrammarImplProxy* grammars;
            ARRAY_LIMIT grammarCount;
        };
        
        class ReadAudioTask: public utilities::ScheduledTask
        {
          public:
            ReadAudioTask(SrecRecognizerImplProxy& recognizer,
                          UINT32 timeout, GrammarListenerProxy* grammarListeners,
                          ARRAY_LIMIT listenerCount);
                          
            virtual ~ReadAudioTask();
            virtual void run();
          private:
            SrecRecognizerImplProxy recognizer;
            GrammarListenerProxy* grammarListeners;
            ARRAY_LIMIT listenerCount;
        };
        
        class StopTask: public utilities::Task
        {
          public:
            StopTask(SrecRecognizerImplProxy& recognizer);
            virtual void run();
          private:
            SrecRecognizerImplProxy recognizer;
        };
        
        class ResetAcousticStateTask: public utilities::Task
        {
          public:
            ResetAcousticStateTask(SrecRecognizerImplProxy& recognizer);
            virtual void run();
          private:
            SrecRecognizerImplProxy recognizer;
        };
        
        class SetParametersTask: public utilities::Task
        {
          public:
            SetParametersTask(SrecRecognizerImplProxy& recognizer, char** keys, char** values,
                              ARRAY_LIMIT count);
            ~SetParametersTask();
            virtual void run();
          private:
            SrecRecognizerImplProxy recognizer;
            char** keys;
            char** values;
            ARRAY_LIMIT count;
        };
        
        class GetParametersTask: public utilities::Task
        {
          public:
            GetParametersTask(SrecRecognizerImplProxy& recognizer, char** keys, ARRAY_LIMIT count);
            ~GetParametersTask();
            virtual void run();
          private:
            SrecRecognizerImplProxy recognizer;
            char** keys;
            ARRAY_LIMIT count;
        };
      }
    }
  }
}

/**
 * Creates an EmbeddedRecognizer across library-space.
 */
extern "C" SREC_EXPORT android::speech::recognition::EmbeddedRecognizerProxy* ConfigureEmbeddedRecognizer(
    const char* config, android::speech::recognition::ReturnCode::Type& returnCode);
    
#endif
