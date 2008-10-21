/*---------------------------------------------------------------------------*
 *  VoicetagItemImpl.h                                                       *
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

#ifndef __UAPI__VOICETAGITEMIMPL
#define __UAPI__VOICETAGITEMIMPL

#include "exports.h"
#include "ReturnCode.h"
#include "VoicetagItem.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace impl
      {
        class VoicetagItemImplProxy;
        /**
         * Voicetag that may be inserted into an embedded grammar slot.
         */
        class VoicetagItemImpl: public VoicetagItem
        {
          public:

            /**
             * (Optional operation) Returns the audio used to construct the Voicetag. The
             * audio is in PCM format and is start-pointed and end-pointed. The audio is
             * only generated if the enableGetWaveform recognition parameter is set
             * prior to recognition.
             *
             * @param waveform the read-only endpointed waveform
             * @param size the size of the waveform in bytes
             * @param returnCode the return code
             * @see RecognizerParameters.enableGetWaveform
             */
            UAPI_EXPORT virtual void getAudio(const INT16** waveform,
                                  ARRAY_LIMIT* size,
                                  ReturnCode::Type& returnCode) const = 0;
                                  
            /**
             * (Optional operation) Sets the audio used to construct the Voicetag. The
             * audio is in PCM format and is start-pointed and end-pointed. The audio is
             * only generated if the enableGetWaveform recognition parameter is set
             * prior to recognition.
             *
             * @param waveform the endpointed waveform
             * @param size the size of the waveform in bytes
             * @param returnCode the return code
             * @see RecognizerParameters.enableGetWaveform
             */
            UAPI_EXPORT virtual void setAudio(const android::speech::recognition::INT16* waveform,
                                  ARRAY_LIMIT size,
                                  ReturnCode::Type& returnCode) = 0;
          protected:
            /**
             * Creates a new VoicetagItemImpl.
             *
             * @param waveform the audio used to generate the voicetag
             * @param size the size of the waveform in bytes
             */
            UAPI_EXPORT VoicetagItemImpl(const INT16* waveform,
                             ARRAY_LIMIT size);
            /**
             * Prevent destruction.
             */
            UAPI_EXPORT virtual ~VoicetagItemImpl();
            
            const android::speech::recognition::INT16* waveform;
            android::speech::recognition::ARRAY_LIMIT waveformSize;
          private:
            /**
             * Prevent assignment.
             */
            VoicetagItemImpl& operator=(VoicetagItemImpl& other);
            
            friend class VoicetagItemImplProxy;
        };
        
        /*
         * @see android::speech::recognition::SmartProxy
         */
        DECLARE_SMARTPROXY(UAPI_EXPORT, VoicetagItemImplProxy, VoicetagItemProxy, VoicetagItemImpl)
      }
    }
  }
}

#endif
