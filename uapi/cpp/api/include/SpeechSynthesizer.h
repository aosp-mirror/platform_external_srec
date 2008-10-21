/*---------------------------------------------------------------------------*
 *  SpeechSynthesizer.h                                                      *
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

#ifndef __UAPI__SPEECH_SYNTHESIZER__
#define __UAPI__SPEECH_SYNTHESIZER__

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
      class SpeechSynthesizerProxy;
      class SpeechSynthesizerDataProxy;
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
       * This resource processes text markup provided by the client and
       * generates a stream of synthesized speech in real-time
       */
      class SpeechSynthesizer: public AudioSource
      {
        public:

          enum SpeechType
          {
            /**
             * The speech to synthesize is plain text
             */
            TEXT,
            /**
             * The speech to synthesize is located at this URI.
             */
            URI,
            /**
             * The speech to synthesize is SSML (markup language).
             * @see http://www.w3.org/TR/speech-synthesis/ 
             */
            SSML
          };
         
          /**
           * Sets the synthesizer listener.
           *
           * @param listener synthesizer listener
           * @param returnCode returns SUCCESS unless a fatal error occurs
           */
          virtual void setListener(AudioSourceListenerProxy& listener,
                                   ReturnCode::Type& returnCode) = 0;

         /**
           * Sets the synthesized codec. This must be called before start() is invoked.
           *
           * @param synthesizeCodec the codec in which the samples will be synthesized
           * @param returnCode the return code
           */
          virtual void setCodec(Codec::Type synthesizeCodec, ReturnCode::Type& returnCode) = 0;

          /**
           * Add speech to systhesize on the queue. All the items in the queue
           * will be systhesized once "start()" is called.
           *
           * @param speechType the speech input type
           * @param value the content of the speechType
           * @param language language contained in the "value" argument. Note
           * that this only applies to TEXT type. If the type is TEXT is this
           * parameter is set to NULL, the default value of "en-US" will be
           * used. For other types, simply set this to NULL.
           * @param returnCode ILLEGAL_ARGUMENT if value is null
           */
          virtual void queue( SpeechSynthesizer::SpeechType speechType, 
                              const char* value, const char * language, 
                              ReturnCode::Type& returnCode ) = 0;

        protected:
          /**
          * Prevent destruction.
          */
          UAPI_EXPORT SpeechSynthesizer();
          /**
          * Prevent destruction.
          */
          UAPI_EXPORT virtual ~SpeechSynthesizer();
          
          friend class SpeechSynthesizerProxy;
      };
      
      /*
      * @see android::speech::recognition::SmartProxy
      */
      DECLARE_SMARTPROXY(UAPI_EXPORT, SpeechSynthesizerProxy, AudioSourceProxy, SpeechSynthesizer)


    }
  }
}

#endif
