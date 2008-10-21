/*---------------------------------------------------------------------------*
 *  MutexImpl.h                                                              *
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

#ifndef __UAPI__MUTEX_IMPL
#define __UAPI__MUTEX_IMPL

#include "Mutex.h"
#include "types.h"

#include <errno.h>
#include <pthread.h>

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace impl
      {
        class LoggerImpl;
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
         * Class used to create, destroy, lock and unlock a mutex.
         */
        class UAPI_EXPORT MutexImpl: public Mutex
        {
          public:
            /**
             * Creates a new Mutex.
             *
             * @param loggingAllowed true if the object and its dependencies are allowed logging
             * @param returnCode THREAD_ERROR on failure.
             * @return a valid mutex handle if SUCCESS, 0 on failure.
             */
            static MutexImpl* create(bool loggingAllowed, ReturnCode::Type& returnCode);
            /**
             * Destructor. Releases the mutex handle.
             */
            virtual ~MutexImpl();
            
            /**
             * Locks the mutex
             *
             * @param returnCode THREAD_ERROR on failure.
             */
            virtual void lock(recognition::ReturnCode::Type& returnCode);
            
            /**
             * Releases the lock on the mutex.
             *
             * @param returnCode THREAD_ERROR on failure.
             */
            virtual void unlock(recognition::ReturnCode::Type& returnCode);
            
            /**
             * Attempts to enter the protected section without blocking. If the call is
             * successful, the calling thread takes ownership of the mutex.
             *
             * @param returnCode ALREADY_LOCKED if the mutex is already in locked by another thread
             * locked, THREAD_ERROR if another error occurs.
             */
            virtual void tryLock(recognition::ReturnCode::Type& returnCode);
            
            /**
             * Returns true if the object and its dependencies are allowed to log.
             *
             * @return true if the object and its dependencies are allowed to log
             */
            virtual bool isLoggingAllowed() const;
          protected:
            /**
             * Creates a new logger if possible.
             *
             * @return null proxy if a logger could not be created
             */
            //virtual LoggerProxy createLogger(char* function) = 0;
            /**
             * Logs a message using the LEVEL_ERROR logging level.
             *
             * @param format format of variable arguments that follow
             */
            //virtual void logError(LoggerProxy& log, const char* format, ...) = 0;
          private:
            /**
             * Prevent construction.
             */
            MutexImpl(pthread_mutex_t* mutex, bool loggingAllowed);
            
            /**
             * Shared implementation for locking the mutex.
             */
            void lockImpl(ReturnCode::Type& returnCode);
            
            /**
             * Shared implementation for unlocking the mutex.
             */
            void unlockImpl(ReturnCode::Type& returnCode);
            
#if defined(USE_CUSTOM_RECURSIVE_MUTEX)
            /**
             * Invoked before unlocking a mutex (from a locked-state).
             */
            void preUnlock(ReturnCode::Type& returnCode);
            /**
             * Invoked after locking a mutex (from a non-locked state).
             */
            void postLock(ReturnCode::Type& returnCode);
#endif
            
            /**
             * the handle to the mutex.
             */
            pthread_mutex_t* mutex;
            
            bool loggingAllowed;
            
#if defined(USE_CUSTOM_RECURSIVE_MUTEX)
            /**
             * Thread that currently hold the lock. The value is only valid if lockCount > 0.
             */
            pthread_t threadId;
            /**
             * Number of that the current thread locked the mutex.
             */
            UINT8 lockCount;
#endif
            friend class Mutex;
            friend class ConditionVariableImpl; // access to mutex
        };
      }
    }
  }
}

#endif
