/*---------------------------------------------------------------------------*
 *  android_speech_recognition_impl_EmbeddedGrammarImpl.cpp                                        *
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

/*
 */

#include "jniapi.h"
#include "android_speech_recognition_impl_EmbeddedGrammarImpl.h"
#include "JNIHelper.h"
#include "EmbeddedGrammar.h"


using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_EmbeddedGrammarImpl_compileAllSlotsProxy(JNIEnv* env,
    jobject grammar, jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EmbeddedGrammarImpl_compileAllSlotsProxy");
  
  EmbeddedGrammarProxy* nativeGrammarPointer = (EmbeddedGrammarProxy*) nativeObj;

  if (!nativeGrammarPointer)
  {
    JNIHelper::throwJavaException(env, grammar, ReturnCode::INVALID_STATE);
    return;
  }
  EmbeddedGrammarProxy& nativeGrammar = *nativeGrammarPointer;
  nativeGrammar->compileAllSlots(returnCode);
  
  if (returnCode)
    JNIHelper::throwJavaException(env, grammar, returnCode);
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_EmbeddedGrammarImpl_resetAllSlotsProxy(JNIEnv* env,
    jobject grammar, jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EmbeddedGrammarImpl_resetAllSlotsProxy");
  
  EmbeddedGrammarProxy* nativeGrammarPointer = (EmbeddedGrammarProxy*) nativeObj;

  if (!nativeGrammarPointer)
  {
    JNIHelper::throwJavaException(env, grammar, ReturnCode::INVALID_STATE);
    return;
  }
  EmbeddedGrammarProxy& nativeGrammar = *nativeGrammarPointer;
  nativeGrammar->resetAllSlots(returnCode);
  
  if (returnCode)
    JNIHelper::throwJavaException(env, grammar, returnCode);
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_EmbeddedGrammarImpl_saveProxy(JNIEnv* env,
    jobject grammar,
    jlong nativeObj,
    jstring url)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EmbeddedGrammarImpl_saveProxy");
  
  EmbeddedGrammarProxy* nativeGrammarPointer = (EmbeddedGrammarProxy*) nativeObj;

  if (!nativeGrammarPointer)
  {
    JNIHelper::throwJavaException(env, grammar, ReturnCode::INVALID_STATE);
    return;
  }
  EmbeddedGrammarProxy& nativeGrammar = *nativeGrammarPointer;
  
  const char* nativeURL = env->GetStringUTFChars(url, 0);
  nativeGrammar->save(nativeURL, returnCode);
  env->ReleaseStringUTFChars(url, nativeURL);
  
  if (returnCode != ReturnCode::SUCCESS)
  {
    JNIHelper::throwJavaException(env, grammar, returnCode);
  }
}
