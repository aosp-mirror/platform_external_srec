/*---------------------------------------------------------------------------*
 *  CodecHelper.h                                                            *
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

#ifndef __UAPI__CODECHELPER_
#define __UAPI__CODECHELPER_

#include "types.h"
#include "Codec.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class UAPI_EXPORT CodecHelper
        {
          public:
            /**
             * Function used to know the size of AudioBuffer(s) for a certain codec
             */
            static UINT16 GetPreferredBufferSize(Codec::Type codec);
            
            /**
             * Function used to know the number of bytes we should read when
             * using a media file reader.
             */
            static UINT16 GetPreferredFileReaderBufferSize(Codec::Type codec);

            /**
             * Function used to know the buffer duration (in ms) for a certain codec
             */
            static UINT32 GetPreferredRealTimeDelay(Codec::Type codec);

            /**
             * Get the number of miliseconds that a number of bytes represent
             * for a certain codec.
             */
            static UINT32 GetNumMsecForNumBytes(Codec::Type codec, UINT32 fileLength);

            /**
             * returns the codec that corresponds to the sample rate and the number
             * of bytes per sample
             */
            static Codec::Type GetCodecFromRateNumBytesPerSample(INT32 sampleRate, INT16 numBytesPerSample, ReturnCode::Type & returnCode);

          private:
            /**
             * Prevent construction.
             */
            CodecHelper();
        };
      }
    }
  }
}

#endif
