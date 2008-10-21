/*---------------------------------------------------------------------------*
 *  LibraryLoader.cpp                                                        *
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



#include "LibraryLoader.h"
#include "LoggerImpl.h"
#include "ReturnCode.h"

#if defined(UAPI_WIN32)
#  include <direct.h>
#elif defined(UAPI_LINUX)
#  include <dlfcn.h>
#endif

#include <stdio.h>
#include <stdlib.h>

using namespace android::speech::recognition::utilities;


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      LibraryLoader* LibraryLoader::create(ReturnCode::Type& returnCode)
      {
        LibraryLoader* result = new LibraryLoader(returnCode);
        if (result == 0)
        {
          returnCode = ReturnCode::OUT_OF_MEMORY;
          return 0;
        }
        if (returnCode != ReturnCode::SUCCESS)
        {
          delete result;
          return 0;
        }
        returnCode = ReturnCode::SUCCESS;
        return result;
      }
      
      
      LibraryLoader::~LibraryLoader()
      {}
      
      void LibraryLoader::load(char const* libname, ReturnCode::Type& returnCode)
      {
        UAPI_FN_SCOPE("LibraryLoader::load");
        UAPI_INFO(fn,"libname %s\n", libname);
        
        //increase the count of  this library. If the count is greater than 1, this
        //means that the library was already loaded and we have nothing to do.
        UINT8 newCount = addRef(returnCode);
        if (returnCode != ReturnCode::SUCCESS)
        {
          UAPI_ERROR(fn,"Failed to addRef on libraryLoader %s\n", libname);
          return;
        }
        
        if (newCount > 1)
        {
          //nothing to do, already opened.
          returnCode = ReturnCode::SUCCESS;
          return;
        }
        
        char dl_name[256];
#if defined(UAPI_WIN32)
        _snprintf(dl_name, 256, "%s.dll", libname);
        
        m_handle = LoadLibrary((LPCSTR) & dl_name);
        if (m_handle == NULL_HANDLE)
        {
          char workingDirectory[_MAX_PATH];
          if (_getcwd(workingDirectory, _MAX_PATH) == 0)
            strcpy(workingDirectory, "");
          UAPI_WARN(fn,"Could not load library %s, code=%d, workingDirectory=%s\n", dl_name, GetLastError(), workingDirectory);
          returnCode = ReturnCode::UNKNOWN_MODULE;
          ReturnCode::Type rc;
          LibraryLoader::release(rc);
          return;
        }
#elif defined(UAPI_LINUX)
        snprintf(dl_name, 256, "lib%s.so", libname);
        
        m_handle = dlopen(dl_name, RTLD_NOW | RTLD_GLOBAL);
        if (m_handle == NULL_HANDLE)
        {
          char const* dl_error_msg;
          dl_error_msg = dlerror();
          UAPI_WARN(fn,"Could not load library %s, error=%s\n", dl_name, dl_error_msg);
          returnCode = ReturnCode::UNKNOWN_MODULE;
          ReturnCode::Type rc;
          LibraryLoader::release(rc);
          return;
        }
#endif
        returnCode = ReturnCode::SUCCESS;
      }
      
      void* LibraryLoader::symbolToAddress(const char* symbol, ReturnCode::Type& returnCode)
      {
        UAPI_FN_SCOPE("LibraryLoader::symbolToAddress");
        UAPI_INFO(fn,"symbol %s\n", symbol);
        
        void* address = 0;
        returnCode = ReturnCode::SUCCESS;
#if defined(UAPI_WIN32)
        address = GetProcAddress(m_handle, symbol);
        if (address == 0)
        {
          UAPI_WARN(fn,"Could not find symbol %s, code=%d\n", symbol, GetLastError());
          returnCode = ReturnCode::UNKNOWN_SYMBOL;
          return 0;
        }
#elif defined(UAPI_LINUX)
        address = dlsym(m_handle, symbol);
        if (address == 0)
        {
          char const* dl_error_msg;
          dl_error_msg = dlerror();
          UAPI_WARN(fn,"Could not find symbol %s, error=%s\n", symbol, dl_error_msg);
          returnCode = ReturnCode::UNKNOWN_SYMBOL;
          return 0;
        }
#endif
        return address;
      }
      
      void LibraryLoader::close(ReturnCode::Type& returnCode)
      {
        UAPI_FN_SCOPE("LibraryLoader::close");
          
        UINT8 newCount = LibraryLoader::release(returnCode);
        if (returnCode != ReturnCode::SUCCESS)
        {
          UAPI_ERROR(fn,"Failed to Release the count on library\n");
          return;
        }
        
        if (newCount == 0)
        {
#if defined(UAPI_WIN32)
          FreeLibrary(m_handle);
#elif defined(UAPI_LINUX)
          dlclose(m_handle);
#endif
        }
      }
      
      UINT8 LibraryLoader::release(ReturnCode::Type& returnCode)
      {
        UAPI_FN_SCOPE("LibraryLoader::release");
          
        LockScope ls(m_mutex, returnCode);
        if (returnCode != ReturnCode::SUCCESS)
        {
          UAPI_ERROR(fn,"Failed to create lockscope\n");
          return m_nRefCount;
        }
        returnCode = ReturnCode::SUCCESS;
        m_nRefCount--;
        return m_nRefCount;
      }
    }
  }
}
