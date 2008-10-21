/*---------------------------------------------------------------------------*
 *  ConditionVariable.h                                                      *
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

#ifndef __UAPI__CONDITIONVARIABLE
#define __UAPI__CONDITIONVARIABLE

#include "exports.h"
#include "types.h"
#include "TimeInstant.h"
#include "ReturnCode.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class Mutex;
      }
    }
  }
}


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        /**
         * To avoid entering a busy waiting state, processes must be able to signal each other about
         * events of interest. Monitors provide this capability through condition variables.
         * When a monitor function requires a particular condition to be true before it can proceed,
         * it waits on an associated condition variable. By waiting, it gives up the lock and is removed
         * from the set of runnable processes. Any process that subsequently causes the condition to be
         * true may then use the condition variable to notify a process waiting for the condition.
         * A process that has been notified regains the lock and can proceed.
         *
         * Source: http://en.wikipedia.org/wiki/Monitor_(synchronization)#Condition_variables
         */
        class UAPI_EXPORT ConditionVariable
        {
          public:
            /**
             * Creates a new ConditionVariable.
             *
             * @param mutex the mutex that must be locked when signaling and waiting on the
             * condition variable
             * @param returnCode ILLEGAL_ARGUMENT if mutex is null
             */
            static ConditionVariable* create(Mutex* mutex, ReturnCode::Type& returnCode);
            
            /**
             * Destructor destroy a condition variable.  It can fail if the condition
             * variable is busy.
             */
            virtual ~ConditionVariable();
            
            /**
             * Wakes one thread waiting on a condition variable.  The thread to wake is
             * determined by its scheduling policy.
             *
             * @param returnCode THREAD_ERROR on failure
             */
            virtual void signal(ReturnCode::Type& returnCode) = 0;
            
            /**
             * Wait forever until the condition variable is Signal(ed). Note the the
             * mutex must be Locked before tihs funciton is called. Once Signal(ed),
             * the mutex will be relocked and the Wait function will return.
             *
             * <b>WARNING</b>: Invoking this method while holding more than one lock over a
             * recursive mutex will result in a deadlock.
             *
             * @param returnCode THREAD_ERROR on failure
             */
            virtual void wait(ReturnCode::Type& returnCode) = 0;
            
            /**
             * Wait until the condition variable is Signal(ed) or until the timeout has
             * elapsed. Note the the mutex must be Locked before tihs funciton is
             * called. Once Signal(ed), the mutex will be relocked and the TimedWaitMs
             * function will return.
             *
             * <b>WARNING</b>: Invoking this method while holding more than one lock over a
             * recursive mutex will result in a deadlock.
             *
             * @param milliseconds the amount of time, in milliseconds, to wait for the condition
             * @param returnCode TIMEOUT if the operation timed out before the condition variable
             * was signaled. THREAD_ERROR if an error occured.
             */
            virtual void wait(UINT32 milliseconds, ReturnCode::Type& returnCode) = 0;
          protected:
            /**
             * Prevent construction.
             */
            ConditionVariable();
          private:
            /**
             * Prevent copying.
             */
            ConditionVariable(const ConditionVariable&);
            /**
             * Prevent copying.
             */
            ConditionVariable& operator=(ConditionVariable&);
        };
      }
    }
  }
}

#endif
