/*---------------------------------------------------------------------------*
 *  Microphone.h                                                             *
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

#ifndef __UAPI__MICROPHONE
#define __UAPI__MICROPHONE

#include "exports.h"
#include "AudioSource.h"
#include "Codec.h"
#include "Singleton.h"
#include "SmartProxy.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class AudioSourceListenerProxy;
      class System;
    }
  }
}


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class MicrophoneProxy;
      /**
       * Records audio.
       */
      class Microphone: public AudioSource, public Singleton
      {
        public:
          /**
           * Returns the microphone.
           *
           * @param returnCode the return code.
           * @return the microphone
           */
          UAPI_EXPORT static MicrophoneProxy getInstance(ReturnCode::Type& returnCode);
          
          /**
           * Sets the recording codec. This must be called before start() is invoked.
           *
           * @param recordingCodec the codec in which the samples will be recorded
           * @param returnCode the return code
           */
          virtual void setCodec(Codec::Type recordingCodec, ReturnCode::Type& returnCode) = 0;
          
          /**
           * Set the microphone listener.
           *
           * @param listener the microphone listener
           * @param returnCode the return code
           */
          virtual void setListener(AudioSourceListenerProxy& listener, ReturnCode::Type& returnCode) = 0;
          
        protected:
          /**
           * Prevent construction.
           */
          Microphone();
          /**
           * Prevent destruction.
           */
          virtual ~Microphone();
          
          /**
           * singleton instance
           */
          static Microphone* instance;
        private:
          /**
           * Prevent assignment.
           */
          Microphone& operator=(Microphone&);
          
          friend class MicrophoneProxy;
      };
      
      /**
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, MicrophoneProxy, AudioSourceProxy, Microphone)
    }
  }
}

#endif
