/*---------------------------------------------------------------------------*
 *  LibraryLoader.h                                                          *
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

#ifndef LIBRARY_LOADER_H__
#define LIBRARY_LOADER_H__

#include "exports.h"
#include "RefCounted.h"
#include "ReturnCode.h"

#if defined(UAPI_WIN32)
# ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
# endif
# include <windows.h>
#elif defined(UAPI_LINUX)
# include <dlfcn.h>
# include <stddef.h> // size_t
#else
# error unsupported platform
#endif


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
#if defined(UAPI_WIN32)
        typedef HINSTANCE dl_handle_t;
#elif defined(UAPI_LINUX)
        typedef void* dl_handle_t;
#else
# error unsupported platform
#endif
#define NULL_HANDLE (dl_handle_t)(NULL)
        
        class UAPI_EXPORT LibraryLoader : private RefCounted
        {
          public:
          
            /**
             * Creates an instance of this class.
             *
             * @param returnCode SUCCESS unless a fatal error has occured.
             */
            static LibraryLoader * create(ReturnCode::Type & returnCode);
            
            /**
             * Destructor
             */
            ~LibraryLoader();
            
            /**
             * call this method to load the library from the standard PATH or from
             * LD_LIBRARY_PATH.
             * @param libname name of the library to load. DO NOT include the .dll or
             * .so in the name.
             * @param returnCode ILLEGAL_ARGUMENT on failure when adding reference on the libraryLoader.
             * UNKNOWN_MODULE on failure when loading the library.
             *
             */
            void load(char const* libname, ReturnCode::Type& returnCode);
            
            /**
             * This functions returns the address of a specified symbol. It is a
             * wrapper on top of dlsym on linux and GetProcAddress on Windows.
             * @param symbol name of the symbol to load.
             * @param returnCode UNKNOWN_SYMBOL if symbol could not be found
             * @return address of the loaded symbol. Returns null if symbol could not
             * be found.
             */
            void* symbolToAddress(const char* symbol, ReturnCode::Type& returnCode);
            
            /**
             * call this method to close the opened library. The library will be closed
             * only if no one is using the library. For this reason, this class has a
             * RefCount (private RefCounted).
             *
             * @param returnCode SUCCESS unless a fatal error has occured.
             * THREAD_ERROR on failure.  ILLEGAL_ARGUMENT on failure when removing reference
             */
            void close(ReturnCode::Type& returnCode);
            
          private:
            /**
             * Constructor
             */
            LibraryLoader(ReturnCode::Type & returnCode): RefCounted(0, returnCode), m_handle(NULL_HANDLE)
            {}
            
            /**
             * Suppress copy-constructor.
             */
            LibraryLoader(LibraryLoader& other);
            /**
             * Prevent copying.
             */
            LibraryLoader& operator=(LibraryLoader& other);
            /**
             * This function decreases the reference count by 1. If the count goes down
             * to 0, then then the library is really closed. This changes the default
             * behavior of RefCounted which deletes the class when the count is down to
             * 0.
             *
             * @param returnCode THREAD_ERROR on failure.
             * ILLEGAL_ARGUMENT on failure when removing reference
             * @return the new reference count after the Release.
             */
            virtual UINT8 release(ReturnCode::Type & returnCode);
            
            /**
             * handle to the opened library.
             */
            dl_handle_t     m_handle;
        };
      }
    }
  }
}

#endif

