/*---------------------------------------------------------------------------*
 *  RecognizerListener.h                                                     *
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

#ifndef __UAPI__RECOGNIZERLISTENER
#define __UAPI__RECOGNIZERLISTENER

#include "exports.h"
#include "ReturnCode.h"
#include "ParametersListener.h"
#include "SmartProxy.h"
#include "RecognitionResult.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class Recognizer;
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
       * Listens for recognizer events.
       */
      class RecognizerListener : public ParametersListener
      {
        public:
          enum FailureReason
          {
            /**
             * The audio did not generate any results.
             */
            NO_MATCH,
            /**
             * Beginning of speech occured too soon.
             */
            SPOKE_TOO_SOON,
            /**
             * A timeout occured before the beginning of speech.
             */
            BEGINNING_OF_SPEECH_TIMEOUT,
            /**
             * A timeout occured before the recognition could complete.
             */
            RECOGNITION_TIMEOUT,
            /**
             * The recognizer encountered more audio than was acceptable according to its configuration.
             */
            TOO_MUCH_SPEECH,
            /**
             * Posting the recognition results to the 3rd party app server failed.
             */
            RECOGNITION_3RD_PARTY_ERROR,
            /**
             * The speech server is unavailable.
             */
            SPEECH_SERVER_UNAVAILABLE,
            /**
             * Unknown failure code.
             */
            UNKNOWN
          };
          
          /**
           * Returns the textual message associated with the error type.
           *
           * @param reason the failure reason
           */
          UAPI_EXPORT const char* toString(FailureReason reason);
          
          /**
            * Invoked after recognition begins.
           */
          virtual void onStarted() = 0;
          
          /**
           * Invoked when the recognizer detects the beginning of speech.
           */
          virtual void onBeginningOfSpeech() = 0;
          
          /**
           * Invoked when the recognizer detects the end of speech.
           */
          virtual void onEndOfSpeech() = 0;
          
          /**
           * Invoked when the recognizer acoustic state is reset.
           *
           * @see EmbeddedRecognizer::resetAcousticState()
           */
          virtual void onAcousticStateReset() = 0;
          
          /**
           * Invoked when a recognition success occurs.
           *
           * @param result the recognition result. The result object can not be
           * used outside of the scope of the onRecognitionSuccess() callback method.
           * To be able to do so, copy it's contents to an user-defined object.
           * An example of this object could be a vector of string arrays; where the
           * vector represents a list of recognition result entries and each entry
           * is an array of strings to hold the entry's values (the semantic
           * meaning, confidence score and literal meaning).
           */
          virtual void onRecognitionSuccess(RecognitionResultProxy& result) = 0;
          
          /**
           * Invoked when a recognition failure occurs.
           *
           * @param reason the failure reason
           * @see toString(FailureReason)
           */
          virtual void onRecognitionFailure(FailureReason reason) = 0;
          
          /**
           * Invoked when an unexpected error occurs. This is normally followed by
           * onStopped() if the component shuts down successfully.
           *
           * @param returnCode the return code
           */
          virtual void onError(ReturnCode::Type returnCode) = 0;
          
          /**
           * Invoked when the recognizer shuts down (either normally or due to an error).
           */
          virtual void onStopped() = 0;
        protected:
          /**
           * Prevent construction.
           */
          UAPI_EXPORT RecognizerListener();
          /**
           * Prevent destruction.
           */
          UAPI_EXPORT virtual ~RecognizerListener();
          
          friend class RecognizerListenerProxy;
      };
      
      /*
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, RecognizerListenerProxy, android::speech::recognition::SmartProxy,
                         RecognizerListener)
    }
  }
}

#endif
