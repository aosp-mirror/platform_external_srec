/*---------------------------------------------------------------------------*
 *  FileASCII.cpp                                                            *
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
#include <string.h>
#include "ReturnCode.h"
#include "FileASCII.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::utilities;


FileASCII::~FileASCII()
{
  ReturnCode::Type returnCode;
  close(returnCode);
}

void FileASCII::close(ReturnCode::Type& returnCode)
{
  if (m_fp != 0)
  {
    int ret = fclose(m_fp);
    if (ret != 0)
    {
      returnCode = ReturnCode::UNKNOWN;
      return;
    }
    m_fp = 0;
  }
  returnCode = ReturnCode::SUCCESS;
}

void FileASCII::open(const char* path , const char* mode, ReturnCode::Type& returnCode)
{
  if (path == 0 || mode == 0 || strlen(path) == 0)
  {
    returnCode = ReturnCode::ILLEGAL_ARGUMENT;
    return;
  }
  
  if (m_fp != 0)
  {
    returnCode = ReturnCode::INVALID_STATE;
    return;
  }
  
  m_fp = fopen(path, mode);
  if (m_fp == 0)
  {
    returnCode = ReturnCode::FILE_NOT_FOUND;
    return;
  }
  returnCode = ReturnCode::SUCCESS;
}

void FileASCII::read(void* buffer, UINT32 size, UINT32& in_out_nNumItems,
                     ReturnCode::Type& returnCode)
{
  if (buffer == 0 || in_out_nNumItems <= 0)
  {
    returnCode = ReturnCode::ILLEGAL_ARGUMENT;
    return;
  }
  
  if (m_fp == 0)
  {
    returnCode = ReturnCode::INVALID_STATE;
    return;
  }
  
  UINT32 numRead = (UINT32)fread(buffer, size, in_out_nNumItems, m_fp);
  if (numRead != in_out_nNumItems)
  {
    in_out_nNumItems = numRead;
    returnCode = ReturnCode::READ_ERROR;
    return;
  }
  
  in_out_nNumItems = numRead;
  returnCode = ReturnCode::SUCCESS;
}

void FileASCII::write(void* buffer, UINT32 size, UINT32& in_out_nNumItems,
                      ReturnCode::Type& returnCode)
{
  if (buffer == 0 || in_out_nNumItems <= 0)
  {
    returnCode = ReturnCode::ILLEGAL_ARGUMENT;
    return;
  }
  
  if (m_fp == 0)
  {
    returnCode = ReturnCode::INVALID_STATE;
    return;
  }
  
  UINT32 numWritten = (UINT32) fwrite(buffer, size, in_out_nNumItems, m_fp);
  if (numWritten != in_out_nNumItems)
  {
    in_out_nNumItems = numWritten;
    returnCode = ReturnCode::WRITE_ERROR;
    return;
  }
  
  in_out_nNumItems = numWritten;
  returnCode = ReturnCode::SUCCESS;
}

void FileASCII::seek(LONG offset, SeekType origin, ReturnCode::Type& returnCode)
{
  if (m_fp == 0)
  {
    returnCode = ReturnCode::INVALID_STATE;
    return;
  }
  
  int whence;
  switch (origin)
  {
    case UAPI_SEEK_SET:
      whence = SEEK_SET;
      break;
    case UAPI_SEEK_END:
      whence = SEEK_END;
      break;
    case UAPI_SEEK_CUR:
      whence = SEEK_CUR;
      break;
    default:
      returnCode = ReturnCode::ILLEGAL_ARGUMENT;
      return;
  }
  
  int ret = fseek(m_fp, offset, whence);
  if (ret == -1)
  {
    returnCode = ReturnCode::UNKNOWN;
    return;
  }
  returnCode = ReturnCode::SUCCESS;
}

void FileASCII::getPosition(UINT32& out_position, ReturnCode::Type& returnCode)
{
  if (m_fp == 0)
  {
    returnCode = ReturnCode::INVALID_STATE;
    return;
  }
  
  long pos = ftell(m_fp);
  if (pos == -1)
  {
    out_position = 0;
    returnCode = ReturnCode::UNKNOWN;
    return;
  }
  
  out_position = (UINT32) pos;
  returnCode = ReturnCode::SUCCESS;
}

void FileASCII::flush()
{
  if (m_fp != 0)
    fflush(m_fp);
}
