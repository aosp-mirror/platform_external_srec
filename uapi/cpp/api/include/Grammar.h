/*---------------------------------------------------------------------------*
 *  Grammar.h                                                                *
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

#ifndef __UAPI__GRAMMAR
#define __UAPI__GRAMMAR

#include "exports.h"
#include "ReturnCode.h"
#include "SmartProxy.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class GrammarProxy;
      /**
       * Speech recognition grammar.
       */
      class Grammar
      {
        public:
          

          /**
           * Indicates that the grammar will be used in the near future.
           *
           * @param returnCode ILLEGAL_ARGUMENT if the associated recognizer has been deleted
           */
          virtual void load(ReturnCode::Type& returnCode) = 0;
          /**
           * Indicates that the grammar won't be used in the near future.
           *
           * @param returnCode ILLEGAL_ARGUMENT if the associated recognizer has been deleted
           */
          virtual void unload(ReturnCode::Type& returnCode) = 0;
        protected:
          /**
           * Prevent construction.
           */
          UAPI_EXPORT Grammar();
          /**
           * Prevent destruction.
           */
          UAPI_EXPORT virtual ~Grammar();
          
          friend class GrammarProxy;
      };
      
      /*
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, GrammarProxy, android::speech::recognition::SmartProxy, Grammar)
    }
  }
}

#endif
