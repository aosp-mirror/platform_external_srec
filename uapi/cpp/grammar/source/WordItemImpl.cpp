/*---------------------------------------------------------------------------*
 *  WordItemImpl.cpp                                                         *
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
#include "WordItemImpl.h"
#include "ReturnCode.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::impl;


DEFINE_SMARTPROXY(impl, WordItemImplProxy, WordItemProxy, WordItemImpl)

WordItemImpl::WordItemImpl(const char* word, const char* pronunciations):
    _word(word),
    _pronunciations(pronunciations)
{
}

WordItemImplProxy WordItemImpl::create(const char* word, const char** pronunciations,
                                       ARRAY_LIMIT pronunciationCount,
                                       ReturnCode::Type& returnCode)
{
  if (pronunciationCount > 0 && pronunciations == 0)
  {
    returnCode = ReturnCode::ILLEGAL_ARGUMENT;
    return WordItemImplProxy();
  }
  char* wordCopy = new char[strlen(word)+1];
  if (wordCopy == 0)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return WordItemImplProxy();
  }
  strcpy(wordCopy, word);
  
  
  /* when making a copy of the prons, let's make one big allocation
     beacuse this is how SREC wants it anyways:
     SREC needs a char* pronunciations like (double null at the end):
     [data1\0data2\0\data3\0\0]
  */
  size_t szBuff = 0;
  for (ARRAY_LIMIT i = 0; i < pronunciationCount; ++i)
      szBuff+=strlen(pronunciations[i]);
  szBuff+=pronunciationCount; //add for null character for each pronunciation
  szBuff+=1;                  //add for termination null 
    
  char* pronunciationsCopy = new char[szBuff];
  if (pronunciationCount>0)
  {
        char* p = pronunciationsCopy;
        size_t count = 0;
        for (ARRAY_LIMIT i = 0; i < pronunciationCount; ++i)
        {
            strcpy(p,pronunciations[i]);    
            count+=strlen(pronunciations[i]);
            count++;
            p = &pronunciationsCopy[count];
            p[0]='\0';
        }
        p = NULL;
  }
  else 
      pronunciationsCopy[0] = '\0';
  WordItemImpl* object = new WordItemImpl(wordCopy,pronunciationsCopy);

  if (object == 0)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    delete[] wordCopy;
    delete[] pronunciationsCopy;
    return WordItemImplProxy();
  }
  WordItemImplProxy result(object);
  if (!result)
    returnCode = ReturnCode::OUT_OF_MEMORY;
  else
    returnCode = ReturnCode::SUCCESS;
  return result;
}

WordItemImpl::~WordItemImpl()
{
  delete[] _word;
  delete[] _pronunciations;
}

const char* WordItemImpl::getWord() const
{
  return _word;
}

const char* WordItemImpl::getPronunciation() const
{
  return _pronunciations;
}
