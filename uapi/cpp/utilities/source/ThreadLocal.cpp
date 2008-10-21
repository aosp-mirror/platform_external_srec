/*---------------------------------------------------------------------------*
 *  ThreadLocal.cpp                                                          *
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
#include "ThreadLocal.h"
#include <stdio.h>
#include <assert.h>


//
// NOTE: The Logger API depends on ThreadLocal so it must use printf instead
//
namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        ThreadLocal::ThreadLocal(pthread_key_t _key, void (*_onThreadShutdown)(void*)):
            key(_key),
            onThreadShutdown(_onThreadShutdown)
        {}
        
        ThreadLocal::~ThreadLocal()
        {
          int rc = pthread_key_delete(key);
          switch (rc)
          {
            case 0:
              return;
            case EINVAL:
              fprintf(stderr, "pthread_key_delete(): The key value is invalid\n");
              assert(false);
            default:
              fprintf(stderr, "pthread_key_delete(): unknown error (%d)\n", rc);
              assert(false);
              return;
          }
        }
        
        ThreadLocal* ThreadLocal::create(void (*onThreadShutdown)(void*), ReturnCode::Type& returnCode)
        {
          pthread_key_t key;
          int rc = pthread_key_create(&key, onThreadShutdown);
          switch (rc)
          {
            case 0:
              break;
            case EAGAIN:
              returnCode = ReturnCode::OUT_OF_MEMORY;
              fprintf(stderr, "pthread_key_create(): The system lacked the "
                      "necessary resources to create another thread-specific data key, "
                      "or the system-imposed limit on the total number of keys per "
                      "process {PTHREAD_KEYS_MAX} has been exceeded\n");
              assert(false);
              return 0;
            case ENOMEM:
              returnCode = ReturnCode::OUT_OF_MEMORY;
              fprintf(stderr, "pthread_key_create(): Insufficient memory exists "
                      "to create the key\n");
              assert(false);
              return 0;
            default:
              returnCode = ReturnCode::THREAD_ERROR;
              fprintf(stderr, "pthread_key_create(): unknown error (%d)\n", rc);
              assert(false);
              return 0;
          }
          ThreadLocal* result = new ThreadLocal(key, onThreadShutdown);
          if (!result)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return 0;
          }
          returnCode = ReturnCode::SUCCESS;
          return result;
        }
        
        void ThreadLocal::set(void* value, ReturnCode::Type& returnCode)
        {
          int rc = pthread_setspecific(key, value);
          switch (rc)
          {
            case 0:
              returnCode = ReturnCode::SUCCESS;
              return;
            case ENOMEM:
              returnCode = ReturnCode::OUT_OF_MEMORY;
              fprintf(stderr, "pthread_setspecific(): Insufficient memory exists "
                      "to associate the non-NULL value with the key\n");
              assert(false);
              return;
            case EINVAL:
              returnCode = ReturnCode::THREAD_ERROR;
              fprintf(stderr, "pthread_setspecific(): The key value is invalid\n");
              assert(false);
              return;
            default:
              returnCode = ReturnCode::THREAD_ERROR;
              fprintf(stderr, "pthread_setspecific(): unknown error (%d)\n", rc);
              assert(false);
              return;
          }
        }
        
        void* ThreadLocal::get(ReturnCode::Type& returnCode)
        {
          returnCode = ReturnCode::SUCCESS;
          return pthread_getspecific(key);
        }
      }
    }
  }
}
