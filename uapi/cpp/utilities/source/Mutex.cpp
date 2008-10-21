/*---------------------------------------------------------------------------*
 *  Mutex.cpp                                                                *
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
#include "Mutex.h"
#include "MutexImpl.h"
#include "Logger.h"
#include <stdio.h>

using namespace android::speech::recognition::utilities;


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        Mutex* Mutex::create(ReturnCode::Type& returnCode)
        {
          return create(true, returnCode);
        }
        
        Mutex* Mutex::create(bool loggingAllowed, ReturnCode::Type& returnCode)
        {
          UAPI_FN_NAME("Mutex::create");

          MutexImpl* mutex = MutexImpl::create(loggingAllowed, returnCode);
          if (returnCode)
          {
            const char* message = "Failed to create MutexImpl\n";
            if (loggingAllowed)
            {
              UAPI_ERROR(fn,message);
            }
            else
            {
              fprintf(stderr, message);
            }
            delete mutex;
            return 0;
          }
          return mutex;
        }
        
        Mutex::Mutex()
        {}
        
        Mutex::~Mutex()
        {}
        
        
        LockScope::LockScope(Mutex* mutex, ReturnCode::Type& returnCode):
            lockable(mutex),
            loggingAllowed( mutex == 0 ? false : mutex->isLoggingAllowed())
        {
          UAPI_FN_NAME("LockScope::LockScope");
          
          if (!lockable)
          {
            const char* message = "mutex handle is null\n";
            if (loggingAllowed)
            {
              UAPI_ERROR(fn,message);
            }
            else
            {
              fprintf(stderr, message);
            }
            returnCode = ReturnCode::ILLEGAL_ARGUMENT;
            return;
          }
          lockable->lock(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            lockable = 0;
            const char* message = "Unable to lock mutex\n";
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
        }
        
        
        void LockScope::cancel(ReturnCode::Type& returnCode)
        {
          UAPI_FN_NAME("LockScope::cancel");
          
          if (lockable)
          {
            lockable->unlock(returnCode);
            lockable = 0;
            if (returnCode == ReturnCode::INVALID_STATE)
            {
              const char* message = "LockScope.cancel(): mutex was already unlocked\n";
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
            else if (returnCode)
            {
              const char* message = "Unable to unlock mutex\n";
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
            lockable = 0;
          }
          else
          {
            const char* message = "LockScope.cancel(): LockScope does not own the mutex\n";
            if (loggingAllowed)
            {
              UAPI_ERROR(fn,message);
            }
            else
            {
              fprintf(stderr, message);
            }
            returnCode = ReturnCode::SUCCESS;
          }
        }
        
        /**
         * Destructor. Will unlock the mutex.
         */
        LockScope::~LockScope()
        {
          UAPI_FN_NAME("LockScope::~LockScope");

          ReturnCode::Type returnCode;
          
          if (lockable)
          {
            cancel(returnCode);
            if (returnCode)
            {
              const char* message = "Unable to cancel lock on mutex\n";
              if (loggingAllowed)
              {
                UAPI_ERROR(fn,message);
              }
              else
              {
                fprintf(stderr, message);
              }
            }
          }
        }


      }
    }
  }
}
