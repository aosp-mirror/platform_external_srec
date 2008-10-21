/*---------------------------------------------------------------------------*
 *  AudioSource.h                                                            *
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

#ifndef __UAPI__AUDIOSOURCE
#define __UAPI__AUDIOSOURCE

#include "exports.h"
#include "ReturnCode.h"
#include "SmartProxy.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
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
      /**
       * A collection of audio data.
       */
      class AudioSource
      {
        public:
          virtual ~AudioSource()
          {}
          /**
           * Returns an object that contains audio samples.
           *
           * @param returnCode the return code
           * @return a pointer to an Audio object.
           * @see AudioStream
           */
          virtual AudioStreamProxy createAudio(ReturnCode::Type& returnCode) = 0;
          
          /**
           * Starts collecting audio samples.
           *
           * @param returnCode the return code
           */
          virtual void start(ReturnCode::Type& returnCode) = 0;
          
          /**
           * Stops collecting audio samples.
           *
           * @param returnCode the return code
           */
          virtual void stop(ReturnCode::Type& returnCode) = 0;
        private:
          /**
          * Prevent assignment.
          */
          AudioSource& operator=(AudioSource&);
      };
   /*
      * @see android::speech::recognition::SmartProxy
      */
      DECLARE_SMARTPROXY(UAPI_EXPORT, AudioSourceProxy, android::speech::recognition::SmartProxy, AudioSource)

    }
  }
}

#endif
