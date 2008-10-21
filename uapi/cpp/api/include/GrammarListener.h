/*---------------------------------------------------------------------------*
 *  GrammarListener.h                                                        *
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

#ifndef __UAPI__GRAMMARLISTENER
#define __UAPI__GRAMMARLISTENER

#include "exports.h"
#include "ReturnCode.h"
#include "SmartProxy.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      /**
       * Listens for Grammar events.
       */
      class GrammarListener
      {
        public:
          UAPI_EXPORT virtual ~GrammarListener();
          
          /**
           * Invoked after the grammar is loaded.
           */
          virtual void onLoaded() = 0;
          
          /**
           * Invoked after the grammar is unloaded.
           */
          virtual void onUnloaded() = 0;
          
          /**
           * Invoked when a grammar operation fails.
           *
           * @param returnCode READ_ERROR if the grammar could not be loaded.<br/>
           * WRITE_ERROR if the grammar could not be saved.
           */
          virtual void onError(ReturnCode::Type returnCode) = 0;
          
          /**
           * Returns true if the listener is an EmbeddedGrammarListener.
           *
           * @return true if the listener is an EmbeddedGrammarListener
           */
          virtual bool isEmbeddedGrammarListener() = 0;

           /**
           * Returns true if the listener is an SrecGrammarListener.
           *
           * @return true if the listener is an SrecGrammarListener
           */
          virtual bool isSrecGrammarListener() = 0;
      };
      /**
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, GrammarListenerProxy, SmartProxy, GrammarListener)
    }
  }
}
#endif
