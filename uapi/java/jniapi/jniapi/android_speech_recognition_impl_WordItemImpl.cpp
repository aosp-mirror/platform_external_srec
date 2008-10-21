/*---------------------------------------------------------------------------*
 *  android_speech_recognition_impl_WordItemImpl.cpp                                               *
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
 * android_speech_recognition_WordItemImpl.cpp
 */
#include <assert.h>
#include "jniapi.h"
#include "android_speech_recognition_impl_WordItemImpl.h"
#include "WordItem.h"
#include "JNIHelper.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;


JNIEXPORT void JNICALL Java_android_speech_recognition_impl_WordItemImpl_deleteNativeObject(JNIEnv*, jobject,
    jlong nativeObj)
{
  delete(WordItemProxy*) nativeObj;
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_WordItemImpl_initNativeObject(JNIEnv* env,
    jobject wordItem,
    jstring word,
    jobjectArray pronunciations)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_WordItemImpl_initNativeObject");
  
  const char* nativeWord = env->GetStringUTFChars(word, 0);
  
  jsize nativePronunciationCount = env->GetArrayLength(pronunciations);
  
  WordItemProxy temp;
  const char** nativePronunciations = 0;
  if (nativePronunciationCount > 0)
  {
    nativePronunciations = new const char*[nativePronunciationCount];
    for (jsize i = 0; i < nativePronunciationCount; ++i)
    {
      jstring pronunciation = (jstring) env->GetObjectArrayElement(pronunciations, i);
      nativePronunciations[i] = env->GetStringUTFChars(pronunciation, 0);
    }
    temp = WordItem::create(nativeWord, nativePronunciations, nativePronunciationCount, returnCode);
  }
  else
    temp = WordItem::create(nativeWord, NULL, 0, returnCode);
  if (returnCode)
  {
    JNIHelper::throwJavaException(env, wordItem, returnCode);
    return;
  }
  WordItemProxy* nativeWordItem = new WordItemProxy(temp);
  if (!nativeWordItem)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    JNIHelper::throwJavaException(env, wordItem, returnCode);
    return;
  }
  
  env->ReleaseStringUTFChars(word, nativeWord);
  if (nativePronunciationCount > 0)
  {
    for (jsize i = 0; i < nativePronunciationCount; ++i)
    {
      jstring pronunciation = (jstring) env->GetObjectArrayElement(pronunciations, i);
      env->ReleaseStringUTFChars(pronunciation, nativePronunciations[i]);
    }
    delete[] nativePronunciations;
  }
  
  if (returnCode)
  {
    JNIHelper::throwJavaException(env, wordItem, returnCode);
    return;
  }
  
  // Store a reference to nativeWordItem in WordItem.nativeObject
  jclass wordItemClass = env->GetObjectClass(wordItem);
  jfieldID nativeObjectField = env->GetFieldID(wordItemClass, "nativeObject", "J");
  assert(nativeObjectField != 0);
  env->DeleteLocalRef(wordItemClass);
  env->SetLongField(wordItem, nativeObjectField, (jlong) nativeWordItem);
}

