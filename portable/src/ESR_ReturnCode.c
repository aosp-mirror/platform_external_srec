/*---------------------------------------------------------------------------*
 *  ESR_ReturnCode.c  *
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



#include "ESR_ReturnCode.h"

#define RETURNCODE_COUNT 28
static LCHAR* rcStringMapping[RETURNCODE_COUNT+1] =
  {
    L("ESR_SUCCESS"),
    L("ESR_CONTINUE_PROCESSING"),
    L("ESR_FATAL_ERROR"),
    L("ESR_BUFFER_OVERFLOW"),
    L("ESR_OPEN_ERROR"),
    L("ESR_ALREADY_OPEN"),
    L("ESR_CLOSE_ERROR"),
    L("ESR_ALREADY_CLOSED"),
    L("ESR_READ_ERROR"),
    L("ESR_WRITE_ERROR"),
    L("ESR_FLUSH_ERROR"),
    L("ESR_SEEK_ERROR"),
    L("ESR_OUT_OF_MEMORY"),
    L("ESR_ARGUMENT_OUT_OF_BOUNDS"),
    L("ESR_NO_MATCH_ERROR"),
    L("ESR_INVALID_ARGUMENT"),
    L("ESR_NOT_SUPPORTED"),
    L("ESR_INVALID_STATE"),
    L("ESR_THREAD_CREATION_ERROR"),
    L("ESR_IDENTIFIER_COLLISION"),
    L("ESR_TIMED_OUT"),
    L("ESR_INVALID_RESULT_TYPE"),
    L("ESR_NOT_IMPLEMENTED"),
    L("ESR_CONNECTION_RESET_BY_PEER"),
    L("ESR_PROCESS_CREATE_ERROR"),
    L("ESR_TTS_NO_ENGINE"),
    L("ESR_MUTEX_CREATION_ERROR"),
    L("ESR_DEADLOCK"),
    L("invalid return code") /* must remain last element for ESR_rc2str() to function */
  };
  
const LCHAR* ESR_rc2str(const ESR_ReturnCode rc)
{
  if (rc >= RETURNCODE_COUNT)
    return rcStringMapping[RETURNCODE_COUNT];
  return rcStringMapping[rc];
}


#ifdef _WIN32
__declspec(thread) unsigned long stackEnd = 0;
__declspec(thread) unsigned long stackBeginMin = LONG_MAX;

#include "plog.h"

#ifdef __cplusplus
extern "C"
#endif

  void __declspec(naked) _cdecl _penter(void)
{
  _asm
  {
    pushad
    mov ebp, esp
  }
  
  {
    unsigned long espBuffer;
    
    _asm
    {
      mov espBuffer, esp
    }
    if (espBuffer > stackEnd)
      stackEnd = espBuffer;
    //printf("\nThread=%d,-addressStack-begin=%lu\n", GetCurrentThreadId(), espBuffer);
  }
  
  _asm
  {
    popad
    ret
  }
}

void __declspec(naked) _cdecl _pexit(void)
{
  _asm
  {
    pushad
    mov ebp, esp
  }
  
  {
    unsigned long stackBegin;
    
    _asm
    {
      sub esp, 4
      mov stackBegin, esp
    }
    
    if (stackBegin < stackBeginMin)
    {
      stackBeginMin = stackBegin;
      if (stackEnd - stackBegin > 20*1000)
        printf("*****************************\nThread %d, Maximum stack usage=%lu\n*******************************\n", GetCurrentThreadId(), (stackEnd - stackBeginMin));
    }
    /*
    if (stackEnd - stackBegin > 15*1000)
     printf("\nThread=%d,Stack-size=%lu\n", GetCurrentThreadId(), (stackEnd - stackBegin));
    */
    
    _asm
    {
      add esp, 4
      }
    }
    
  _asm
  {
    popad
    ret
  }
}
#endif
