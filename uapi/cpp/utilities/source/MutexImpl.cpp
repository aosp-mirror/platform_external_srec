/*---------------------------------------------------------------------------*
 *  MutexImpl.cpp                                                            *
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

#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#include "exports.h"
#include "ReturnCode.h"
#include "Logger.h"
#include "MutexImpl.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::utilities;


MutexImpl::MutexImpl(pthread_mutex_t* _mutex, bool _loggingAllowed):
    mutex(_mutex),
    loggingAllowed(_loggingAllowed)
{
#ifdef USE_CUSTOM_RECURSIVE_MUTEX
  // Simulate a recursive mutex by keeping track of # lock attempts within the same thread
  lockCount = 0;
#endif
}

MutexImpl* MutexImpl::create(bool loggingAllowed, ReturnCode::Type& returnCode)
{
  UAPI_FN_NAME("MutexImpl::create");
  
  pthread_mutex_t* mutex = new pthread_mutex_t;
  if (!mutex)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return 0;
  }
  int rc;
  pthread_mutexattr_t attr;
  rc = pthread_mutexattr_init(&attr);
  if (rc)
    delete mutex;
  switch (rc)
  {
    case 0:
      break;
    case ENOMEM:
    {
      const char* message = "pthread_mutexattr_init(): Insufficient memory exists to initialize "
                      "the mutex attributes object\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::THREAD_ERROR;
      return 0;
    }
    default:
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,"pthread_mutexattr_init(): unknown error (%d)\n", rc);
      }
      else
      {
        fprintf(stderr, "pthread_mutexattr_init(): unknown error (%d)\n", rc);
      }
      returnCode = ReturnCode::THREAD_ERROR;
      return 0;
  }
#ifdef USE_CUSTOM_RECURSIVE_MUTEX
#ifdef ANDROID
  rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
#else
  rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
#endif
#else
  rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif
  if (rc)
  {
    pthread_mutexattr_destroy(&attr);
    delete mutex;
  }
  switch (rc)
  {
    case 0:
      break;
    case EINVAL:
    {
      const char* message = "pthread_mutexattr_settype(): The value type is invalid or the "
                      "value specified by attr is invalid\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::THREAD_ERROR;
      return 0;
    }
    default:
    {
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,"pthread_mutexattr_settype(): unknown error (%d)\n", rc);
      }
      else
      {
        fprintf(stderr, "pthread_mutexattr_settype(): unknown error (%d)\n", rc);
      }
      returnCode = ReturnCode::THREAD_ERROR;
      return 0;
    }
  }
  rc = pthread_mutex_init(mutex, &attr);
  if (rc)
  {
    pthread_mutexattr_destroy(&attr);
    delete mutex;
  }
  switch (rc)
  {
    case 0:
      break;
    case EAGAIN:
    {
      const char* message = "pthread_mutex_init(): The system lacked the necessary resources "
                      "(other than memory) to initialize another mutex\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::OUT_OF_MEMORY;
      return 0;
    }
    case ENOMEM:
    {
      const char* message = "pthread_mutex_init(): Insufficient memory exists to initialize the "
                      "mutex\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::OUT_OF_MEMORY;
      return 0;
    }
    case EPERM:
    {
      const char* message = "pthread_mutex_init(): The caller does not have the privilege to "
                      "perform the operation\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::THREAD_ERROR;
      return 0;
    }
    case EBUSY:
    {
      const char* message = "pthread_mutex_init(): The implementation has detected an attempt to "
                      "re-initialize the object referenced by mutex, a previously initialized, but not "
                      "yet destroyed, mutex\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::THREAD_ERROR;
      return 0;
    }
    case EINVAL:
    {
      const char* message = "pthread_mutex_init(): The value specified by attr is invalid\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::THREAD_ERROR;
      return 0;
    }
    default:
    {
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,"pthread_mutex_init(): unknown error (%d)\n", rc);
      }
      else
      {
        fprintf(stderr, "pthread_mutex_init(): unknown error (%d)\n", rc);
      }
      returnCode = ReturnCode::THREAD_ERROR;
      return 0;
    }
  }
  
  rc = pthread_mutexattr_destroy(&attr);
  if (rc)
    delete mutex;
  switch (rc)
  {
    case 0:
      break;
    case EINVAL:
    {
      const char* message = "pthread_attr_destroy(): attr had an invalid value\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::THREAD_ERROR;
      return 0;
    }
    default:
    {
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,"pthread_attr_destroy(): unknown error (%d)\n", rc);
      }
      else
      {
        fprintf(stderr, "pthread_attr_destroy(): unknown error (%d)\n", rc);
      }
      returnCode = ReturnCode::THREAD_ERROR;
      return 0;
    }
  }
  MutexImpl* result = new MutexImpl(mutex, loggingAllowed);
  if (!result)
  {
    pthread_mutex_destroy(mutex);
    delete mutex;
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return 0;
  }
  returnCode = ReturnCode::SUCCESS;
  return result;
}

MutexImpl::~MutexImpl()
{
  UAPI_FN_NAME("MutexImpl::~MutexImpl");
  
  int rc = pthread_mutex_destroy(mutex);
  switch (rc)
  {
    case 0:
      break;
    case EBUSY:
    {
      const char* message = "The implementation has detected an attempt to destroy the object "
                      "referenced by mutex while it is locked or referenced (for example, while being "
                      "used in a pthread_cond_wait() or pthread_cond_timedwait()) by another thread\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      break;
    }
    case EINVAL:
    {
      const char* message = "pthread_mutex_destroy(): The value specified by mutex is invalid\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      break;
    }
    default:
    {
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,"pthread_mutex_destroy(): unknown error (%d)\n", rc);
      }
      else
      {
        fprintf(stderr, "pthread_mutex_destroy(): unknown error (%d)\n", rc);
      }
      break;
    }
  }
  delete mutex;
}

void MutexImpl::lockImpl(ReturnCode::Type& returnCode)
{
  UAPI_FN_NAME("MutexImpl::lockImpl");
  
  int rc = pthread_mutex_lock(mutex);
  switch (rc)
  {
    case 0:
      returnCode = ReturnCode::SUCCESS;
      return;
    case EINVAL:
    {
      const char* message = "pthread_mutex_lock(): The mutex was created with the protocol "
                      "attribute having the value PTHREAD_PRIO_PROTECT and the calling thread's "
                      "priority is higher than the mutex's current priority ceiling.\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::INVALID_STATE;
      return;
    }
    case EAGAIN:
    {
      const char* message = "pthread_mutex_lock(): The mutex could not be acquired because the "
                      "maximum number of recursive locks for mutex has been exceeded\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::INVALID_STATE;
      return;
    }
    case EDEADLK:
    {
      const char* message = "pthread_mutex_lock(): The current thread already owns the mutex\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::INVALID_STATE;
      return;
    }
    default:
    {
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,"pthread_mutex_lock(): unknown error (%d)\n", rc);
      }
      else
      {
        fprintf(stderr, "pthread_mutex_lock(): unknown error (%d)\n", rc);
      }
      returnCode = ReturnCode::THREAD_ERROR;
      return;
    }
  }
}

void MutexImpl::lock(ReturnCode::Type& returnCode)
{
  
#ifdef USE_CUSTOM_RECURSIVE_MUTEX
  UAPI_FN_NAME("MutexImpl::lock");

  // Determine if lock will succeed, but don't block if it won't.
  tryLock(returnCode);
  switch (returnCode)
  {
    case ReturnCode::SUCCESS:
      postLock(returnCode);
      if (returnCode)
        return;
      break;
    case ReturnCode::ALREADY_LOCKED:
      // The mutex is already locked
      if (lockCount > 0 && pthread_equal(threadId, pthread_self()))
      {
        // The current thread owns the mutex so increment the number
        // of times it locked over it.
        if (lockCount == UINT8_MAX)
        {
          returnCode = ReturnCode::OVERFLOW_ERROR;
          const char* message = "MutexImpl::lockCount overflowed";
          if (loggingAllowed)
          {
            UAPI_ERROR(fn,message);
          }
          else
          {
            fprintf(stderr, message);
          }
          return;
        }
        ++lockCount;
      }
      else
      {
        // The current thread does not own the mutex, so we lock over it
        // knowing that it will block.
        lockImpl(returnCode);
        if (returnCode)
          return;
        postLock(returnCode);
        if (returnCode)
          return;
      }
      break;
    default:
      return;
  }
#else
  lockImpl(returnCode);
  if (returnCode)
    return;
#endif
  returnCode = ReturnCode::SUCCESS;
}

void MutexImpl::unlockImpl(ReturnCode::Type& returnCode)
{
  UAPI_FN_NAME("MutexImpl::unlockImpl");
  
  int rc = pthread_mutex_unlock(mutex);
  switch (rc)
  {
    case 0:
      returnCode = ReturnCode::SUCCESS;
      return;
    case EINVAL:
    {
      const char* message = "pthread_mutex_unlock(): The value specified "
                      "by mutex does not refer to an initialized mutex object\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::THREAD_ERROR;
      return;
    }
    case EAGAIN:
    {
      const char* message = "pthread_mutex_unlock(): The mutex could not "
                      "be acquired because the maximum number of recursive locks "
                      "for mutex has been exceeded\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::THREAD_ERROR;
      return;
    }
    case EPERM:
    {
      const char* message = "pthread_mutex_unlock(): The current thread does "
                      "not own the mutex\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::INVALID_STATE;
      return;
    }
    default:
    {
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,"pthread_mutex_unlock(): unknown error (%d)\n", rc);
      }
      else
      {
        fprintf(stderr, "pthread_mutex_unlock(): unknown error (%d)\n", rc);
      }
      returnCode = ReturnCode::THREAD_ERROR;
      return;
    }
  }
}

#ifdef USE_CUSTOM_RECURSIVE_MUTEX
void MutexImpl::postLock(ReturnCode::Type& returnCode)
{
  // Take ownership of the mutex
  threadId = pthread_self();
  lockCount = 1;
  returnCode = ReturnCode::SUCCESS;
}

void MutexImpl::preUnlock(ReturnCode::Type& returnCode)
{
  if (lockCount > 0 && pthread_equal(threadId, pthread_self()))
  {
    // Unlocking the mutex from the same thread that locked it.
    if (lockCount > 0)
      --lockCount;
  }
  returnCode = ReturnCode::SUCCESS;
}
#endif

void MutexImpl::unlock(ReturnCode::Type& returnCode)
{
  
#ifdef USE_CUSTOM_RECURSIVE_MUTEX
  preUnlock(returnCode);
  if (returnCode)
    return;
  if (lockCount == 0)
  {
    // No more nested locks, so the mutex can finally be unlocked.
    unlockImpl(returnCode);
    if (returnCode)
      return;
  }
#else
  unlockImpl(returnCode);
  if (returnCode)
    return;
#endif
  returnCode = ReturnCode::SUCCESS;
}

void MutexImpl::tryLock(ReturnCode::Type& returnCode)
{
  UAPI_FN_NAME("MutexImpl::tryLock");
  
  int rc = pthread_mutex_trylock(mutex);
  switch (rc)
  {
    case 0:
    {
#ifdef USE_CUSTOM_RECURSIVE_MUTEX
      postLock(returnCode);
      if (returnCode)
        return;
#endif
      returnCode = ReturnCode::SUCCESS;
      return;
    }
    case EINVAL:
    {
      const char* message = "pthread_mutex_trylock(): The mutex was created with the protocol "
                      "attribute having the value PTHREAD_PRIO_PROTECT and the calling thread's "
                      "priority is higher than the mutex's current priority ceiling.\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::INVALID_STATE;
      return;
    }
    case EBUSY:
    {
      const char* message = "pthread_mutex_trylock(): The mutex could not be acquired because "
                      "it was already locked\n";
      if (loggingAllowed)
      {
        UAPI_INFO(fn,message);
      }
      returnCode = ReturnCode::ALREADY_LOCKED;
      return;
    }
    case EAGAIN:
    {
      const char* message = "pthread_mutex_trylock(): The mutex could not be acquired because the "
                      "maximum number of recursive locks for mutex has been exceeded\n";
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,message);
      }
      else
      {
        fprintf(stderr, message);
      }
      returnCode = ReturnCode::INVALID_STATE;
      return;
    }
    default:
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,"pthread_mutex_trylock(): unknown error (%d)\n", rc);
      }
      else
      {
        fprintf(stderr, "pthread_mutex_trylock(): unknown error (%d)\n", rc);
      }
      return;
  }
}

bool MutexImpl::isLoggingAllowed() const
{
  return loggingAllowed;
}
