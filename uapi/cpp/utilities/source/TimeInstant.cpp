/*---------------------------------------------------------------------------*
 *  TimeInstant.cpp                                                          *
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
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#ifdef UAPI_WIN32
# include <sys/timeb.h>          /* ftime() */
#else
# include <sys/time.h>
#endif

#include "TimeInstant.h"
#include "Logger.h"

using namespace android::speech::recognition::utilities;


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      TimeInstant::TimeInstant()
      {}
      
      TimeInstant TimeInstant::now()
      {
        TimeInstant result;
#ifdef UAPI_WIN32
        struct timeb timebuf;
        
        ftime(&timebuf);
        // timeval.tv_sec uses long whereas timeb.time uses unsigned long
        result.value.tv_sec = (long) timebuf.time;
        result.value.tv_usec = timebuf.millitm * 1000;
#else
        gettimeofday(&result.value, 0);
#endif
        return result;
      }
      
      TimeInstant TimeInstant::plus(UINT32 milliseconds)
      {
        timeval result;
        
        result.tv_sec = milliseconds / 1000 + value.tv_sec;
        result.tv_usec = 1000 * (milliseconds % 1000) + value.tv_usec;
        if (result.tv_usec >= 1000000)
        {
          result.tv_usec -= 1000000;
          result.tv_sec += 1;
        }
        TimeInstant instant;
        instant.value = result;
        return instant;
      }
      
      INT32 TimeInstant::minus(TimeInstant instant, ReturnCode::Type& returnCode)
      {
        // timeval.usec is guaranteed to be less than one million
        // so timeval.usec / 1000 is guaranteed to be less than one thousand
        INT32 seconds = value.tv_sec - instant.value.tv_sec;
        INT32 milliseconds = (value.tv_usec - instant.value.tv_usec) / 1000;
        INT32 millisecondsInSeconds = milliseconds / 1000;
        if (seconds + millisecondsInSeconds < (INT32_MIN / 1000))
        {
          returnCode = ReturnCode::UNDERFLOW_ERROR;
          return INT32_MIN;
        }
        else if (seconds + millisecondsInSeconds > (INT32_MAX / 1000))
        {
          returnCode = ReturnCode::OVERFLOW_ERROR;
          return INT32_MAX;
        }
        
        returnCode = ReturnCode::SUCCESS;
        return seconds * 1000 + milliseconds;
      }
      
      timeval TimeInstant::toTimeval()
      {
        return value;
      }
    }
  }
}
