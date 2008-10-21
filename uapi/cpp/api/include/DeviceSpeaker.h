/*---------------------------------------------------------------------------*
 *  DeviceSpeaker.h                                                          *
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

#ifndef __UAPI__DEVICESPEAKER
#define __UAPI__DEVICESPEAKER

#include "exports.h"
#include "ReturnCode.h"
#include "Codec.h"
#include "Singleton.h"
#include "SmartProxy.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class DeviceSpeakerListenerProxy;
      class System;
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
      class DeviceSpeakerProxy;
      /**
       * DeviceSpeaker used to playback audio samples.
       */
      class DeviceSpeaker : public Singleton
      {
        public:
          /**
           * Returns the device speaker instance.
           *
           * @param returnCode the return code.
           * @return an instance of a DeviceSpeaker class.
           */
          UAPI_EXPORT static DeviceSpeakerProxy getInstance(ReturnCode::Type& returnCode);
          
          /**
           * Start audio playback.
           *
           * @param audio the audio to play
           * @param returnCode INVALID_STATE if the component is already stated.
           * ILLEGAL_ARGUMENT if the audio object is invalid.
           * AUDIO_ALREADY_IN_USE if the audio is already in use by another component.
           * END_OF_STREAM if the end of the audio stream has been reached.
           */
          virtual void start(AudioStreamProxy& audio, ReturnCode::Type& returnCode) = 0;
          
          /**
           * Stops audio playback.
           *
           * @return SUCCESS if stop in progress
           * @param returnCode the return code
           */
          virtual void stop(ReturnCode::Type& returnCode) = 0;
          
          /**
           * Sets the playback codec. This must be called before start is called.
           *
           * @param playbackCodec the codec to use for the playback operation.
           * @param returnCode the return code.
           */
          virtual void setCodec(Codec::Type playbackCodec, ReturnCode::Type& returnCode) = 0;
          
          /**
           * Sets the microphone listener.
           *
           * @param listener the device speaker listener.
           * @param returnCode the return code.
           */
          virtual void setListener(DeviceSpeakerListenerProxy& listener, ReturnCode::Type& returnCode) = 0;
          
        protected:
          /**
           * Prevent construction.
           */
          DeviceSpeaker();
          /**
           * Prevent destruction.
           */
          virtual ~DeviceSpeaker();
          
          /**
           * Singleton instance.
           */
          static DeviceSpeaker* instance;
          
          friend class DeviceSpeakerProxy;
      };
      
      /*
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, DeviceSpeakerProxy, SmartProxy, DeviceSpeaker)
    }
  }
}

#endif
