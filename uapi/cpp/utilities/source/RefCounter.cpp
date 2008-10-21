/*---------------------------------------------------------------------------*
 *  RefCounter.cpp                                                           *
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
#include "RefCounter.h"
#include "Logger.h"
#include "Mutex.h"
#include <stdio.h>

using namespace android::speech::recognition;
using namespace android::speech::recognition::utilities;


RefCounter::RefCounter(void* _object, bool _loggingAllowed, ReturnCode::Type& returnCode):
    count(1),
    object(_object),
    loggingAllowed(_loggingAllowed)
{
  returnCode = ReturnCode::SUCCESS;
}

RefCounter::~RefCounter()
{
}

ARRAY_LIMIT RefCounter::increment(ReturnCode::Type& returnCode)
{
  if (count < ARRAY_LIMIT_MAX)
  {
    returnCode = ReturnCode::SUCCESS;
    ++count;
  }
  else
    returnCode = ReturnCode::OVERFLOW_ERROR;
  return count;
}

ARRAY_LIMIT RefCounter::decrement(ReturnCode::Type& returnCode)
{
  returnCode = ReturnCode::SUCCESS;
  if (count > ARRAY_LIMIT_MIN)
  {
    returnCode = ReturnCode::SUCCESS;
    --count;
  }
  else
    returnCode = ReturnCode::UNDERFLOW_ERROR;
  return count;
}

ARRAY_LIMIT RefCounter::getCount() const
{
  return count;
}

void* RefCounter::getObject() const
{
  return object;
}


bool RefCounter::isLoggingAllowed() const
{
  return loggingAllowed;
}
