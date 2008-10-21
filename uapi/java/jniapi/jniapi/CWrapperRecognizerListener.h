/*---------------------------------------------------------------------------*
 *  CWrapperRecognizerListener.h                                             *
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

#ifndef __UAPI__CWRAPPERRECOGNIZERLISTENER
#define __UAPI__CWRAPPERRECOGNIZERLISTENER

#include <jni.h>
#include "ReturnCode.h"
#include "RecognizerListener.h"
#include "NBestRecognitionResult.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace jni
      {
        class CWrapperRecognizerListener: public RecognizerListener
        {
          public:
            CWrapperRecognizerListener(JavaVM* jvm, jobject listener, ReturnCode::Type& returnCode);
            ~CWrapperRecognizerListener();
            
            const char* toString(FailureReason reason);
            void onStarted();
            void onBeginningOfSpeech();
            void onEndOfSpeech();
            void onRecognitionSuccess(RecognitionResultProxy& result);
            void onRecognitionFailure(FailureReason reason);
            void onError(jthrowable exception);
            void onError(ReturnCode::Type returnCode);
            void onStopped();
            void onParametersSetError(const char** keys, const char** values, ARRAY_LIMIT count,
                                      ReturnCode::Type returnCode);
            void onParametersGetError(const char** keys, ARRAY_LIMIT count,
                                      ReturnCode::Type returnCode);
            void onParametersSet(const char** keys, const char** values, ARRAY_LIMIT count);
            void onParametersGet(const char** keys, const char** values, ARRAY_LIMIT count);
            void onAcousticStateReset();
          protected:
            /**
            * Invokes the method with the specified name.
            */
            void invokeMethod(const char* methodName);
          private:
            /**
            * Converts key-value pairs to a hashtable.
            */
            jobject pairsToHashtable(const char** keys, const char** values, ARRAY_LIMIT count);
            /**
            * Converts a list of keys to a vector.
            */
            jobject keysToVector(const char** keys, ARRAY_LIMIT count);
            JavaVM* jvm;
            jobject listener;
        };
      }
    }
  }
}

#endif
