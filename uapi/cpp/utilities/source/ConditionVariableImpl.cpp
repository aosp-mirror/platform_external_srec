/*---------------------------------------------------------------------------*
 *  ConditionVariableImpl.cpp                                                *
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

#ifdef UAPI_WIN32
// Do not warn on "while(true)"
#  pragma warning (disable: 4127)
#endif

#include "ConditionVariableImpl.h"
#include "ConditionVariable.h"
#include "LoggerImpl.h"
#include "MutexImpl.h"

#include <time.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <pthread.h>

using namespace android::speech::recognition::utilities;


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      ConditionVariable* ConditionVariable::create(Mutex* mutex, ReturnCode::Type& returnCode)
      {
        UAPI_FN_NAME("ConditionVariable::create");
          
        if (!mutex)
        {
          UAPI_ERROR(fn,"Tried to create a ConditionVariable with a null mutex");
          returnCode = ReturnCode::ILLEGAL_ARGUMENT;
          return 0;
        }
        
        pthread_cond_t* condition = new pthread_cond_t;
        if (!condition)
        {
          returnCode = ReturnCode::OUT_OF_MEMORY;
          return 0;
        }
        int rc = pthread_cond_init(condition, 0);
        if (rc)
          delete condition;
        switch (rc)
        {
          case 0:
            break;
          case EAGAIN:
            returnCode = ReturnCode::OUT_OF_MEMORY;
            UAPI_ERROR(fn,"pthread_cond_init(): The system lacked the necessary "
                       "resources (other than memory) to initialize another condition variable\n");
            return 0;
          case ENOMEM:
            returnCode = ReturnCode::OUT_OF_MEMORY;
            UAPI_ERROR(fn,"pthread_cond_init(): Insufficient memory exists to "
                       "initialize the condition variable\n");
            return 0;
          case EBUSY:
            returnCode = ReturnCode::INVALID_STATE;
            UAPI_ERROR(fn,"pthread_cond_init(): The implementation has detected an "
                       "attempt to re-initialize the object referenced by cond, a previously "
                       "initialized, but not yet destroyed, condition variable\n");
            return 0;
          case EINVAL:
            returnCode = ReturnCode::INVALID_STATE;
            UAPI_ERROR(fn,"pthread_cond_init(): The value specified by attr is invalid\n");
            return 0;
          default:
            returnCode = ReturnCode::THREAD_ERROR;
            UAPI_ERROR(fn,"pthread_cond_init(): unknown error (%d)\n", rc);
            return 0;
        }
        
        MutexImpl* mutexImpl = (MutexImpl*) mutex;
        ConditionVariable* result = new ConditionVariableImpl(mutexImpl, condition);
        if (result == 0)
        {
          pthread_cond_destroy(condition);
          delete condition;
          returnCode = ReturnCode::OUT_OF_MEMORY;
          return 0;
        }
        returnCode = ReturnCode::SUCCESS;
        return result;
      }
      
      ConditionVariableImpl::ConditionVariableImpl(MutexImpl* _mutex, pthread_cond_t* _condition):
          mutex(_mutex),
          condition(_condition),
          signaled(false)
      {}
      
      ConditionVariableImpl::~ConditionVariableImpl()
      {
        UAPI_FN_NAME("ConditionVariable::ConditionVariable");
          
        int rc = pthread_cond_destroy(condition);
        switch (rc)
        {
          case 0:
            break;
          case EBUSY:
            UAPI_ERROR(fn,"pthread_cond_destroy(): The implementation has detected an "
                       "attempt to destroy the object referenced by cond while it is referenced "
                       "(for example, while being used in a pthread_cond_wait() or "
                       "pthread_cond_timedwait()) by another thread\n");
            break;
          case EINVAL:
            UAPI_ERROR(fn,"pthread_cond_destroy(): The value specified by cond is invalid\n");
            break;
          default:
            UAPI_ERROR(fn,"pthread_cond_destroy(): unknown error (%d)\n", rc);
        }
        delete condition;
      }
      
      void ConditionVariableImpl::signal(ReturnCode::Type& returnCode)
      {
        UAPI_FN_NAME("ConditionVariable::signal");
          
        int rc = pthread_cond_signal(condition);
        switch (rc)
        {
          case 0:
            signaled = true;
            returnCode = ReturnCode::SUCCESS;
            break;
          case EINVAL:
            returnCode = ReturnCode::THREAD_ERROR;
            UAPI_ERROR(fn,"pthread_cond_signal(): The value cond does not refer "
                       "to an initialized condition variable\n");
            break;
          default:
            returnCode = ReturnCode::THREAD_ERROR;
            UAPI_ERROR(fn,"pthread_cond_signal(): unknown error (%d)\n", rc);
            break;
        }
      }
      
      void ConditionVariableImpl::wait(ReturnCode::Type& returnCode)
      {
        UAPI_FN_NAME("ConditionVariable::wait");
          
#ifdef USE_CUSTOM_RECURSIVE_MUTEX
        mutex->preUnlock(returnCode);
        if (returnCode)
          return;
#endif
        while (true)
        {
          int rc = pthread_cond_wait(condition, mutex->mutex);
          switch (rc)
          {
            case 0:
              if (!signaled)
              {
                // Avoid spurious wakeups
                continue;
              }
              else
                signaled = false;
#ifdef USE_CUSTOM_RECURSIVE_MUTEX
              mutex->postLock(returnCode);
              if (returnCode)
                return;
#endif
              returnCode = ReturnCode::SUCCESS;
              return;
            case EINVAL:
              returnCode = ReturnCode::THREAD_ERROR;
              UAPI_ERROR(fn,"pthread_cond_wait(): The value specified by cond, "
                         "mutex, or abstime is invalid\n");
              return;
            case EPERM:
              returnCode = ReturnCode::THREAD_ERROR;
              UAPI_ERROR(fn,"pthread_cond_wait(): The mutex was not owned by "
                         "the current thread at the time of the call\n");
              return;
            default:
              returnCode = ReturnCode::THREAD_ERROR;
              UAPI_ERROR(fn,"pthread_cond_wait(): unknown error (%d)\n", rc);
              return;
          }
        }
      }
      
      void ConditionVariableImpl::wait(UINT32 milliseconds, ReturnCode::Type& returnCode)
      {
        UAPI_FN_NAME("ConditionVariable::wait(timeout)");
          
        timespec abstime;
        if (milliseconds == UINT32_MAX)
        {
          //UAPI_INFO(fn,"ConditionVariable::TimedWaitMs will wait forever\n");
          //special case, wait INFINITE!!!
          abstime.tv_sec = LONG_MAX;
          abstime.tv_nsec = 0;
        }
        else
        {
          TimeInstant timeout = TimeInstant::now();
          timeout = timeout.plus(milliseconds);
          timeval timeoutVal = timeout.toTimeval();
          abstime.tv_sec = timeoutVal.tv_sec;
          abstime.tv_nsec = timeoutVal.tv_usec * 1000;
        }
        
#ifdef USE_CUSTOM_RECURSIVE_MUTEX
        mutex->preUnlock(returnCode);
        if (returnCode)
          return;
#endif
        while (true)
        {
          int rc = pthread_cond_timedwait(condition, mutex->mutex, &abstime);
          switch (rc)
          {
            case 0:
              if (!signaled)
              {
                // Avoid spurious wakeups
                continue;
              }
              else
                signaled = false;
#ifdef USE_CUSTOM_RECURSIVE_MUTEX
              mutex->postLock(returnCode);
              if (returnCode)
                return;
#endif
              returnCode = ReturnCode::SUCCESS;
              return;
            case ETIMEDOUT:
              returnCode = ReturnCode::TIMEOUT;
              UAPI_INFO(fn,"pthread_cond_timedwait(): The time specified by "
                        "abstime to pthread_cond_timedwait() has passed\n");
              return;
            case EINVAL:
              returnCode = ReturnCode::THREAD_ERROR;
              UAPI_ERROR(fn,"pthread_cond_timedwait(): The value specified by "
                         "cond, mutex, or abstime is invalid\n");
              return;
            case EPERM:
              returnCode = ReturnCode::THREAD_ERROR;
              UAPI_ERROR(fn,"pthread_cond_wait(): The mutex was not owned by "
                         "the current thread at the time of the call\n");
              return;
            default:
              returnCode = ReturnCode::THREAD_ERROR;
              UAPI_ERROR(fn,"pthread_cond_timedwait(): unknown error (%d)\n", rc);
              return;
          }
        }
      }
    }
  }
}
