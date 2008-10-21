/*---------------------------------------------------------------------------*
 *  EmbeddedGrammarListener.h                                                *
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

#ifndef __UAPI__EMBEDDEDGRAMMARLISTENER
#define __UAPI__EMBEDDEDGRAMMARLISTENER

#include "exports.h"
#include "ReturnCode.h"
#include "GrammarListener.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      /**
       * Listens for EmbeddedGrammar events.
       */
      class EmbeddedGrammarListener: public GrammarListener
      {
        public:
          UAPI_EXPORT virtual ~EmbeddedGrammarListener();
          
          /**
           * Invoked after the grammar is saved.
           *
           * @param path the path the grammar was saved to
           */
          virtual void onSaved(const char* path) = 0;
          
          /**
           * Invokes after all grammar slots have been compiled.
           */
          virtual void onCompileAllSlots() = 0;
          
          /**
           * Invokes after all grammar slots have been reset.
           */
          virtual void onResetAllSlots() = 0;
          
          /**
           * Invoked when an unexpected error occurs. This is normally followed by
           * onStopped() if the component shuts down successfully.
           *
           * @param returnCode GRAMMAR_SLOT_FULL if an item cannot be added into a grammar slot
           * because it is full.<br/>
           * NOT_SUPPORTED if different words with the same pronunciation are added.<br/>
           * INVALID_STATE if reseting or compiling the slots fails.<br/>
           * READ_ERROR if the grammar could not be loaded.<br/>
           * WRITE_ERROR if the grammar could not be saved.
           */
          virtual void onError(ReturnCode::Type returnCode) = 0;
      };
      /**
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, EmbeddedGrammarListenerProxy, GrammarListenerProxy, EmbeddedGrammarListener)
    }
  }
}

#endif
