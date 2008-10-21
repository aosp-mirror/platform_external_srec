/*---------------------------------------------------------------------------*
 *  FileASCII.h                                                              *
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

#ifndef __UAPI_FILEASCII_H_
#define __UAPI_FILEASCII_H_

#include <stdio.h>
#include "ReturnCode.h"
#include "File.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        /**
         * Portable file ASCII implementation. Should be supported on Linux, windows,
         * WinCE.
         */
        class UAPI_EXPORT FileASCII: public File
        {
          public:
            /**
             * constructor
             */
            FileASCII() : m_fp(0)
            {}
            
            /**
             * destructor
             */
            virtual ~FileASCII();
            
            /**
             * Closes the File.
             *
             * @param returnCode SUCCESS unless a fatal error occurs
             */
            virtual void close(ReturnCode::Type& returnCode);
            
            /**
             * Opens a File.
             * @param path path of the file to open
             * @param mode the mode in which we have to open the file. Refer to fopen
             * documentation to know which values are supporeted.
             * @param returnCode SUCCESS on open completed.
             * ILLEGAL_ARGUMENT if parameters path or mode are null, or path is empty string.
             * INVALID_STATE if file handle is not valid. FILE_NOT_FOUND if file open failed.
             */
            virtual void open(const char* path , const char* mode, ReturnCode::Type& returnCode);
            
            /**
             * Reads from a File.
             * @param buffer Pointer to data to be written
             * @param size Item size in bytes
             * @param in_out_nNumItems [in/out] Maximum number of items to be read.
             *                         On output, contains the number of full items
             *                         actually written, which may be less than
             *                         count if an error occurs.
             * @param returnCode SUCCESS if read completed.
             * ILLEGAL_ARGUMENT if buffer is null or in_out_nNumItems is less equal to zero.
             * INVALID_STATE if file handle is not valid.
             * READ_ERROR if bytes read is different from in_out_nNumItems.
             */
            virtual void read(void* buffer, UINT32 size, UINT32& in_out_nNumItems,
                              ReturnCode::Type& returnCode);
                              
            /**
             * Writes data to a File.
             * @param buffer Pointer to data to be written
             * @param size Item size in bytes
             * @param in_out_nNumItems [in/out] Maximum number of items to be read.
             *                         On output, contains the number of full items
             *                         actually written, which may be less than
             *                         count if an error occurs.
             * @param returnCode SUCCESS if write completed.
             * ILLEGAL_ARGUMENT if buffer is null or in_out_nNumItems is less equal to zero.
             * INVALID_STATE if file handle is not valid.
             * WRITE_ERROR if bytes written is different from in_out_nNumItems.
             */
            virtual void write(void* buffer, UINT32 size, UINT32& in_out_nNumItems,
                               ReturnCode::Type& returnCode);
                               
            /**
             * Seek into a File.
             *
             * @param offset Number of bytes from <code>origin</code>
             * @param origin Initial position
             * @param returnCode SUCCESS if seek completed.
             * INVALID_STATE if file handle is not valid.
             * ILLEGAL_ARGUMENT if origin type is not valid (has to be one of these values UAPI_SEEK_SET,
             * UAPI_SEEK_END, UAPI_SEEK_CUR). Or on seek failure.
             */
            virtual void seek(LONG offset, SeekType origin, ReturnCode::Type& returnCode);
            
            /**
             * Gets the current position of a File.
             *
             * @param position [out] The position
             * @param returnCode SUCCESS if seek completed.
             * INVALID_STATE if file handle is not valid.
             * UNKNOWN on a fatal error.
             */
            virtual void getPosition(UINT32& out_position, ReturnCode::Type& returnCode);
            
            /**
             * Used to know if the file is opened or closed.
             *
             * @return true if the file is opened.
             */
            virtual bool isOpened() const
            {
              return m_fp != 0;
            };
            
            /**
             * Used to flush the content of the internal buffer into the file
             */
            virtual void flush();
            
          private:
            FILE* m_fp;
        };
      }
    }
  }
}

#endif
