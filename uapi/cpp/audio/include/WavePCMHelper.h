/*---------------------------------------------------------------------------*
 *  WavePCMHelper.h                                                          *
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

#ifndef __UAPI_WAVE_PCM_HELPER__H_
#define __UAPI_WAVE_PCM_HELPER__H_
#include "types.h"
#include "ReturnCode.h"
#include "Codec.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class File;
      }
    }
  }
}

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class WavePCMHelper
        {
        public:
          /**
          * Add a RIFF wave header to a file.
          *
          * @param file pointer to in which we want to add the RIFF header.
          * @param sampleRate sample rate at which the audio samples were
          * collected, e.g. 11025, 8000.
          * @param numChannels 1 for mono, 2 for stereo.
          * @param bitsPerSample Number of bits per sample, could be 16 or 8.
          * @param numBytes Number of bytes in the audio payload.
          *
          */
          static void writeWavHeader(File * file, INT32 sampleRate, INT16 numChannels, 
              INT16 bitsPerSample, INT32 numBytes, ReturnCode::Type & returnCode);

          /**
          * Update the file length of an existing RIFF wave file.
          *
          * @param file pointer to in which we want to add the RIFF header.
          * @param numBytes Number of bytes in the audio payload.
          */
          static void updateFileLength(File * file, INT32 numBytes, ReturnCode::Type & returnCode);


          /**
           * get the codec from a wave file.
           *
           * @param file pointer to a file that contains NIST Sphere or RIFF data.
           * @param out_codec the codec contained in that file
           * @param out_headerOffset the header size of that file.
           */
          static void getWavFileInfo( File * file, Codec::Type & out_codec, 
                                      UINT16 & out_headerOffset, ReturnCode::Type & returnCode );

        private:
          /**
           * get the information when the file is RIFF
           *
           * @param file pointer to a file that contains RIFF data.
           * @param out_codec the codec contained in that file
           */
          static void getRIFFfileInfo( File * file, Codec::Type & out_codec, 
                                              ReturnCode::Type & returnCode );
          /**
           * get the information when the file is Sphere NIST
           *
           * @param file pointer to a file that contains NIST Sphere.
           * @param out_codec the codec contained in that file
           */
          static void getNISTfileInfo( File * file, Codec::Type & out_codec, 
                                    ReturnCode::Type & returnCode );
        };
      }
    }
  }
}
#endif

