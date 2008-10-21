/*---------------------------------------------------------------------------*
 *  SrecRecognitionResultImpl.cpp                                            *
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


//Memory leak detection
#if defined(_DEBUG) && defined(_WIN32)
#include "crtdbg.h"
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif
#include <stdlib.h>

#include "SrecRecognitionResultImpl.h"
#include "types.h"
#include "SrecHelper.h"
#include "SrecVoicetagItemImpl.h"
#include "Logger.h"

#include "plog.h"
#include "SR_RecognizerResult.h"
#include "SR_Nametag.h"


/*#define SREC_MEASURE_LATENCY    1*/
 
#ifdef SREC_MEASURE_LATENCY
#include <sys/time.h>
 
struct timeval latency_result;
#endif


using namespace android::speech::recognition::utilities;
using namespace android::speech::recognition::srec;


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace srec
      {
        DEFINE_SMARTPROXY(srec, SrecRecognitionResultImplProxy, NBestRecognitionResultProxy,
                          SrecRecognitionResultImpl)
                          
        DEFINE_SMARTPROXY(srec::SrecRecognitionResultImpl, EntryImplProxy, EntryProxy,
                          SrecRecognitionResultImpl::EntryImpl)
                          
        SrecRecognitionResultImplProxy SrecRecognitionResultImpl::create(
          SR_RecognizerResult* recognitionResult, ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognitionResultImpl::create");
            
          SrecRecognitionResultImpl* result = new SrecRecognitionResultImpl(recognitionResult);
          if (!result)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return SrecRecognitionResultImplProxy();
          }
          else
            returnCode = ReturnCode::SUCCESS;
          return SrecRecognitionResultImplProxy(result);
        }
        
        SrecRecognitionResultImpl::SrecRecognitionResultImpl(SR_RecognizerResult* _result):
            result(_result)
        {
          UAPI_FN_NAME("SrecRecognitionResultImpl::SrecRecognitionResultImpl");
            
          UAPI_TRACE(fn,"this=%p\n", this);
        }
        
        SrecRecognitionResultImpl::~SrecRecognitionResultImpl()
        {
          UAPI_FN_NAME("SrecRecognitionResultImpl::~SrecRecognitionResultImpl");
            
          UAPI_TRACE(fn,"this=%p\n", this);
        }
        
        bool SrecRecognitionResultImpl::isNBestList() const
        {
          return true;
        }
       
        bool SrecRecognitionResultImpl::isAppServerResult() const
        {
          return false;
        }

        VoicetagItemProxy SrecRecognitionResultImpl::createVoicetagItem(const char* VoicetagId, VoicetagItemListenerProxy listener, ReturnCode::Type& returnCode) const
        {
          UAPI_FN_SCOPE("SrecRecognitionResultImpl::createVoicetagItem");
            
          char* voicetagCopy = 0;
          voicetagCopy = new char[strlen(VoicetagId)+1];
          if (voicetagCopy == 0)
          {
             returnCode = ReturnCode::OUT_OF_MEMORY;
             return VoicetagItemProxy();
          }
          strcpy(voicetagCopy, VoicetagId);
          SR_Nametag* nametag;
          ESR_ReturnCode rc = SR_NametagCreate(result, voicetagCopy, &nametag);
          delete []voicetagCopy;
          if (rc != ESR_SUCCESS)
          {
            returnCode = SrecHelper::toUAPI(rc);
            return VoicetagItemProxy();
          }
          
          const INT16* data;
          size_t len;
          
          // TODO: What happens if the "enableGetWaveform" parameter is not set?
          rc = SR_RecognizerResultGetWaveform(result, &data, &len);
          if (rc != ESR_SUCCESS)
          {
            returnCode = SrecHelper::toUAPI(rc);
            SR_NametagDestroy(nametag);
            return VoicetagItemProxy();
          }
          
          assert(len < (size_t)ARRAY_LIMIT_MAX);
          return SrecVoicetagItemImpl::create(nametag, data, (ARRAY_LIMIT) len, listener, returnCode);
        }
        
        ARRAY_LIMIT SrecRecognitionResultImpl::getSize() const
        {
          size_t size;
          ESR_ReturnCode rc;
          CHKLOG(rc, SR_RecognizerResultGetSize(result, &size));
          return (ARRAY_LIMIT) size;
CLEANUP:
          return 0;
        }
        
        SrecRecognitionResultImpl::EntryProxy SrecRecognitionResultImpl::getEntry(
          ARRAY_LIMIT index, ReturnCode::Type& returnCode) const
        {
          if (index >= getSize())
          {
            returnCode = ReturnCode::ARRAY_INDEX_OUT_OF_BOUNDS;
            return EntryProxy();
          }
          returnCode = ReturnCode::SUCCESS;
          return EntryImpl::createEntry(index, result, returnCode);
        }
        
        SrecRecognitionResultImpl::EntryImplProxy SrecRecognitionResultImpl::EntryImpl::createEntry(
          ARRAY_LIMIT index, const SR_RecognizerResult* recognitionResult, ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("SrecRecognitionResultImpl::EntryImpl::createEntry");
            
          SrecRecognitionResultImpl::EntryImpl* result = 0;
          size_t len;
          ARRAY_LIMIT keyCount = 0;
          LCHAR** rawKeys = 0;
          LCHAR** keys = 0;
          LCHAR** values = 0;
          LCHAR* rawMeaning = 0;
          LCHAR* literalMeaning = 0;
          bool confidenceScoreFound = false;
          UINT8 confidenceScore = 0;
          SrecRecognitionResultImpl::EntryImplProxy resultProxy;
          
          // Get the number of keys
          ARRAY_LIMIT rawKeyCount;
          {
            size_t len = 0;
            ESR_ReturnCode rc = SR_RecognizerResultGetKeyList(recognitionResult, index, 0, &len);
            if (rc != ESR_BUFFER_OVERFLOW)
            {
              returnCode = SrecHelper::toUAPI(rc);
              goto CLEANUP;
            }
            rawKeyCount = (ARRAY_LIMIT) len;
          }
          
          rawKeys = new LCHAR*[rawKeyCount];
          
          if (rawKeys == 0)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            goto CLEANUP;
          }
          ESR_ReturnCode rc;
          len = rawKeyCount;
          rc = SR_RecognizerResultGetKeyList(recognitionResult, index, rawKeys, &len);
          if (rc != ESR_SUCCESS)
          {
            returnCode = SrecHelper::toUAPI(rc);
            goto CLEANUP;
          }
          
            // 3 reserved keys: "literal", "raws", "conf"
            keyCount = rawKeyCount - 3;
          
          keys = new LCHAR*[keyCount];
          if (!keys)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            goto CLEANUP;
          }
          
          values = new LCHAR*[keyCount];
          if (!values)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            goto CLEANUP;
          }
          
          for (ARRAY_LIMIT i = 0; i < keyCount; ++i)
          {
            keys[i] = 0;
            values[i] = 0;
          }
          literalMeaning = 0;
          confidenceScore = 0;
          
          // Generate a list of keys and values excluding semanticMeaning, literal and confidenceScore
          for (ARRAY_LIMIT i = 0, j = 0; i < rawKeyCount; ++i)
          {
            const LCHAR* key = rawKeys[i];
            size_t valueLength = 0;
            rc = SR_RecognizerResultGetValue(recognitionResult, index, key, 0, &valueLength);
            if (rc != ESR_BUFFER_OVERFLOW)
            {
              returnCode = SrecHelper::toUAPI(rc);
              goto CLEANUP;
            }
            
            if (strcmp(key, "meaning") == 0)
            {
              assert(rawMeaning == 0);
              rawMeaning = new LCHAR[valueLength];
              if (!rawMeaning)
              {
                returnCode = ReturnCode::OUT_OF_MEMORY;
                goto CLEANUP;
              }
              rc = SR_RecognizerResultGetValue(recognitionResult, index, key, rawMeaning, &valueLength);
              if (rc != ESR_SUCCESS)
              {
                returnCode = SrecHelper::toUAPI(rc);
                goto CLEANUP;
              }
            }
            else if (strcmp(key, "literal") == 0)
            {
              assert(literalMeaning == 0);
              literalMeaning = new LCHAR[valueLength];
              if (!literalMeaning)
              {
                returnCode = ReturnCode::OUT_OF_MEMORY;
                goto CLEANUP;
              }
              rc = SR_RecognizerResultGetValue(recognitionResult, index, key, literalMeaning, &valueLength);
              if (rc != ESR_SUCCESS)
              {
                returnCode = SrecHelper::toUAPI(rc);
                goto CLEANUP;
              }
            }
            else if (strcmp(key, "conf") == 0)
            {
              LCHAR* temp = new LCHAR[valueLength];
              if (!temp)
              {
                returnCode = ReturnCode::OUT_OF_MEMORY;
                goto CLEANUP;
              }
              rc = SR_RecognizerResultGetValue(recognitionResult, index, key, temp, &valueLength);
              if (rc != ESR_SUCCESS)
              {
                returnCode = SrecHelper::toUAPI(rc);
                goto CLEANUP;
              }
              confidenceScoreFound = true;
              // SREC returns value 0-1000, UAPI returns 0-100
              char* endptr;
              long srecConfidenceScore = strtol(temp, &endptr, 10);
              if (strlen(temp) == 0 || endptr != temp + strlen(temp) ||
                  srecConfidenceScore == LONG_MAX || srecConfidenceScore == LONG_MIN)
              {
                // If the string was empty or contained a non-numeric character, or an overflow or underflow
                // occured
                delete[] temp;
                returnCode = ReturnCode::INVALID_STATE;
                goto CLEANUP;
              }
              else if ((srecConfidenceScore < 0) || (srecConfidenceScore > 1000))
              {
                delete[] temp;
                returnCode = ReturnCode::INVALID_STATE;
                goto CLEANUP;
              }
              confidenceScore = (UINT8)(srecConfidenceScore / 10);
              delete[] temp;
            }
            else
            {
              if (j >= keyCount)
              {
                // There are more "normal" keys than we expected, this means that "literal", "meaning" or "conf"
                // is missing and a normal key took its place.
                returnCode = ReturnCode::INVALID_STATE;
                UAPI_ERROR(fn,"More normal keys than expected");
                goto CLEANUP;
              }
              keys[j] = new LCHAR[strlen(key)+1];
              if (!keys[j])
              {
                returnCode = ReturnCode::OUT_OF_MEMORY;
                goto CLEANUP;
              }
              strcpy(keys[j], key);
              
              values[j] = new LCHAR[valueLength];
              if (!values[j])
              {
                returnCode = ReturnCode::OUT_OF_MEMORY;
                goto CLEANUP;
              }
              rc = SR_RecognizerResultGetValue(recognitionResult, index, key, values[j], &valueLength);
              if (rc != ESR_SUCCESS)
              {
                returnCode = SrecHelper::toUAPI(rc);
                goto CLEANUP;
              }
              ++j;
            }
          }
          
          if (rawMeaning == 0 || literalMeaning == 0 || !confidenceScoreFound)
          {
            returnCode = ReturnCode::INVALID_STATE;
            goto CLEANUP;
          }
          
          result = new SrecRecognitionResultImpl::EntryImpl(rawMeaning, literalMeaning,
              confidenceScore, keys, values, keyCount);
          if (!result)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            goto CLEANUP;
          }
          resultProxy = SrecRecognitionResultImpl::EntryImplProxy(result);
          if (!resultProxy)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            goto CLEANUP;
          }
          delete[] rawKeys;
          return resultProxy;
