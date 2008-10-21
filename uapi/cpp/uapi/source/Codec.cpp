/*---------------------------------------------------------------------------*
 *  Codec.cpp                                                                *
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
#include "Codec.h"
#include "ReturnCode.h"

using namespace android::speech::recognition;


UINT16 Codec::getSampleRate(Type codec, ReturnCode::Type& returnCode)
{
  switch (codec)
  {
    case ULAW_8BIT_8K:
    case PCM_16BIT_8K:
    {
      returnCode = ReturnCode::SUCCESS;
      return 8000;
    }
    case PCM_16BIT_11K:
    {
      returnCode = ReturnCode::SUCCESS;
      return 11025;
    }
    case PCM_16BIT_22K:
    {
      returnCode = ReturnCode::SUCCESS;
      return 22050;
    }
    default:
    {
      returnCode = ReturnCode::ILLEGAL_ARGUMENT;
      return 0;
    }
  }
}

UINT16 Codec::getBitsPerSample(Type codec, ReturnCode::Type& returnCode)
{
  switch (codec)
  {
    case PCM_16BIT_8K:
    case PCM_16BIT_11K:
    case PCM_16BIT_22K:
    {
      returnCode = ReturnCode::SUCCESS;
      return 16;
    }
    case ULAW_8BIT_8K:
    {    
      returnCode = ReturnCode::SUCCESS;
      return 8;
    }
    default:
    {
      returnCode = ReturnCode::ILLEGAL_ARGUMENT;
      return 0;
    }
  }
}
