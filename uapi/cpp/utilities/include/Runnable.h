/*---------------------------------------------------------------------------*
 *  Runnable.h                                                               *
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

#ifndef _THREAD_PACKAGE_H_
#define _THREAD_PACKAGE_H_

#include "exports.h"
#include "types.h"
#include <pthread.h>
#include "ReturnCode.h"

#if defined(UAPI_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

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
#define UAPI_INVALID_THREAD_ID ((unsigned long)-1)
      
        enum ThreadPriority
        {
          MIN_THR_PRIORITY,
          NORM_THR_PRIORITY,
          HIGH_THR_PRIORITY,
          HIGHER_THR_PRIORITY,
          MAX_THR_PRIORITY
        };
        
        
        /**
         * Derive from this class if you want to execute code from a new thread.
         */
        class UAPI_EXPORT Runnable
        {
          public:
            /**
             * Constructor using default thread scheduling (Round-robin) and priority
             * (medium).
             *
             * @param mutex used to synchronize access to internal state
             */
            Runnable(Mutex* mutex);
            
            /**
             * Destructor, frees resources allocated for the thread.
             */
            virtual ~Runnable();
            
            /**
             * Method that is called when the thread is started.
             *
             * @return SUCCESS if the thread terminated normally
             */
            virtual ReturnCode::Type runThread() = 0;
            
            /**
             * Indicates whether the object should be deleted when the thread shuts down.
             *
             * @param value true if the object should be deleted when the thread shuts down
             * @returnCode SUCCESS unless a fatal error occurs
             */
            void setDeleteOnShutdown(bool value, ReturnCode::Type& returnCode);
            
            /**
             * Creates the thread.
             *
             * WARNING: a Runnable may not be reused. Once the thread terminates it may not be
             * restarted.
             *
             * PRECONDITION: The thread must be locked over the Runnable mutex.
             *
             * @param returnCode THREAD_ERROR in case of failure.
             * INVALID_STATE if the thread was already used.
             */
            void start(ReturnCode::Type& returnCode);
            
            /**
             * Blocks until the thread shuts down. The return values of multiple threads joining on the
             * same thread is undefined.
             *
             * @param returnCode THREAD_ERROR in case of failure
             */
            void join(ReturnCode::Type& returnCode);
            
            /**
             * Call this method to change the priority of the thread.
             *
             * @param priority the new priority of the thread.
             * @param returnCode SUCCESS unless a fatal error occurs
             */
            void setPriority(const ThreadPriority priority, ReturnCode::Type& returnCode);
            
            /**
             * Change the stack size of the thread.
             *
             * @param size New stack size of the thread
             * @param returnCode SUCCESS unless a fatal error occurs
             */
            void setStackSize(UINT32 size, ReturnCode::Type& returnCode);
            
            /**
             * Returns the thread stack size.
             *
             * @param returnCode SUCCESS unless a fatal error occurs
             * @return the thread stack size.
             */
            UINT32 getStackSize(ReturnCode::Type& returnCode) const;
            
            /**
             * Suspends the current thread for a minimum of the specified number of
             * milliseconds.
             *
             * @param milliseconds the number of milliseconds to sleep
             */
            static void sleep(UINT32 milliseconds);
            
            
            /**
             * Returns the current thread id.
             *
             * @param threadId the string to copy the id into
             * @param size the size of threadId in bytes
             * @param returnCode SUCCESS unless a fatal error occurs
             */
            static void getCurrentThreadId(char* threadId, ARRAY_LIMIT size, ReturnCode::Type& returnCode);
            
            /**
             * Indicates if the thread is running.
             *
             * @param returnCode SUCCESS unless a fatal error occurs
             */
            bool isRunning(ReturnCode::Type& returnCode);
          private:
            /**
             * This static function is passed into pthread_create.  It is the start of
             * the thread routine.  In turn, it calls the virtual function runThread()
             * which is written by the user of this class.
             * @param obj pointer to the Runnable object (the thread class).
             */
            static void* start_routine(void* obj);
            
            /**
             * Thread cleanup, called in the destructor
             *
             * @param returnCode SUCCESS unless a fatal error occurs
             */
            void terminate(ReturnCode::Type& returnCode);
            
            /**
             * initial stack size
             */
            UINT32 stackSize;
            
            /**
             * Indicates if the object should be deleted when the thread shuts down.
             */
            bool deleteOnShutdown;
          protected:
            /**
             * handle to the thread.
             */
            pthread_t thread;
            
            /**
             * Used to synchronize access to internal state
             */
            Mutex* mutex;
            
            /**
             * Prevent copying.
             */
            Runnable(const Runnable &);
            /**
             * Prevent assignment.
             */
            Runnable& operator=(const Runnable&);
            
            /**
             * Flag that indicates if the thread was started.
             */
            bool running;
            
            /**
             * True if the Runnable has been started before.
             */
            bool used;
        };
      }
    }
  }
}

#endif
