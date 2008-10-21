/*---------------------------------------------------------------------------*
 *  RefCounted.cpp                                                           *
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
#include "RefCounted.h"
#include "Logger.h"

using namespace android::speech::recognition::utilities;


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      RefCounted::RefCounted(ReturnCode::Type& returnCode):
          m_nRefCount(1)
      {
        init(returnCode);
      }
      
      RefCounted::RefCounted(UINT8 initial_count, ReturnCode::Type& returnCode):
          m_nRefCount(initial_count)
      {
        init(returnCode);
      }
      
      void RefCounted::init(ReturnCode::Type& returnCode)
      {
        UAPI_FN_NAME("RefCounted::init");
          
        m_mutex = Mutex::create(returnCode);
        if (returnCode != ReturnCode::SUCCESS)
        {
          UAPI_ERROR(fn,"Could not create mutex\n");
          return;
        }
        returnCode = ReturnCode::SUCCESS;
      }
      
      RefCounted::~RefCounted()
      {
        if (m_mutex)
          delete m_mutex;
      }
      
      
      UINT8 RefCounted::addRef(ReturnCode::Type& returnCode)
      {
        UAPI_FN_NAME("RefCounted::addRef");
          
        LockScope ls(m_mutex, returnCode);
        if (returnCode != ReturnCode::SUCCESS)
        {
          UAPI_ERROR(fn,"failed to create LockScope\n");
          return m_nRefCount;
        }
        returnCode = ReturnCode::SUCCESS;
        m_nRefCount++;
        return m_nRefCount;
      }
      
      UINT8 RefCounted::removeRef(ReturnCode::Type& returnCode)
      {
        UAPI_FN_NAME("RefCounted::removeRef");
          
        {
          LockScope ls(m_mutex, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"failed to create LockScope\n");
            return m_nRefCount;
          }
          returnCode = ReturnCode::SUCCESS;
          m_nRefCount--;
          if (m_nRefCount)
            return m_nRefCount;
        }
        delete this;
        return 0;
      }
      
      UINT8 RefCounted::getCount() const
      {
        return m_nRefCount;
      }
    }
  }
}
