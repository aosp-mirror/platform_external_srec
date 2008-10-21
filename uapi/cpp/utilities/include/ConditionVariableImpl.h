/*---------------------------------------------------------------------------*
 *  ConditionVariableImpl.h                                                  *
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

#ifndef __UAPI__CONDITIONVARIABLEIMPL
#define __UAPI__CONDITIONVARIABLEIMPL

#include "exports.h"
#include "types.h"
#include "TimeInstant.h"
#include "ReturnCode.h"
#include "ConditionVariable.h"
#include <pthread.h>

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class MutexImpl;
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
        class UAPI_EXPORT ConditionVariableImpl: public ConditionVariable
        {
          public:
            virtual ~ConditionVariableImpl();
            
            /**
             * Wakes one thread waiting on a condition variable.  The thread to wake is
             * determined by its scheduling policy.
             *
             * @param returnCode THREAD_ERROR on failure
             */
            virtual void signal(ReturnCode::Type& returnCode);
            
            /**
             * Wait forever until the condition variable is Signal(ed). Note the the
             * mutex must be Locked before tihs funciton is called. Once Signal(ed),
             * the mutex will be relocked and the wait function will return.
             *
             * @param returnCode THREAD_ERROR on failure
             */
            virtual void wait(ReturnCode::Type& returnCode);
            
            /**
             * Wait until the condition variable is Signal(ed) or until the timeout has
             * elapsed. Note the the mutex must be Locked before tihs funciton is
             * called. Once Signal(ed), the mutex will be relocked and the TimedWaitMs
             * function will return.
             *
             * @param milliseconds the amount of time, in milliseconds, to wait for the condition
             * @param returnCode TIMEOUT if the operation timed out before the condition vairable
             * was signaled. THREAD_ERROR if an error occured.
             */
            virtual void wait(UINT32 milliseconds, ReturnCode::Type& returnCode);
            
          private:
            /**
             * Prevent construction.
             */
            ConditionVariableImpl(MutexImpl* mutex, pthread_cond_t* condition);
            /**
             * Prevent copying.
             */
            ConditionVariableImpl(const ConditionVariable&);
            /**
             * Prevent copying.
             */
            ConditionVariableImpl& operator=(ConditionVariable&);
            
            friend class ConditionVariable;
            
            
            /**
             * Handle to the mutex.
             */
            MutexImpl* mutex;
            /**
             * Handle to the condition to signal.
             */
            pthread_cond_t* condition;
            /**
             * keep track of signals before EINTR so the signal won't be lost due to
             * interrupts.
             */
            bool signaled;
        };
      }
    }
  }
}

#endif
