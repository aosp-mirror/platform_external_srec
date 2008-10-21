/*---------------------------------------------------------------------------*
 *  ReturnCode.cpp                                                           *
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
#include "ReturnCode.h"

using namespace android::speech::recognition;


const char* ReturnCode::toString(Type code)
{
  // *** Please also update JNIHelper::getJavaException() ***
  switch (code)
  {
    case SUCCESS:
      return "SUCCESS";
    case ILLEGAL_ARGUMENT:
      return "Illegal or inappropriate argument (ILLEGAL_ARGUMENT)";
    case NOT_SUPPORTED:
      return "Operation is not supported by the implementation (NOT_SUPPORTED)";
    case OUT_OF_MEMORY:
      return "Could not allocate memory to perform the operation (OUT_OF_MEMORY)";
    case INVALID_STATE:
      return "Invalid State. We cannot call this method at this point in time (INVALID_STATE)";
    case FILE_NOT_FOUND:
      return "Specified file could not be found (FILE_NOT_FOUND)";
    case PENDING_DATA:
      return "No data is currently available but more is expected to arrive in the "
             "future (PENDING_DATA)";
    case END_OF_STREAM:
      return "End of stream has been reached. (END_OF_STREAM)";
    case SOCKET_IO_ERROR:
      return "Socket I/O error (SOCKET_IO_ERROR)";
    case SOCKET_CLOSED:
      return "Socket was closed (SOCKET_CLOSED)";
    case UNKNOWN_MODULE:
      return "Library could not be loaded (UNKNOWN_MODULE)";
    case UNKNOWN_SYMBOL:
      return "Library does not contain a specific symbol. (UNKNOWN_SYMBOL)";
    case NO_MATCH:
      return "The specified search criteria did not yield any results (NO_MATCH)";
    case OPEN_ERROR:
      return "An error occured while opening a file or socket (OPEN_ERROR)";
    case READ_ERROR:
      return "An error occured while reading a file or socket (READ_ERROR)";
    case WRITE_ERROR:
      return "An error occured while writing to a file or socket (WRITE_ERROR)";
    case ARRAY_INDEX_OUT_OF_BOUNDS:
      return "The array index was either negative or greater than or equal to the size of "
             "the array. (ARRAY_INDEX_OUT_OF_BOUNDS)";
    case GRAMMAR_SLOT_FULL:
      return "The grammar slot is full and no further items may be added to it (GRAMMAR_SLOT_FULL)";
    case HOMONYM_COLLISION:
      return "Item cannot be added into a grammar slot because another item with the same "
             "pronunciation already resides in the slot. (HOMONYM_COLLISION)";
    case ALREADY_LOCKED:
      return "Mutex or condition variable is already locked by someone else. (ALREADY_LOCKED)";
    case THREAD_ERROR:
      return "The thread-related operation has failed (THREAD_ERROR)";
    case TIMEOUT:
      return "The previous operation has timed out (TIMEOUT)";
    case AUDIO_DRIVER_ERROR:
      return "An error has occured in the audio module (AUDIO_DRIVER_ERROR)";
    case AUDIO_ALREADY_IN_USE:
      return "The AudioStream passed in as an argument is already used by another resource (AUDIO_ALREADY_IN_USE)";
    case OVERFLOW_ERROR:
      return "Variable or buffer overflow (OVERFLOW_ERROR)";
    case PARSE_ERROR:
      return "Could not parse network request. (PARSE_ERROR)";
    case SPEECH_SERVER_UNAVAILABLE:
      return "Could not connect to speech server. Speech server is unavailable. (SPEECH_SERVER_UNAVAILABLE)";
    case SERVER_BUSY:
      return "Could not connect to server. Server is busy (SERVER_BUSY)";
    case SERVER_SHUTTING_DOWN:
      return "Server is shutting down, connection was lost. (SERVER_SHUTTING_DOWN)";
    case UNDERFLOW_ERROR:
      return "Variable or buffer underflow (UNDERFLOW_ERROR)";
    case UNKNOWN:
      return "An unknown error has occured (UNKNOWN)";
    case GRAMMAR_LOAD_FAILURE:
      return "Unable to load the grammar (GRAMMAR_LOAD_FAILURE)";
    case INVALID_URL:
      return "The specified URL is invalid (INVALID_URL)";
    case GRAMMAR_NOT_DEFINED:
      return "The specified grammar was not defined (GRAMMAR_NOT_DEFINED)";
    case INVALID_PARAMETER_NAME:
      return "The parameter you are tyring to set/get is invalid (INVALID_PARAMETER_NAME)";
    case INVALID_PARAMETER_VALUE:
      return "The value for the parameter you are tyring to set is invalid (INVALID_PARAMETER_VALUE)";
    case UNSUPPORTED_CODEC:
      return "The specified codec is not supported (UNSUPPORTED_CODEC)";
    case SERVER_PING_MISSED:
      return "Missed ping request from server. Server or network must be down (SERVER_PING_MISSED)";
    default:
      return "UNKNOWN_RETURN_CODE";
  }
}
