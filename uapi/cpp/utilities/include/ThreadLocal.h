/*---------------------------------------------------------------------------*
 *  ThreadLocal.h                                                            *
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

#ifndef __UAPI__THREADLOCAL
#define __UAPI__THREADLOCAL

#include "exports.h"
#include "types.h"
#include "ReturnCode.h"
#include <errno.h>
#include <pthread.h>


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        /**
         * This class provides thread-local variables. These variables differ from their normal
         * counterparts in that each thread that accesses one (via its get or set method) has its own,
         * independently initialized copy of the variable. ThreadLocal instances are typically private
         * static fields in classes that wish to associate state with a thread (e.g., a user ID or
         * Transaction ID).
         */
        class UAPI_EXPORT ThreadLocal
        {
          public:
            /**
             * Creates a new ThreadLocal.
             *
             * @param onThreadShutdown used to destroy the thread-local value at thread shutdown
             * @param returnCode the return code
             */
            static ThreadLocal* create(void (*onThreadShutdown)(void*), ReturnCode::Type& returnCode);
            virtual ~ThreadLocal();
            
            /**
             * Sets the current thread's copy of this thread-local variable to the specified value.
             * The user is responsible for deallocating the old value.
             *
             * @param returnCode the return code
             */
            virtual void set(void* value, ReturnCode::Type& returnCode);
            
            /**
             * Returns the value in the current thread's copy of this thread-local variable.
             *
             * @param returnCode the return code
             * @return the current thread's value of this thread-local
             */
            virtual void* get(ReturnCode::Type& returnCode);
          private:
            /**
             * Prevent construction.
             */
            ThreadLocal(pthread_key_t key, void (*onThreadShutdown)(void*));
            
            
            /**
             * Thread-local key.
             */
            pthread_key_t key;
            /**
             * Used to destroy the thread-local value at thread shutdown.
             */
            void (*onThreadShutdown)(void* value);
        };
      }
    }
  }
}

#endif
