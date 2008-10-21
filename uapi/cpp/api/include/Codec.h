/*---------------------------------------------------------------------------*
 *  Codec.h                                                                  *
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

#ifndef __UAPI__CODEC_
#define __UAPI__CODEC_

#include "exports.h"
#include "ReturnCode.h"
#include "types.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class UAPI_EXPORT Codec
      {
        public:
          typedef enum Type
          {
            /**
             * PCM, 16 bits, 8KHz.
             */
            PCM_16BIT_8K,
            /**
             * PCM, 16 bits, 11KHz.
             */
            PCM_16BIT_11K,
            /**
              * PCM, 16 bits, 22KHz.
              */
            PCM_16BIT_22K,
            /**
             * ULAW, 8 bits, 8KHz
             */
            ULAW_8BIT_8K
          } Type;
          
          /**
           * Returns the sample-rate of the specified codec.
           *
           * @param codec the codec to evaluate
           * @param returnCode the return code
           */
          static UINT16 getSampleRate(Type codec,
                                      ReturnCode::Type& returnCode);
                                      
          /**
           * Returns the bitrate of the specified codec.
           *
           * @param codec the codec to evaluate
           * @param returnCode the return code
           */
          static UINT16 getBitsPerSample(Type codec, ReturnCode::Type& returnCode);
      };
    }
  }
}

#endif
