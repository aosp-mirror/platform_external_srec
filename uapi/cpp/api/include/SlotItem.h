/*---------------------------------------------------------------------------*
 *  SlotItem.h                                                               *
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

#ifndef __UAPI__SLOTITEM
#define __UAPI__SLOTITEM

#include "exports.h"
#include "ReturnCode.h"
#include "SmartProxy.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace srec
      {
        class SrecGrammarImpl;
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
      class SlotItemProxy;
      /**
       * Item that may be inserted into an embedded grammar slot.
       */
      class UAPI_EXPORT SlotItem
      {
        public:
          /**
           * Returns true if the item is a word.
           */
          virtual bool isWord() const = 0;
          
          /**
           * Returns true if the item is a voicetag.
           */
          virtual bool isVoicetag() const = 0;
          
        protected:
          /**
           * Prevent construction.
           */
          SlotItem();
          /**
           * Prevent destruction.
           */
          virtual ~SlotItem();
          
          friend class android::speech::recognition::srec::SrecGrammarImpl; // used by SrecGrammarImpl::addItem()
          friend class SlotItemProxy;
      };
      
      /*
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, SlotItemProxy, android::speech::recognition::SmartProxy, SlotItem)
    }
  }
}

#endif
