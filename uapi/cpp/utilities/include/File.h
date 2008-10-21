/*---------------------------------------------------------------------------*
 *  File.h                                                                   *
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

#ifndef __UAPI_FILE__
#define __UAPI_FILE__

/*
 * @file
 * Portable file API.
 */
#include "exports.h"
#include "types.h"
#include "ReturnCode.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        /**
         * Portable file.
         */
        class UAPI_EXPORT File
        {
          public:
            enum SeekType
            {
              /**
               * seek from the start of the file
               */
              UAPI_SEEK_SET,
              /**
               * seek from the end of the file
               */
              UAPI_SEEK_END,
              /**
               * seek from the current position
               */
              UAPI_SEEK_CUR
            };
            
            
            /**
             * destructor
             */
            virtual ~File()
            {}
            
            /**
             * Closes the File.
             *
             * @param returnCode the return code
             */
            virtual void close(ReturnCode::Type& returnCode) = 0;
            
            /**
             * Opens a File.
             * @param path path of the file to open
             * @param mode the mode in which we have to open the file. Refer to fopen
             * documentation to know which values are supporeted.
             * @param returnCode the return code
             */
            virtual void open(const char* path , const char* mode, ReturnCode::Type& returnCode) = 0;
            
            /**
             * Reads from a File.
             * @param buffer Pointer to data to be written
             * @param size Item size in bytes
             * @param in_out_nNumItems [in/out] Maximum number of items to be read.
             *                         On output, contains the number of full items
             *                         actually written, which may be less than
             *                         count if an error occurs.
             * @param returnCode the return code
             */
            virtual void read(void* buffer,
                              UINT32 size,
                              UINT32& in_out_nNumItems,
                              ReturnCode::Type& returnCode) = 0;
                              
            /**
             * Writes data to a File.
             * @param buffer Pointer to data to be written
             * @param size Item size in bytes
             * @param in_out_nNumItems [in/out] Maximum number of items to be read.
             *                         On output, contains the number of full items
             *                         actually written, which may be less than
             *                         count if an error occurs.
             * @param returnCode the return code
             */
            virtual void write(void* buffer,
                               UINT32 size,
                               UINT32& in_out_nNumItems,
                               ReturnCode::Type& returnCode) = 0;
                               
            /**
             * Seek into a File.
             *
             * @param offset Number of bytes from <code>origin</code>
             * @param origin Initial position
             * @param returnCode the return code
             */
            virtual void seek(LONG offset, SeekType origin, ReturnCode::Type& returnCode) = 0;
            
            /**
             * Gets the current position of a File.
             *
             * @param position [out] The position
             * @param returnCode the return code
             */
            virtual void getPosition(UINT32& out_position, ReturnCode::Type& returnCode) = 0;
            
            /**
             * Used to know if the file is opened or closed.
             *
             * @return true if the file is opened.
             */
            virtual bool isOpened() const = 0;
            
            /**
             * Used to flush the content of the internal buffer into the file
             */
            virtual void flush() = 0;
        };
      }
    }
  }
}

#endif

