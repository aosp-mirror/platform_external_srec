/*---------------------------------------------------------------------------*
 *  MediaFileReader.h                                                        *
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

#ifndef __UAPI__MEDIAFILEREADER_
#define __UAPI__MEDIAFILEREADER_

#include "exports.h"
#include "AudioSource.h"
#include "types.h"
#include "Codec.h"
#include "SmartProxy.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class AudioSourceListenerProxy;
      class MediaFileReaderProxy;
    }
  }
}


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      /**
      * Reads audio from a file.
      */
      class MediaFileReader: public AudioSource
      {
        public:
          enum ReadingMode
          {
            /**
            * Reads the file at the rate real-time audio would be spoken.
            */
            REAL_TIME,
            
            /**
            * Reads the entire file at once.
            */
            ALL_AT_ONCE
          };
          
          /**
          * Creates an instance of this class.
          *
          * @param pszFileName the name of the file in which we will read the
          * audio samples. Note: The file MUST be of type Microsoft WAVE RIFF
          * format (PCM 16 bits 8000 Hz or PCM 16 bits 11025 Hz).
          * @param listener listens for MediaFileReader events
          * @param logger the logger
          * @param returnCode the return code
          */
          UAPI_EXPORT static MediaFileReaderProxy create(const char* pszFileName,
              AudioSourceListenerProxy& listener,
              ReturnCode::Type& returnCode);
              
          /**
          * Set the reading mode
          *
          * @param mode the reading mode
          * @param returnCode the return code
          */
          virtual void setReadingMode(ReadingMode mode, ReturnCode::Type& returnCode) = 0;
        protected:
          /**
          * Prevent destruction.
          */
          MediaFileReader();
          /**
          * Prevent destruction.
          */
          virtual ~MediaFileReader();
          
          friend class MediaFileReaderProxy;
      };
      
      /*
      * @see android::speech::recognition::SmartProxy
      */
      DECLARE_SMARTPROXY(UAPI_EXPORT, MediaFileReaderProxy, AudioSourceProxy, MediaFileReader)
    }
  }
}

#endif
