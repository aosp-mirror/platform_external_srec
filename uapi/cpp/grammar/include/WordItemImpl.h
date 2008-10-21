/*---------------------------------------------------------------------------*
 *  WordItemImpl.h                                                           *
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

#ifndef __UAPI__WORDITEMIMPL
#define __UAPI__WORDITEMIMPL

#include "exports.h"
#include "ReturnCode.h"
#include "WordItem.h"
#include <string.h>


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace impl
      {
        class WordItemImplProxy;
        /**
         * Word that may be inserted into an embedded grammar slot.
         */
        class UAPI_EXPORT WordItemImpl: public WordItem
        {
          public:
            /**
             * Creates a new WordItemImpl.
             *
             * @param word the word to be added
             * @param pronunciations the pronunciations to associated with the item. If the list is
             * is empty the recognizer will attempt to guess the pronunciations.
             * @param pronunciationCount the number of pronunciations
             * @param returnCode ILLEGAL_ARGUMENT if pronunciations is null or pronunciationCount < 1
             */
            static WordItemImplProxy create(const char* word, const char** pronunciations,
                                            ARRAY_LIMIT pronunciationCount,
                                            ReturnCode::Type& returnCode);
                                            
            /**
             * Returns the word to add.
             */
            const char* getWord() const;
            
            /**
             * Returns a pronunciation that is associated with the WordItem.
             */
            const char* getPronunciation() const;
            
          protected:
            /**
             * Prevent construction.
             */
            WordItemImpl(const char* word, const char* pronunciations);
            /**
             * Prevent destruction.
             */
            virtual ~WordItemImpl();

        private:
            const char* _word;
            const char* _pronunciations;
            friend class WordItemImplProxy;
        };
        
        /*
         * @see android::speech::recognition::SmartProxy
         */
        DECLARE_SMARTPROXY(UAPI_EXPORT, WordItemImplProxy, WordItemProxy, WordItemImpl)
      }
    }
  }
}

#endif
