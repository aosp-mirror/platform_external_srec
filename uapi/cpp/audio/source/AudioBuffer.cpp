/*---------------------------------------------------------------------------*
 *  AudioBuffer.cpp                                                          *
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
#include "AudioBuffer.h"
#include "Logger.h"
#include <stdlib.h>

using namespace android::speech::recognition;
using namespace android::speech::recognition::utilities;

AudioBuffer* AudioBuffer::create(UINT8 initial_count, UINT32 bufSize, ReturnCode::Type& returnCode)
{
  AudioBuffer* result = new AudioBuffer(initial_count, returnCode);
  if (result == 0)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return 0;
  }
  if (returnCode != ReturnCode::SUCCESS)
  {
    delete result;
    return 0;
  }
  
  result->buffer = (unsigned char*) malloc(sizeof(char) * bufSize);
  if (result->buffer == 0)
  {
    delete result;
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return 0;
  }
  
  returnCode = ReturnCode::SUCCESS;
  return result;
}


AudioBuffer::AudioBuffer(UINT8 initial_count, ReturnCode::Type& returnCode):
    RefCounted(initial_count, returnCode), buffer(0), size(0), isLastBuffer(false)
{
  UAPI_FN_NAME("AudioBuffer::AudioBuffer");
    
  if (returnCode)
  {
    UAPI_ERROR(fn,"Failed to create an AudioBuffer\n");
    return;
  }
}

AudioBuffer::~AudioBuffer()
{
  free(buffer);
}
