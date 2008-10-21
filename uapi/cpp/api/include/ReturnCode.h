/*---------------------------------------------------------------------------*
 *  ReturnCode.h                                                             *
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

#ifndef __UAPI__RETURNCODE
#define __UAPI__RETURNCODE

#include "exports.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      /**
       * All possible return codes.
       */
      class ReturnCode
      {
        public:
          enum Type
          {
            /**
             * Indicates that the operation completed successfully.
             *
             * DESIGN NOTE: This value must be equal to zero to ensure that: <code>if (returnCode)</code>
             * returns true in case of an error, false otherwise.
             */
            SUCCESS = 0,
            /**
             * An unknown error has occured.
             */
            UNKNOWN,
            /**
             * Indicates that a method has been passed an illegal or inappropriate argument.
             */
            ILLEGAL_ARGUMENT,
            /**
             * The optional operation is not supported by the implementation.
             */
            NOT_SUPPORTED,
            /**
             * Could not allocate memory to perform the operation.
             */
            OUT_OF_MEMORY,
            /**
             * Indicates that we cannot call this method at this point in time.
             */
            INVALID_STATE,
            /**
             * Indicates that the specified file could not be found.
             */
            FILE_NOT_FOUND,
            /**
             * Indicates that no data is currently available but more is expected to arrive in the future.
             */
            PENDING_DATA,
            /**
             * Indicates that the end of stream has been reached.
             */
            END_OF_STREAM,
            /**
             * Indicates that a socket error happened.
             */
            SOCKET_IO_ERROR,
            /**
             * Indicates that the socket was closed
             */
            SOCKET_CLOSED,
            /**
             * Indicates that a library could not be loaded
             */
            UNKNOWN_MODULE,
            /**
             * Indicates that a library does not contain a specific symbol
             */
            UNKNOWN_SYMBOL,
            /**
             * Indicates that the specified search criteria did not yield any results.
             */
            NO_MATCH,
            /**
             * Indicates an error occurs while opening a file or socket.
             */
            OPEN_ERROR,
            /**
             * Indicates an error occured while reading a file or socket.
             */
            READ_ERROR,
            /**
             * Indicates an error occured while writing to a file or socket.
             */
            WRITE_ERROR,
            /**
             * Indicates that an array index was either negative or greater than or equal to the size
             * of the array.
             */
            ARRAY_INDEX_OUT_OF_BOUNDS,
            /**
             * Indicates that the grammar slot is full and no further items may be added to it.
             */
            GRAMMAR_SLOT_FULL,
            /**
             * Indicates that an item cannot be added into a grammar slot because another item with
             * the same pronunciation already resides in the slot.
             */
            HOMONYM_COLLISION,
            /**
             * Indicates that the mutex or condition variable is already locked by someone else.
             */
            ALREADY_LOCKED,
            /**
             * Indicates that the thread-related operation has failed.
             */
            THREAD_ERROR,
            /**
             * Indicates that the previous operation has timed out.
             */
            TIMEOUT,
            /**
             * Indicates that an error occured when invoking an audio driver function.
             */
            AUDIO_DRIVER_ERROR,
            /**
             * The audio passed in as an argument is already in use by another module.
             */
            AUDIO_ALREADY_IN_USE,
            /**
             * Indicates that the operation failed because it would have caused a variable or buffer to
             * overflow its maximum value or capacity.
             */
            OVERFLOW_ERROR,
            /**
             * Indicates that the operation failed because it would have caused a variable or buffer to
             * underflow its minimum value or capacity.
             */
            UNDERFLOW_ERROR,
            /**
             * Could not parse a request coming from the server.
             */
            PARSE_ERROR,
            /**
             * The network speech server is not available.
             */
            SPEECH_SERVER_UNAVAILABLE,
            /**
             * The network server is busy.
             */
            SERVER_BUSY,
            /**
             * The network server is shutting down
             */
            SERVER_SHUTTING_DOWN,
            /**
             * Unable to load the grammar.
             */
            GRAMMAR_LOAD_FAILURE,
            /**
             * The specified URL is invalid.
             */
            INVALID_URL,
            /**
             * No grammar defined.
             */
            GRAMMAR_NOT_DEFINED,
            /**
             * the parameter you are tyring to set/get is invalid.
             */
            INVALID_PARAMETER_NAME,
            /**
             * the value for the parameter you are tyring to set is invalid.
             */
            INVALID_PARAMETER_VALUE,
            /**
             * This codec is not supported
             */
            UNSUPPORTED_CODEC,
            /**
             * We did not receive the ping request from the server. The server
             * or the network connection must be down.
             */
            SERVER_PING_MISSED,
            /**
             * Return codes having this value or greater are invalid.
             */
            MAXIMUM_BOUND
          };
          
          /**
           * Returns the textual representation of the specified return-code.
           *
           * @param code the return code
           */
          UAPI_EXPORT static const char* toString(Type code);
      };
    }
  }
}

#endif
