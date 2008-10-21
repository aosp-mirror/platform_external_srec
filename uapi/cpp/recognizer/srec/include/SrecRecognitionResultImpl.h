/*---------------------------------------------------------------------------*
 *  SrecRecognitionResultImpl.h                                              *
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

#ifndef __UAPI__SRECRECOGNITIONRESULTIMPL
#define __UAPI__SRECRECOGNITIONRESULTIMPL

#include "exports.h"
#include "types.h"
#include "NBestRecognitionResult.h"


typedef struct SR_RecognizerResult_t SR_RecognizerResult;
#include "LCHAR.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace srec
      {
        class SrecRecognitionResultImplProxy;
        /**
         * SREC recognition result.
         */
        class SrecRecognitionResultImpl: public NBestRecognitionResult
        {
          public:
            class EntryImplProxy; // This is the only legal way to forward-declare a nested class
            
            /**
             * NBestList entry.
             *
             * @see NBestRecognitionResult
             */
          class EntryImpl: public Entry
            {
              public:
                /**
                 * Returns the number of semantic key-value pairs.
                 */
                SREC_EXPORT virtual ARRAY_LIMIT getKeyCount() const;
                /**
                 * Returns the keys associated with the nbest entry.
                 */
                SREC_EXPORT virtual const char* const* getKeys();
                /**
                 * Returns the values associated with the nbest entry.
                 *
                 * @param key the semantic key to look up
                 * @param returnCode the return code
                 */
                SREC_EXPORT virtual const char* getValue(const char* key,
                    ReturnCode::Type& returnCode) const;

                /**
                 * The literal meaning of a recognition result (i.e. literally what the user said).
                 * In an example where a person's name is mapped to a phone-number, the person's name
                 * is the literal meaning.
                 *
                 * @param returnCode returns SUCCESS unless a fatal error occurs
                 */
                SREC_EXPORT virtual const char* getLiteralMeaning(ReturnCode::Type& returnCode)
                const;
                /**
                 * The semantic meaning of a recognition result (i.e. the application-specific value
                 * associated with what the user said). In an example where a person's name is mapped
                 * to a phone-number, the phone-number is the semantic meaning.
                 *
                 * @param returnCode returns SUCCESS unless a fatal error occurs
                 */
                SREC_EXPORT virtual const char* getSemanticMeaning(ReturnCode::Type& returnCode)
                const;
                /**
                 * The confidence score of a recognition result. Values range from 0 to 100 (inclusive).
                 *
                 * @param returnCode NOT_SUPPORTED if the engine does not generate a confidence score
                 *        for this n-best entry.
                 */
                SREC_EXPORT virtual UINT8 getConfidenceScore(ReturnCode::Type& returnCode) const;
                
                friend class SrecRecognitionResultImpl;
              private:
                /**
                 * Prevent destruction.
                 */
                virtual ~EntryImpl();
                /**
                 * Prevent construction.
                 */
                EntryImpl(LCHAR* semanticMeaning, LCHAR* literalMeaning, UINT8 confidenceScore,
                          LCHAR ** keys, LCHAR ** values, ARRAY_LIMIT keyCount);
                /**
                 * Prevent copying.
                 */
                EntryImpl& operator=(EntryImpl& other);
                
                /**
                 * Creates a new nbest entry.
                 *
                 * @param index the n-best list index associated with this entry
                 * @param result the underlying recognition result
                 * @param grammarConfigs the grammar configurations
                 * @param configCount the number of configurations
                 * @param returnCode NO_MATCH if no GrammarConfiguration::grammarToMeaning() matched
                 * the recognition result
                 */
                static SrecRecognitionResultImpl::EntryImplProxy createEntry(
                  ARRAY_LIMIT index, const SR_RecognizerResult* result,
                  ReturnCode::Type& returnCode);
                  
                  
                const LCHAR*    semanticMeaning;
                const LCHAR*    literalMeaning;
                const UINT8     confidenceScore;
                LCHAR**   keys;
                LCHAR**   values;
                ARRAY_LIMIT     keyCount;
                
                friend class EntryImplProxy;
            };
            
            /*
             * @see android::speech::recognition::SmartProxy
             */
            DECLARE_SMARTPROXY(SREC_EXPORT, EntryImplProxy, EntryProxy, EntryImpl)
            
            
            /**
             * Creates a new SrecRecognitionResultImpl.
             *
             * @param recognitionResult the underlying recognition result
             * @param grammarConfigs the grammar configurations
             * @param configCount the number of configurations
             * @param returnCode SUCCESS unless a fatal error has occurred.
             */
            static SrecRecognitionResultImplProxy create(SR_RecognizerResult* recognitionResult,
                ReturnCode::Type& returnCode);
            /**
             * Returns true if the recognition result is an n-best list.
             */
            virtual bool isNBestList() const;
            /**
             * Returns true if the recognition is from an app server.
             */
            virtual bool isAppServerResult() const;
            /**
             * Returns the number of entries in the n-best list.
             */
            virtual ARRAY_LIMIT getSize() const;
            /**
             * Returns an NBest entry.
             *
             * @param index the entry index number
             * @param returnCode ARRAY_INDEX_OUT_OF_BOUNDS if index is out of bounds
             * @return null if all active GrammarConfiguration.grammarToMeaning() return null
             */
            virtual EntryProxy getEntry(ARRAY_LIMIT index,
                                        ReturnCode::Type& returnCode) const;
            /**
            * Creates a new VoicetagItem if the last recognition was an enrollment operation.
            *
            * @param VoicetagId unique string voicetag id, to be used as the NameTagId
            * @param returnCode INVALID_STATE if the last recognition was not an enrollment operation
            */
            virtual VoicetagItemProxy createVoicetagItem(const char* VoicetagId, VoicetagItemListenerProxy listener, ReturnCode::Type& returnCode) const;
          private:
            /**
             * Prevent construction.
             */
            SrecRecognitionResultImpl(SR_RecognizerResult* recognitionResult);
            /**
             * Prevent destruction.
             */
            virtual ~SrecRecognitionResultImpl();
            /**
             * Prevent copying.
             */
            SrecRecognitionResultImpl& operator=(SrecRecognitionResultImpl& other);
            const SR_RecognizerResult* result;
            
            friend class SrecRecognitionResultImplProxy;
        };
        
        /*
         * @see android::speech::recognition::SmartProxy
         */
        DECLARE_SMARTPROXY(SREC_EXPORT, SrecRecognitionResultImplProxy, NBestRecognitionResultProxy,
                           SrecRecognitionResultImpl)
      }
    }
  }
}

#endif
