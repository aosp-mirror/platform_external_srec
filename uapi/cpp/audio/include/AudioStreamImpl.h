/*---------------------------------------------------------------------------*
 *  AudioStreamImpl.h                                                        *
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

#ifndef __UAPI__AUDIOIMPL
#define __UAPI__AUDIOIMPL

#include "exports.h"
#include "AudioStream.h"
#include "Codec.h"
#include "SmartProxy.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class SinglyLinkedNode;
        class AudioQueue;
        class AudioBuffer;
      }
      namespace impl
      {
        class AudioStreamImplProxy;
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
      namespace impl
      {
        /**
         * Implementation of the collection of audio samples.
         */
        class UAPI_EXPORT AudioStreamImpl: public AudioStream
        {
          public:
            /**
             * Creates an instance of this class.
             *
             * @param queue the shared audio queue used by this stream.
             * @param returnCode the returnCode
             */
            static AudioStreamImplProxy create(utilities::AudioQueue* queue, ReturnCode::Type& returnCode);
            
            /**
             * Returns the next audio buffer. If read returns 0 and returnCode is set
             * to PENDING_DATA it means that read() should be called later.
             *
             * @param returnCode END_OF_STREAM if done. PENDING_DATA if more data will
             * get added later.
             * @return the next audio buffer, or 0 if no data is available. In the
             * latter case, the return code indicates whether more data will be
             * forthcoming.
             */
            utilities::AudioBuffer* read(ReturnCode::Type& returnCode);
            
            /**
             * Releases the buffer that was acquired by calling read.
             * @see read.
             * @param audioBuffer the audio buffer to release.
             * @param returnCode the return code.
             */
            void release(utilities::AudioBuffer* audioBuffer, ReturnCode::Type& returnCode);
            
            /**
             * Call this function to know the audio format of the buffers returned by
             * read().
             */
            Codec::Type getCodec();
            
            /**
             * An AudioStreamImpl can only be used by one consumer at a time. When the
             * Recongizer, the FileWritter or the DeviceSpeaker are asked to start with
             * an audio, they must check if this audio already Locked. If it's not
             * locked, they must call this function to lock it.
             *
             * @param returnCode AUDIO_ALREADY_IN_USE if the audio is already in use by another component.
             * END_OF_STREAM if the end of the audio stream has been reached.
             */
            void lock(ReturnCode::Type& returnCode);
            
            /**
             * @see lock
             * call this function to unlock this audio object.
             */
            void unlock();
            
            
            
          private:
          
            /**
             * consturctor
             *
             * @param queue the shared audio queue used by this stream.
             * @param returnCode the returnCode
             */
            AudioStreamImpl(utilities::AudioQueue* queue, ReturnCode::Type& returnCode);
            
            /**
             * destructor
             */
            virtual ~AudioStreamImpl();
            
            /**
             * The underlying audio data shared amongst all sinks.
             */
            utilities::AudioQueue* audioQueue;
            /**
             * Current read position from the AudioQueue.
             */
            utilities::SinglyLinkedNode* readPosition;
            /**
             * Flag that specifies if the AudioStreamImpl was locked by a Recognizer,
             * DeviceSpeaker or a FileWritter.
             */
            bool isLocked;
            
            friend class AudioStreamImplProxy;
        };
        
        /*
         * @see android::speech::recognition::SmartProxy
         */
        DECLARE_SMARTPROXY(UAPI_EXPORT, AudioStreamImplProxy, AudioStreamProxy, AudioStreamImpl)
      }
    }
  }
}

#endif
