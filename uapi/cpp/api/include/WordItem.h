/*---------------------------------------------------------------------------*
 *  WordItem.h                                                               *
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

#ifndef __UAPI__WORDITEM
#define __UAPI__WORDITEM

#include "exports.h"
#include "ReturnCode.h"
#include "types.h"
#include "SlotItem.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class WordItemProxy;
      /**
       * Word that may be inserted into an embedded grammar slot.
       */
      class UAPI_EXPORT WordItem: public SlotItem
      {
        public:
          /**
           * Creates a new WordItem.
           *
           * @param word the word to insert
           * @param pronunciations the pronunciations to associated with the item. If the list is
           * is empty the recognizer will attempt to guess the pronunciations.
           * @param pronunciationCount the number of pronunciations
           * @param returnCode the return code
           */
          static WordItemProxy create(const char* word, const char** pronunciations,
                                      ARRAY_LIMIT pronunciationCount,
                                      ReturnCode::Type& returnCode);
          /**
           * Creates a new WordItem. This convenience method is equivilent to invoking
           * <code>create(word, pronunciations, pronunciationCount, returnCode)</code> with a single
           * proununciation.
           *
           * @param word the word to insert
           * @param pronunciation the pronunciation to associated with the item. If the value is
           * null the recognizer will attempt to guess the pronunciation.
           * @param returnCode the return code
           */
          static WordItemProxy create(const char* word, const char* pronunciation,
                                      ReturnCode::Type& returnCode);
                                      
          /**
           * Returns true if the item is a word.
           */
          virtual bool isWord() const;
          
          /**
           * Returns true if the item is a voicetag.
           */
          virtual bool isVoicetag() const;
        protected:
          /**
           * Prevent construction.
           */
          WordItem();
          /**
           * Prevent destruction.
           */
          virtual ~WordItem();
          
          friend class WordItemProxy;
      };
      
      /*
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, WordItemProxy, SlotItemProxy, WordItem)
    }
  }
}

#endif
