/*---------------------------------------------------------------------------*
 *  EmbeddedGrammar.h                                                        *
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

#ifndef __UAPI__EMBEDDEDGRAMMAR
#define __UAPI__EMBEDDEDGRAMMAR

#include "exports.h"
#include "ReturnCode.h"
#include "Grammar.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class EmbeddedGrammarProxy;
      /**
       * Grammar on an embedded recognizer.
       */
      class EmbeddedGrammar: public Grammar
      {
        public:
          /**
           * Compiles items that were added to any of the grammar slots.
           *
           * @param returnCode ILLEGAL_ARGUMENT if the associated recognizer has been deleted
           */
          virtual void compileAllSlots(ReturnCode::Type& returnCode) = 0;
          
          /**
           * Removes all words added to all slots.
           *
           * @param returnCode ILLEGAL_ARGUMENT if the associated recognizer has been deleted
           */
          virtual void resetAllSlots(ReturnCode::Type& returnCode) = 0;
          
          /**
           * Saves the compiled grammar.
           *
           * @param path the path to save the grammar to
           * @param returnCode ILLEGAL_ARGUMENT if the associated recognizer has been deleted
           */
          virtual void save(const char* path, ReturnCode::Type& returnCode) = 0;
        protected:
          /**
           * Prevent construction.
           */
          UAPI_EXPORT EmbeddedGrammar();
          /**
           * Prevent destruction.
           */
          UAPI_EXPORT virtual ~EmbeddedGrammar();
          
          friend class EmbeddedGrammarProxy;
      };
      
      /*
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, EmbeddedGrammarProxy, GrammarProxy, EmbeddedGrammar)
    }
  }
}

#endif
