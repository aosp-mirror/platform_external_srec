/*---------------------------------------------------------------------------*
 *  Recognizer.h                                                             *
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

#ifndef __UAPI__RECOGNIZER
#define __UAPI__RECOGNIZER

#include "exports.h"
#include "types.h"
#include "SmartProxy.h"
#include "Grammar.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class GrammarProxy;
      class GrammarListenerProxy;
      class AudioStreamProxy;
      class ParametersProxy;
      class RecognizerListenerProxy;
    }
  }
}


//
// DESIGN NOTES:
//
// @see http://ciaranm.org/show_post/146 for a discussion of shared_ptr and covariant return-types
//
namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class RecognizerProxy;
      /**
       * Speech recognizer.
       */
      class Recognizer
      {
        public:
          /**
           * Sets the recognizer listener.
           *
           * @param listener recognizer listener
           * @param returnCode returns SUCCESS unless a fatal error occurs
           */
          virtual void setListener(RecognizerListenerProxy& listener,
                                   ReturnCode::Type& returnCode) = 0;

          /**
           * Creates a grammar.
           *
           * @param value the contents of the grammar
           * @param listener the grammar listener
           * @param returnCode ILLEGAL_ARGUMENT if value is null
           */
          virtual GrammarProxy createGrammar(const char* value, GrammarListenerProxy& listener, ReturnCode::Type& returnCode) = 0;

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
                                 ReturnCode::Type& returnCode) = 0;
                                 
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
                                 ReturnCode::Type& returnCode) = 0;
                                 
          /**
           * Terminates a recognition if one is in-progress.
           *
           * @param returnCode the return code
           */
          virtual void stop(ReturnCode::Type& returnCode) = 0;
          
          /**
           * Set parameter(s).
           *
           * @param keys parameter keys
           * @param values parameter values
           * @param count the number of parameters
           * @param returnCode the return code
           */
          virtual void setParameters(const char** keys, const char** values,
                                     ARRAY_LIMIT count, ReturnCode::Type& returnCode) = 0;
                                     
          /**
           * Get one or more parameter(s).
           *
           * @param keys parameter keys
           * @param count the number of parameters
           * @param returnCode the return code
           */
          virtual void getParameters(const char** keys, ARRAY_LIMIT count,
                                     ReturnCode::Type& returnCode) = 0;
        protected:
          /**
           * Prevent construction.
           */
          Recognizer();
          /**
           * Prevent destruction.
           */
          virtual ~Recognizer();
          
          friend class RecognizerProxy;
      };
      
      /*
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, RecognizerProxy, android::speech::recognition::SmartProxy, Recognizer)
    }
  }
}

#endif
