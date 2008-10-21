/*---------------------------------------------------------------------------*
 *  Runnable.cpp                                                             *
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

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#if defined(UAPI_LINUX)
# include <pthread.h>
# include <sys/errno.h>
# include <sys/select.h>
# define THREAD_STACK_SIZE 0x80000
#elif defined(UAPI_WIN32)
# include <errno.h>
# include <process.h>
# define THREAD_STACK_SIZE 0
#endif

#include "Runnable.h"
#include "LoggerImpl.h"
#include "Mutex.h"
#include "ConditionVariable.h"

using namespace android::speech::recognition::utilities;


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      void Runnable::getCurrentThreadId(char* threadId, ARRAY_LIMIT size, ReturnCode::Type& returnCode)
      {
#if defined(UAPI_WIN32)
        if (_snprintf(threadId, size, "%08lx", GetCurrentThreadId()) < 0)
          returnCode = ReturnCode::OVERFLOW_ERROR;
        else
          returnCode = ReturnCode::SUCCESS;
#elif defined(UAPI_LINUX)
        if (snprintf(threadId, size, "%10lu", pthread_self()) >= size)
          returnCode = ReturnCode::OVERFLOW_ERROR;
        else
          returnCode = ReturnCode::SUCCESS;
#endif
      }
      
      Runnable::Runnable(Mutex* _mutex):
          stackSize(THREAD_STACK_SIZE),
          deleteOnShutdown(false),
          mutex(_mutex),
          running(false),
          used(false)
      {
        assert(mutex);
      }
      
      Runnable::~Runnable()
      {
        ReturnCode::Type dummy;
        terminate(dummy);
        delete mutex;
      }
      
      void Runnable::setStackSize(UINT32 size, ReturnCode::Type& returnCode)
      {
        LockScope ls(mutex, returnCode);
        if (returnCode)
          return;
        stackSize = size;
        returnCode = ReturnCode::SUCCESS;
      }
      
      UINT32 Runnable::getStackSize(ReturnCode::Type& returnCode) const
      {
        LockScope ls(mutex, returnCode);
        if (returnCode)
          return 0;
        returnCode = ReturnCode::SUCCESS;
        return stackSize;
      }
      
      void Runnable::start(ReturnCode::Type& returnCode)
      {
        UAPI_FN_SCOPE("Runnable::start");
          
        // WARNING: Invoke ConditionVariable.wait() while holding more than one lock over
        // a recursive mutex will result in a deadlock.
        if (used)
        {
          UAPI_ERROR(fn,"INVALID_STATE\n");
          returnCode = ReturnCode::INVALID_STATE;
          return;
        }
        else if (running)
        {
          UAPI_ERROR(fn,"This thread is already running or not joined\n");
          returnCode = ReturnCode::THREAD_ERROR;
          return;
        }
        
        pthread_attr_t attr;
        int rc = pthread_attr_init(&attr);
        switch (rc)
        {
          case 0:
            break;
          case ENOMEM:
            returnCode = ReturnCode::OUT_OF_MEMORY;
            UAPI_WARN(fn,"pthread_attr_init(): Insufficient memory exists to "
                      "initialize the thread attributes object\n");
            return;
          default:
            UAPI_WARN(fn,"pthread_attr_init(): unknown error (%d)\n", rc);
            returnCode = ReturnCode::THREAD_ERROR;
            return;
        }
        
        rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        if (rc)
          pthread_attr_destroy(&attr);
        switch (rc)
        {
          case 0:
            break;
          case EINVAL:
            returnCode = ReturnCode::ILLEGAL_ARGUMENT;
            UAPI_WARN(fn,"pthread_attr_setdetachstate(): The value of detachstate was not valid\n");
            return;
          default:
            UAPI_WARN(fn,"pthread_attr_setdetachstate(): unknown error (%d)\n", rc);
            returnCode = ReturnCode::THREAD_ERROR;
            return;
        }
        
        rc = pthread_attr_setstacksize(&attr, stackSize);
        if (rc)
          pthread_attr_destroy(&attr);
        switch (rc)
        {
          case 0:
            break;
          case EINTR:
            returnCode = ReturnCode::ILLEGAL_ARGUMENT;
            UAPI_WARN(fn,"pthread_attr_setstacksize(): The value of stacksize is less "
                      "than PTHREAD_STACK_MIN or exceeds a system-imposed limit\n");
            return;
          default:
            UAPI_WARN(fn,"pthread_attr_setstacksize(): unknown error (%d)\n", rc);
            returnCode = ReturnCode::THREAD_ERROR;
            return;
        }
        
        rc = pthread_create(&thread, &attr, &Runnable::start_routine, (void*) this);
        switch (rc)
        {
          case 0:
            break;
          case EAGAIN:
            returnCode = ReturnCode::OUT_OF_MEMORY;
            UAPI_ERROR(fn,"pthread_create(): The system lacked the necessary resources to "
                       "create another thread, or the system-imposed limit on the total number of "
                       "threads in a process PTHREAD_THREADS_MAX would be exceeded\n");
            return;
          case EINVAL:
            returnCode = ReturnCode::THREAD_ERROR;
            UAPI_ERROR(fn,"pthread_create(): The value specified by attr is invalid\n");
            return;
          case EPERM:
            returnCode = ReturnCode::THREAD_ERROR;
            UAPI_ERROR(fn,"pthread_create(): The caller does not have appropriate permission to "
                       "set the required scheduling parameters or scheduling policy\n");
            return;
          default:
            UAPI_WARN(fn,"pthread_attr_destroy(): unknown error (%d)\n", rc);
            returnCode = ReturnCode::THREAD_ERROR;
            return;
        }
        rc = pthread_attr_destroy(&attr);
        switch (rc)
        {
          case 0:
            break;
          case ENOMEM:
            UAPI_WARN(fn,"pthread_attr_destroy(): Insufficient memory exists to initialize "
                      "the thread attributes object\n");
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          default:
            UAPI_WARN(fn,"pthread_attr_destroy(): unknown error (%d)\n", rc);
            returnCode = ReturnCode::THREAD_ERROR;
            return;
        }
      }
      
      
      void Runnable::join(ReturnCode::Type& returnCode)
      {
        UAPI_FN_SCOPE("Runnable::join");
          
        if (pthread_equal(pthread_self(), thread))
        {
          UAPI_ERROR(fn,"Thread tried to join() on itself\n");
          returnCode = ReturnCode::THREAD_ERROR;
          return;
        }
        
        void* exitStatus;
        LockScope ls(mutex, returnCode);
        if (returnCode)
        {
          UAPI_ERROR(fn,"Failed to lock mutex in Runnable::join()\n");
          return;
        }
        if (!running && !used)
        {
          // Joining on a Runnable that was never started
          returnCode = ReturnCode::SUCCESS;
          return;
        }
        ls.cancel(returnCode);
        if (returnCode)
        {
          UAPI_ERROR(fn,"Failed to unlock mutex in Runnable::join()\n");
          return;
        }
        int rc = pthread_join(thread, &exitStatus);
        switch (rc)
        {
          case 0:
            returnCode = ReturnCode::SUCCESS;
            return;
          case EINVAL:
            returnCode = ReturnCode::THREAD_ERROR;
            UAPI_ERROR(fn,"pthread_join(): The implementation has detected that the "
                       "value specified by thread does not refer to a joinable thread\n");
            return;
          case ESRCH:
            // Thread was already detached
            UAPI_INFO(fn,"pthread_join(): No thread could be found corresponding to that "
                      "specified by the given thread ID\n");
            returnCode = ReturnCode::SUCCESS;
            return;
          case EDEADLK:
            returnCode = ReturnCode::THREAD_ERROR;
            UAPI_ERROR(fn,"pthread_join(): A deadlock was detected or the value of thread "
                       "specifies the calling thread\n");
            return;
          default:
            returnCode = ReturnCode::THREAD_ERROR;
            UAPI_WARN(fn,"pthread_attr_destroy(): unknown error (%d)\n", rc);
            return;
        }
      }
      
      void Runnable::setDeleteOnShutdown(bool value, ReturnCode::Type& returnCode)
      {
        LockScope ls(mutex, returnCode);
        if (returnCode)
          return;
        returnCode = ReturnCode::SUCCESS;
        deleteOnShutdown = value;
      }
      
      void* Runnable::start_routine(void* objInstance)
      {
        ReturnCode::Type returnCode;
        UAPI_FN_SCOPE("Runnable::start_routine");
        Runnable* threadObj = (Runnable*) objInstance;
        
        // call the pure virtual method.
        {
          LockScope ls(threadObj->mutex, returnCode);
          if (returnCode)
          {
            UAPI_ERROR(fn,"Failed to lock the thread mutex\n");
            return 0;
          }
          threadObj->running = true;
          threadObj->used = true;
        }
        ReturnCode::Type status = threadObj->runThread();
        {
          LockScope ls(threadObj->mutex, returnCode);
          if (returnCode)
          {
            UAPI_ERROR(fn,"Failed to lock the thread mutex\n");
            return 0;
          }
          threadObj->running = false;
          if (returnCode)
          {
            UAPI_ERROR(fn,"Failed to lock the thread mutex\n");
            return 0;
          }
          
          if (threadObj->deleteOnShutdown)
          {
            ls.cancel(returnCode);
            if (returnCode)
            {
              UAPI_ERROR(fn,"Failed to unlock the thread mutex\n");
              return 0;
            }
            delete threadObj;
          }
        }
        return (void*) status;
      }
      
      bool Runnable::isRunning(ReturnCode::Type& returnCode)
      {
        LockScope ls(mutex, returnCode);
        if (returnCode)
          return false;
        return running;
      }
      
      void Runnable::terminate(ReturnCode::Type& returnCode)
      {
        UAPI_FN_SCOPE("Runnable::terminate");
          
        bool running = isRunning(returnCode);
        if (returnCode)
          return;
        if (running)
        {
#if defined(ANDROID)
          // do this since as of 10/1/2007 pthread_cancel() and pthread_detach() are not supported
          join(returnCode);
          return;
#else
          // issue a cancel message to thread
          int rc = pthread_cancel(thread);
          switch (rc)
          {
            case 0:
              break;
            case ESRCH:
              returnCode = ReturnCode::THREAD_ERROR;
              UAPI_ERROR(fn,"pthread_cancel(): No thread could be found corresponding to that specified by the given thread ID\n");
              return;
            default:
              returnCode = ReturnCode::THREAD_ERROR;
              UAPI_ERROR(fn,"pthread_cancel(): Unknown error (%d)\n", rc);
              return;
          }
          rc = pthread_detach(thread);
          switch (rc)
          {
            case EINVAL:
              returnCode = ReturnCode::THREAD_ERROR;
              UAPI_ERROR(fn,"pthread_detach(): The implementation has detected that the value specified by thread does not refer to a joinable thread\n");
              return;
            case ESRCH:
              returnCode = ReturnCode::THREAD_ERROR;
              UAPI_ERROR(fn,"pthread_detach(): No thread could be found corresponding to that specified by the given thread ID\n");
              return;
            default:
              returnCode = ReturnCode::THREAD_ERROR;
              UAPI_ERROR(fn,"pthread_detach(): Unknown error (%d)\n", rc);
              return;
          }
#endif
        }
        else
          returnCode = ReturnCode::SUCCESS;
      }
      
      static const int UAPI_THREAD_PRIORITY_MIN = 127 / 4;
      static const int UAPI_THREAD_PRIORITY_NORM = 127 / 2;
      static const int UAPI_THREAD_PRIORITY_HIGH = (127 * 9) / 16;
      static const int UAPI_THREAD_PRIORITY_HIGHER = (127 * 10) / 16;
      static const int UAPI_THREAD_PRIORITY_MAX = (127 * 3) / 4;
      
      void Runnable::setPriority(const ThreadPriority _priority, ReturnCode::Type& returnCode)
      {
        UAPI_FN_SCOPE("Runnable::setPriority");
          
        LockScope ls(mutex, returnCode);
        if (returnCode)
          return;
        int priority;
        switch (_priority)
        {
          case MIN_THR_PRIORITY:
            priority =  UAPI_THREAD_PRIORITY_MIN;
            break;
          case HIGH_THR_PRIORITY:
            priority = UAPI_THREAD_PRIORITY_HIGH;
            break;
          case HIGHER_THR_PRIORITY:
            priority = UAPI_THREAD_PRIORITY_HIGHER;
            break;
          case MAX_THR_PRIORITY:
            priority = UAPI_THREAD_PRIORITY_MAX;
            break;
          default:
            returnCode = ReturnCode::ILLEGAL_ARGUMENT;
            return;
        }
        
        sched_param param;
        int policy;
        
        int rc = pthread_getschedparam(thread, &policy, &param);
        switch (rc)
        {
          case 0:
            break;
          case ENOSYS:
            returnCode = ReturnCode::THREAD_ERROR;
            UAPI_ERROR(fn,"pthread_getschedparam(): The option _POSIX_THREAD_PRIORITY_SCHEDULING "
                       "is not defined and the implementation does not support the function\n");
            return;
          case ESRCH:
            returnCode = ReturnCode::THREAD_ERROR;
            UAPI_ERROR(fn,"pthread_getschedparam(): The value specified by thread does "
                       "not refer to a existing thread.\n");
            return;
          default:
            returnCode = ReturnCode::THREAD_ERROR;
            UAPI_ERROR(fn,"pthread_getschedparam(): unknown error (%d)\n", rc);
            return;
        }
        param.sched_priority = priority;
        rc = pthread_setschedparam(thread, policy, &param);
        switch (rc)
        {
          case 0:
            break;
          case ENOSYS:
            returnCode = ReturnCode::THREAD_ERROR;
            UAPI_ERROR(fn,"pthread_setschedparam(): The option _POSIX_THREAD_PRIORITY_SCHEDULING "
                       "is not defined and the implementation does not support the function\n");
            return;
          case EINVAL:
            returnCode = ReturnCode::THREAD_ERROR;
            UAPI_ERROR(fn,"pthread_setschedparam(): The value specified by policy or one of "
                       "the scheduling parameters associated with the scheduling policy policy is invalid\n");
            return;
#if defined(ENOTSUP)
          case ENOTSUP:
            returnCode = ReturnCode::NOT_SUPPORTED;
            UAPI_ERROR(fn,"pthread_setschedparam(): An attempt was made to set the policy or "
                       "scheduling parameters to an unsupported value\n");
            return;
#endif
          case EPERM:
            returnCode = ReturnCode::INVALID_STATE;
            UAPI_ERROR(fn,"pthread_setschedparam(): The caller does not have the appropriate "
                       "permission to set either the scheduling parameters or the scheduling policy "
                       "of the specified thread. Or, the implementation does not allow the "
                       "application to modify one of the parameters to the value specified\n");
            return;
          case ESRCH:
            returnCode = ReturnCode::INVALID_STATE;
            UAPI_ERROR(fn,"pthread_setschedparam(): The value specified by thread does not "
                       "refer to a existing thread\n");
            return;
          default:
            returnCode = ReturnCode::THREAD_ERROR;
            UAPI_ERROR(fn,"pthread_setschedparam(): unknown error (%d)\n", rc);
            return;
        }
        returnCode = ReturnCode::SUCCESS;
      }
      
      void Runnable::sleep(UINT32 milliseconds)
      {
        if (milliseconds <= 0)
          return;
#if defined(UAPI_LINUX)
        timeval sleep_tv;
        
        sleep_tv.tv_sec = milliseconds / 1000;                 // seconds
        sleep_tv.tv_usec = 1000 * (milliseconds % 1000);       // microseconds
        select(0, 0, 0, 0, &sleep_tv);
#else
        ::Sleep(milliseconds);
#endif
      }
    }
  }
}
