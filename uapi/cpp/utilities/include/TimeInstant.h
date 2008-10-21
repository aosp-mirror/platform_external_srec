/*---------------------------------------------------------------------------*
 *  TimeInstant.h                                                            *
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

#ifndef __UAPI__TIMEINSTANT
#define __UAPI__TIMEINSTANT

#ifdef WIN32
#include <winsock2.h>          /* timeval */
#else
#include <sys/time.h>
#endif

#include "types.h"
#include "ReturnCode.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        /**
         * An instant in time.
         */
        class TimeInstant
        {
          public:
            TimeInstant();
            /**
             * Returns the current time.
             *
             * @return the resulting time instant
             */
            static TimeInstant now();
            
            /**
             * Returns the time instant that results from adding a duration to the current time instance.
             *
             * @param milliseconds the duration in milliseconds
             * @return the resulting time instant
             */
            TimeInstant plus(UINT32 milliseconds);
            
            /**
             * Returns the number of milliseconds elapsed between the current time-instant minus the second
             * time-instant.
             *
             * @param second the time instant to compare to
             * @param returnCode the return code
             * @return the second time-instant in milliseconds minus the current time in milliseconds
             */
            INT32 minus(TimeInstant second, ReturnCode::Type& returnCode);
            /**
             * Returns the timeval representation of this object.
             *
             * @return timeval representation
             */
            timeval toTimeval();
          private:
            timeval value;
        };
      }
    }
  }
}

#endif
