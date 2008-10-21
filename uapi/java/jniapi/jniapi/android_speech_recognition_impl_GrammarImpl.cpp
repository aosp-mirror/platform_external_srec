/*---------------------------------------------------------------------------*
 *  android_speech_recognition_impl_GrammarImpl.cpp                                                *
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
#include "android_speech_recognition_impl_GrammarImpl.h"
#include "JNIHelper.h"
#include "Grammar.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;


JNIEXPORT void JNICALL Java_android_speech_recognition_impl_GrammarImpl_deleteNativeObject(JNIEnv*,
    jobject,
    jlong nativeObj)
{
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_GrammarImpl_deleteNativeObject");
  delete(GrammarProxy*) nativeObj;
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_GrammarImpl_loadProxy
(JNIEnv* env,
 jobject grammar,
 jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_GrammarImpl_loadProxy");
  
  GrammarProxy* nativeGrammarPointer = (GrammarProxy*) nativeObj;
  if (!nativeGrammarPointer)
  {
    JNIHelper::throwJavaException(env, grammar, ReturnCode::INVALID_STATE);
    return;
  }
  GrammarProxy& nativeGrammar = *nativeGrammarPointer;
  nativeGrammar->load(returnCode);
  
  if (returnCode)
    JNIHelper::throwJavaException(env, grammar, returnCode);
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_GrammarImpl_unloadProxy(JNIEnv* env,
    jobject grammar,jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_GrammarImpl_unloadProxy");
  
  GrammarProxy* nativeGrammarPointer = (GrammarProxy*) nativeObj;
  if (!nativeGrammarPointer)
  {
    JNIHelper::throwJavaException(env, grammar, ReturnCode::INVALID_STATE);
    return;
  }
  GrammarProxy& nativeGrammar = *nativeGrammarPointer;
  nativeGrammar->unload(returnCode);
  
  if (returnCode)
    JNIHelper::throwJavaException(env, grammar, returnCode);
}
