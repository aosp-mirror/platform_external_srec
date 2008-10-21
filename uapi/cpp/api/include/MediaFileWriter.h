/*---------------------------------------------------------------------------*
 *  MediaFileWriter.h                                                        *
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

#ifndef __UAPI__MEDIAFILEWRITER_
#define __UAPI__MEDIAFILEWRITER_

#include "exports.h"
#include "ReturnCode.h"
#include "SmartProxy.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class MediaFileWriterListenerProxy;
      class AudioStreamProxy;
    }
  }
}


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class MediaFileWriterProxy;
      /**
       * Writes audio to a file.
       */
      class MediaFileWriter
      {
        public:
          /**
           * Creates an instace of this class.
           *
           * @param listener listens for MediaFileWriter events
           * @param logger the logger
           * @param returnCode the return code
           */
          UAPI_EXPORT static MediaFileWriterProxy create(MediaFileWriterListenerProxy& listener,
              ReturnCode::Type& returnCode);
              
          /**
           * Saves audio to a file.
           *
           * @param audio the audio to save
           * @param filename the path to
           * @param returnCode ILLEGAL_ARGUMENT if audio object is invalid.
           * AUDIO_ALREADY_IN_USE if the audio is already in use by another component.
           * END_OF_STREAM if the end of the audio stream has been reached.
           */
          virtual void save(AudioStreamProxy& audio, const char* path, ReturnCode::Type& returnCode) = 0;
        protected:
          /**
           * Prevent construction.
           */
          MediaFileWriter();
          /**
           * Prevent destruction.
           */
          virtual ~MediaFileWriter();
          
          friend class MediaFileWriterProxy;
      };
      
      /**
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, MediaFileWriterProxy, SmartProxy, MediaFileWriter)
    }
  }
}

#endif