CLEANUP:
          delete[] rawKeys;
          for (ARRAY_LIMIT i = 0; i < keyCount; ++i)
          {
            delete[] keys[i];
            delete[] values[i];
          }
          delete[] keys;
          delete[] values;
          delete[] rawMeaning;
          delete[] literalMeaning;
#ifdef SREC_MEASURE_LATENCY
          gettimeofday ( &latency_result, NULL );
          printf ( "Result Time :  %ld Seconds  %ld Microseconds\n", latency_result.tv_sec, latency_result.tv_usec );
#endif

          return SrecRecognitionResultImpl::EntryImplProxy();
        }
        
        SrecRecognitionResultImpl::EntryImpl::EntryImpl(LCHAR* _semanticMeaning,
            LCHAR* _literalMeaning, UINT8 _confidenceScore, LCHAR ** _keys, LCHAR ** _values, 
            ARRAY_LIMIT _keyCount):
            semanticMeaning(_semanticMeaning),
            literalMeaning(_literalMeaning),
            confidenceScore(_confidenceScore),
            keys(_keys),
            values(_values),
            keyCount(_keyCount)
        {
        }
        
        SrecRecognitionResultImpl::EntryImpl::~EntryImpl()
        {
          delete[] literalMeaning;
          delete[] semanticMeaning;
          for (ARRAY_LIMIT i = 0; i < keyCount; ++i)
          {
            delete[] keys[i];
            delete[] values[i];
          }
          delete[] keys;
          delete[] values;
        }
        
        const char* SrecRecognitionResultImpl::EntryImpl::getLiteralMeaning(
          ReturnCode::Type& returnCode) const
        {
          returnCode = ReturnCode::SUCCESS;
          return literalMeaning;
        }
        
        const char* SrecRecognitionResultImpl::EntryImpl::getSemanticMeaning(
          ReturnCode::Type& returnCode) const
        {
          returnCode = ReturnCode::SUCCESS;
          return semanticMeaning;
        }
        
        UINT8 SrecRecognitionResultImpl::EntryImpl::getConfidenceScore(ReturnCode::Type& returnCode) const
        {
          returnCode = ReturnCode::SUCCESS;
          return confidenceScore;
        }

        ARRAY_LIMIT SrecRecognitionResultImpl::EntryImpl::getKeyCount() const
        {
          return keyCount;
        }

        const char* const* SrecRecognitionResultImpl::EntryImpl::getKeys()
        {
          return keys;
        }

        const char* SrecRecognitionResultImpl::EntryImpl::getValue(const char* key,
            ReturnCode::Type& returnCode) const
        {
          returnCode = ReturnCode::SUCCESS;
          for (ARRAY_LIMIT i = 0; i < keyCount; ++i)
          {
            if (strcmp(key, keys[i]) == 0)
              return values[i];
          }

          returnCode = ReturnCode::ILLEGAL_ARGUMENT;
          return 0;
        }




      }
    }
  }
}
