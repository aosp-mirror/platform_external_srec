/*---------------------------------------------------------------------------*
 *  NBestRecognitionResult.h                                                 *
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

#ifndef __UAPI__NBESTRECOGNITIONRESULT
#define __UAPI__NBESTRECOGNITIONRESULT

#include "exports.h"
#include "RecognitionResult.h"
#include "ReturnCode.h"
#include "SmartProxy.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class VoicetagItemProxy;
      class VoicetagItemListenerProxy;
    }
  }
}


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class NBestRecognitionResultProxy;
      
      /**
       * N-Best Recognition results. Entries are sorted in decreasing order according to their probability,
       * from the most probable result to the least probable result.
       */
      class NBestRecognitionResult: public RecognitionResult
      {
        public:
          class EntryProxy;
          
          /**
           * NBestList entry.
           *
           * @see NBestRecognitionResult
           */
          class UAPI_EXPORT Entry
          {
            public:
              /**
               * Returns the number of semantic key-value pairs.
               */
              virtual ARRAY_LIMIT getKeyCount() const = 0;
              /**
               * Returns the keys associated with the nbest entry.
               */
              virtual const char* const* getKeys() = 0;
              /**
               * Returns the values associated with the nbest entry.
               *
               * @param key the semantic key to look up
               * @param returnCode the return code
               */
              virtual const char* getValue(const char* key,
                  ReturnCode::Type& returnCode) const = 0;
              /**
               * The literal meaning of a recognition result (i.e. literally what the user said).
               * In an example where a person's name is mapped to a phone-number, the person's name
               * is the literal meaning.
               *
               * @param returnCode the return code
               */
              virtual const char* getLiteralMeaning(ReturnCode::Type& returnCode)
              const = 0;
              /**
               * The semantic meaning of a recognition result (i.e. the application-specific value
               * associated with what the user said). In an example where a person's name is mapped
               * to a phone-number, the phone-number is the semantic meaning.
               *
               * @param returnCode the return code
               */
              virtual const char* getSemanticMeaning(ReturnCode::Type& returnCode)
              const = 0;
              /**
               * The confidence score of a recognition result. Values range from 0 to 100 (inclusive).
               *
               * @param returnCode NOT_SUPPORTED if the engine does not generate a confidence score
               *        for this n-best entry.
               */
              virtual UINT8 getConfidenceScore(ReturnCode::Type& returnCode) const = 0;
            protected:
              /**
               * Prevent construction.
               */
              Entry();
              /**
               * Prevent destruction.
               */
              virtual ~Entry();
              
              friend class EntryProxy;
          };
          
          /*
           * @see android::speech::recognition::SmartProxy
           */
          DECLARE_SMARTPROXY(UAPI_EXPORT, EntryProxy, android::speech::recognition::SmartProxy, Entry)
          
          /**
           * Returns the number of entries in the n-best list.
           */
          virtual ARRAY_LIMIT getSize() const = 0;
          /**
           * Returns an NBest entry.
           *
           * @param index the entry index number
           * @param returnCode the return code
           */
          virtual EntryProxy getEntry(ARRAY_LIMIT index, ReturnCode::Type& returnCode) const = 0;
          /**
           * Creates a new VoicetagItem if the last recognition was an enrollment operation.
           *
           * @param returnCode INVALID_STATE if the last recognition was not an enrollment operation
           */
          virtual VoicetagItemProxy createVoicetagItem(const char* VoicetagId, VoicetagItemListenerProxy listener, ReturnCode::Type& returnCode)
          const = 0;
        protected:
          /**
           * Prevent construction.
           */
          UAPI_EXPORT NBestRecognitionResult();
          /**
           * Prevent destruction.
           */
          UAPI_EXPORT virtual ~NBestRecognitionResult();
          
          friend class NBestRecognitionResultProxy;
      };
      
      /*
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, NBestRecognitionResultProxy, RecognitionResultProxy,
                         NBestRecognitionResult)
    }
  }
}

#endif
