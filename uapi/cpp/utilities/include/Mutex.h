/*---------------------------------------------------------------------------*
 *  Mutex.h                                                                  *
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

#ifndef __UAPI__MUTEX_
#define __UAPI__MUTEX_

#include "exports.h"
#include "ReturnCode.h"
#include "Logger.h"


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
        class UAPI_EXPORT Mutex
        {
          public:
          
            /**
             * Creates a new Mutex that is allowed to log.
             *
             * @param returnCode THREAD_ERROR on failure.
             * @return a valid mutex handle if SUCCESS, 0 on failure.
             */
            static Mutex* create(ReturnCode::Type& returnCode);
            
            /**
             * Creates a new Mutex.
             *
             * @param loggingAllowed true if the object and its dependencies are allowed logging
             * @param returnCode THREAD_ERROR on failure.
             * @return a valid mutex handle if SUCCESS, 0 on failure.
             */
            static Mutex* create(bool loggingAllowed, ReturnCode::Type& returnCode);
            
            /**
             * Destructor. Releases the mutex handle.
             */
            virtual ~Mutex();
            
            /**
             * Locks the mutex
             *
             * @param returnCode THREAD_ERROR on failure.
             */
            virtual void lock(ReturnCode::Type& returnCode) = 0;
            
            /**
             * Releases the lock on the mutex.
             *
             * @param returnCode THREAD_ERROR on failure.
             */
            virtual void unlock(ReturnCode::Type& returnCode) = 0;
            
            /**
             * Attempts to enter the protected section without blocking. If the call is
             * successful, the calling thread takes ownership of the mutex.
             *
             * @param returnCode ALREADY_LOCKED if the mutex is already in locked by another thread
             * locked, THREAD_ERROR if another error occurs.
             */
            virtual void tryLock(ReturnCode::Type& returnCode) = 0;
            
            /**
             * Returns true if the object and its dependencies are allowed to log.
             *
             * @return true if the object and its dependencies are allowed to log
             */
            virtual bool isLoggingAllowed() const = 0;
          protected:
            /**
             * Prevent construction.
             */
            Mutex();
        };
        
        /**
         * Class used to automatically lock and unlock a mutex. The mutex scope will
         * have the same scope as the variable of type LockScope.
         * Ex:
         *
         *  ... safe code
         *  {
         *    LockScope ls(mutex);
         *    ... //unsafe code
         *  }
         *  ... safe code
         *
         *  is the same as:
         *
         *  ... safe code
         *  m_mutex.Lock();
         *  ... //unsafe code
         *  m_mutex.UnLock();
         *  ... safe code
         *
         */
        class UAPI_EXPORT LockScope
        {
          public:
            /**
             * Contructor. Will lock the mutex.
             *
             * @param mutex The Mutex to lock and unlock within the variable scope.
             * @param returnCode THREAD_ERROR on failure.
             */
            LockScope(Mutex* mutex, ReturnCode::Type& returnCode);
            
            /**
             * Cancel the LockScope before the scope is over, this is useful if the
             * scope covers cases where the owner of the mutex could be destructed, and
             * potentially causing unlock on invalid mutex when the scope is over.   By
             * cancelling it before the desstructor of the owner of mutex is called,
             * the unlock will not be attempted at the end of the scope.  Cancel() can
             * be called more than once before the end of the scope.  Only the first
             * call will actually do the UnLock, the subsequent calls are no-op. Cancel
             * also causes the subseqeuent call to IsValid() returns false.
             *
             * @param returnCode THREAD_ERROR on failure.
             */
            void cancel(ReturnCode::Type& returnCode);
            
            /**
             * Destructor. Will unlock the mutex.
             */
            ~LockScope();
            
          private:
            /**
             * Prevent copying.
             */
            LockScope& operator=(LockScope& other);
            
            /**
             * A reference to the mutex handle.
             */
            Mutex* lockable;
            bool   loggingAllowed;
        };
      }
    }
  }
}

#endif
