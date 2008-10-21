/*---------------------------------------------------------------------------*
 *  AudioBuffer.h                                                            *
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

#ifndef __UAPI__AUDIOBUFFER
#define __UAPI__AUDIOBUFFER

#include "exports.h"
#include "RefCounted.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        /**
         * class that holds the actual audio samples.
         *
         * Note: AudioBuffer are RefCounted. This means that every time someone
         * acquires an AudioBuffer, it must call Release on it when this someone is
         * done with it.
         */
        class AudioBuffer: public RefCounted
        {
          public:
            /**
             * Creates an instance of this class
             *
             * @param initial_count The initial reference count on the AudioBuffer,
             * i.e. the number of AudioStreamImpl attached to the AudioQueue.
             * @param bufSize the size of bytes to allocate in the internal buffer, i.e.
             * bytes allocated for pBuffer
             * @param returnCode the return code.
             */
            static AudioBuffer* create(UINT8 initial_count, UINT32 bufSize, ReturnCode::Type& returnCode);
            
            /**
             * destructor
             */
            ~AudioBuffer();
            
            
            // These are made public to make this class easy to use. View this class as
            // a C strut.
            
            /**
             * Pointer to the buffer that contains the audio data
             */
            UINT8* buffer;
            /**
             * number of UINT8 samples in the pBuffer variable.
             */
            ARRAY_LIMIT size;
            /**
             * Specifies if this buffer is the last one in this sequence of
             * AudioBuffer.
             */
            bool isLastBuffer;
            
          private:
            /**
              * constructor
              * @param initial_count The initial reference count on the AudioBuffer,
              * i.e. the number of AudioStreamImpl attached to the AudioQueue.
              * @param returnCode the return code.
              */
            AudioBuffer(UINT8 initial_count, ReturnCode::Type& returnCode);
        };
      }
    }
  }
}

#endif
