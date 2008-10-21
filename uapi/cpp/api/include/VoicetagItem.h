/*---------------------------------------------------------------------------*
 *  VoicetagItem.h                                                           *
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

#ifndef __UAPI__VOICETAGITEM
#define __UAPI__VOICETAGITEM

#include "exports.h"
#include "types.h"
#include "SlotItem.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class VoicetagItemProxy;
     
      /**
       * Voicetag that may be inserted into an embedded grammar slot.
       */
      class UAPI_EXPORT VoicetagItem: public SlotItem
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
          virtual void getAudio(const INT16** waveform, ARRAY_LIMIT* size,
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
          virtual void setAudio(const INT16* waveform, ARRAY_LIMIT size,
                                ReturnCode::Type& returnCode) = 0;

          /**
           * Returns true if the item is a word.
           */
          virtual bool isWord() const; //is text tag
          
          /**
           * Returns true if the item is a voicetag.
           */
          virtual bool isVoicetag() const;
        protected:
          /**
           * Prevent construction.
           */
          VoicetagItem();
          /**
           * Prevent destruction.
           */
          virtual ~VoicetagItem()
          {}
          
          friend class VoicetagItemProxy;
      };
      
      /*
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, VoicetagItemProxy, SlotItemProxy, VoicetagItem)
    }
  }
}

#endif
