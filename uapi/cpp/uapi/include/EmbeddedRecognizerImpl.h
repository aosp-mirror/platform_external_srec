/*---------------------------------------------------------------------------*
 *  EmbeddedRecognizerImpl.h                                                 *
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

#ifndef _UAPI_EMBEDDEDRECOGNIZER_H
#define _UAPI_EMBEDDEDRECOGNIZER_H

#include "EmbeddedRecognizer.h"
#include "Singleton.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class System;
      namespace utilities
      {
        class Mutex;
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
        class EmbeddedRecognizerImpl: public EmbeddedRecognizer, public Singleton
        {
          public:
            /**
             * Configures the embedded recognizer.
             *
             * @param config recognizer configuration file
             * @param returnCode ILLEGAL_ARGUMENT if config is null.<br/>
             * OPEN_ERROR, or READ_ERROR if the recognizer configuration, acoustic model, or
             * vocabulary files could not be opened or read.
             */
            virtual void configure(const char* config, ReturnCode::Type& returnCode);
            
            /**
             * Sets the recognizer listener.
             *
             * @param listener recognizer listener
             * @param returnCode returns SUCCESS unless a fatal error occurs
             */
            virtual void setListener(RecognizerListenerProxy& listener, ReturnCode::Type& returnCode);
            
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
             * or if grammars is null or if grammarCount < 1.<br/>
             * NOT_SUPPORTED if the recognizer does not support the number of grammars specified.
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
            
            /**
             * Returns the SmartProxy root associated with the singleton.
             *
             * @return the SmartProxy root associated with the singleton
             */
            virtual SmartProxy::Root* getRoot();
            
            /**
             * Invoked when the singleton has to shutdown.
             *
             * @param returnCode the return code.
             */
            virtual void shutdown(ReturnCode::Type& returnCode);
            
            /**
             * Prevent construction.
             */
            EmbeddedRecognizerImpl();
            
            /**
             * Prevent destruction.
             */
            virtual ~EmbeddedRecognizerImpl();
            
            static ComponentInitializer componentInitializer;
            static EmbeddedRecognizerImpl* instance;
            SmartProxy::Root* rootProxy;
            EmbeddedRecognizerProxy delegate;
            static utilities::Mutex* mutex;
            
            /**
             * Library loader of the SRecRecognizer. UAPI_srec.dll or UAPI_srec.so.
             * It is put here such that the library is unloaded only after all the
             * objects have been deleted.
             */
            static utilities::LibraryLoader* srecLoader;
            
            friend class EmbeddedRecognizer;
            friend class android::speech::recognition::System;
        };
      }
    }
  }
}

#endif
