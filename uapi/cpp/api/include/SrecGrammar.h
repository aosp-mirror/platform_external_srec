/*---------------------------------------------------------------------------*
 *  SrecGrammar.h                                                            *
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

#ifndef __UAPI__SRECGRAMMAR
#define __UAPI__SRECGRAMMAR

#include "exports.h"
#include "EmbeddedGrammar.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class SlotItemProxy;
    }
  }
}


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class SrecGrammarProxy;
      
      /**
       * Grammar on an SREC recognizer.
       */
      class SrecGrammar: public EmbeddedGrammar
      {
        public:
          /**
           * Adds an item to a slot.
           *
           * @param slotName the name of the slot
           * @param item the item to add to the slot.
           * @param weight the weight of the item. Smaller values are more likely to get recognized. This should be >= 0.
           * @param semanticMeaning the value that will be returned if this item is recognized. This should
           * be of the form "V='Jen_Parker'"
           * @param returnCode ILLEGAL_ARGUMENT if slotName, item or semanticMeaning are null or if
           * semanticMeaning is not in the form "key=value". ILLEGAL_STATE if the associated
           * recognizer has been deleted. HOMONYM_COLLISION if another item with the same
           * pronunciation already exists in the slot.
           */
          UAPI_EXPORT virtual void addItem(const char* slotName, const SlotItemProxy& item,
                                           int weight, const char* semanticMeaning,
                                           ReturnCode::Type& returnCode) = 0;

           /**
           * Add a list of item to a slot.
           *
           * @param slotName the name of the slot
           * @param items the array of SlotItems to add to the slot.
           * @param weights the array of weights for each item in the list. Smaller values are more likely to get recognized.  This should be >= 0.
           * @param semanticMeanings the array of strings that will be returned for each item during recognition.
           * @param itemsCount number of items in the list
           * @throws IllegalArgumentException if slotName, items, weights or semanticMeanings are null;if any semanticMeaning of the list is not of the format "V=&#039;Jen_Parker&#039; if the size of list parameters is not equal."
           */
           UAPI_EXPORT virtual void addItemList(const char* slotName, 
                                            SlotItemProxy** items,
                                            int* weights,
                                            const char** semanticMeanings,
                                            ARRAY_LIMIT itemsCount,
                                            ReturnCode::Type& returnCode) = 0;

        protected:
          /**
           * Prevent construction.
           */
          UAPI_EXPORT SrecGrammar();
          /**
           * Prevent destruction.
           */
          UAPI_EXPORT virtual ~SrecGrammar();
          
          friend class SrecGrammarProxy;
      };
      
      /*
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, SrecGrammarProxy, EmbeddedGrammarProxy, SrecGrammar)
    }
  }
}

#endif
