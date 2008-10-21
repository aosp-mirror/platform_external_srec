/*---------------------------------------------------------------------------*
 *  android_speech_recognition_impl_EntryImpl.cpp                                                  *
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
 * android_speech_recognition_impl_EntryImpl.cpp
 */
#include "jniapi.h"
#include <jni.h>
#include "android_speech_recognition_impl_EntryImpl.h"
#include "NBestRecognitionResult.h"
#include "JNIHelper.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;


JNIEXPORT void JNICALL Java_android_speech_recognition_impl_EntryImpl_deleteNativeObject
(JNIEnv* ,
 jobject ,
 jlong nativeObj)
{
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EntryImpl_deleteNativeObject");
  delete (NBestRecognitionResult::EntryProxy*) nativeObj;
}

JNIEXPORT jstring JNICALL Java_android_speech_recognition_impl_EntryImpl_getLiteralMeaningProxy
(JNIEnv* env,
 jobject entry,
 jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EntryImpl_getLiteralMeaningProxy");
  
  NBestRecognitionResult::EntryProxy* nativeEntryPointer = (NBestRecognitionResult::EntryProxy*) nativeObj;
  if (!nativeEntryPointer)
  {
    JNIHelper::throwJavaException(env, entry, ReturnCode::INVALID_STATE);
    return 0;
  }
  NBestRecognitionResult::EntryProxy& nativeEntry = *nativeEntryPointer;
  
  const char* nativeLiteralMeaning = nativeEntry->getLiteralMeaning(returnCode);
  
  if (returnCode)
  {
    JNIHelper::throwJavaException(env, entry, returnCode);
    return 0;
  }
  return env->NewStringUTF(nativeLiteralMeaning);
}


JNIEXPORT jstring JNICALL Java_android_speech_recognition_impl_EntryImpl_getSemanticMeaningProxy
(JNIEnv* env,
 jobject entry,
 jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EntryImpl_getSemanticMeaningProxy");
  
  NBestRecognitionResult::EntryProxy* nativeEntryPointer = (NBestRecognitionResult::EntryProxy*) nativeObj;
      
  if (!nativeEntryPointer)
  {
    JNIHelper::throwJavaException(env, entry, ReturnCode::INVALID_STATE);
    return 0;
  }
  NBestRecognitionResult::EntryProxy& nativeEntry = *nativeEntryPointer;
  
  const char * nativeSemanticMeaning = nativeEntry->getSemanticMeaning(returnCode);
  if (returnCode)
  {
    JNIHelper::throwJavaException(env, entry, returnCode);
    return 0;
  }
  return env->NewStringUTF(nativeSemanticMeaning);
}



JNIEXPORT jbyte JNICALL Java_android_speech_recognition_impl_EntryImpl_getConfidenceScoreProxy
(JNIEnv* env,
 jobject entry,
 jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EntryImpl_getConfidenceScoreProxy");
  
  NBestRecognitionResult::EntryProxy* nativeEntryPointer = (NBestRecognitionResult::EntryProxy*)nativeObj;
      
  if (!nativeEntryPointer)
  {
    JNIHelper::throwJavaException(env, entry, ReturnCode::INVALID_STATE);
    return 0;
  }
  NBestRecognitionResult::EntryProxy& nativeEntry = *nativeEntryPointer;
  
  android::speech::recognition::UINT8 nativeConfidenceScore = nativeEntry->getConfidenceScore(returnCode);
  if (returnCode)
  {
    JNIHelper::throwJavaException(env, entry, returnCode);
    return 0;
  }
  return nativeConfidenceScore;
}

JNIEXPORT jstring JNICALL Java_android_speech_recognition_impl_EntryImpl_getProxy
  (JNIEnv * env, jobject entry, jlong nativeObj, jstring key)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EntryImpl_getProxy");

  NBestRecognitionResult::EntryProxy* nativeEntryPointer = (NBestRecognitionResult::EntryProxy*)nativeObj;
      
  if (!nativeEntryPointer)
  {
    JNIHelper::throwJavaException(env, entry, ReturnCode::INVALID_STATE);
    return 0;
  }
  NBestRecognitionResult::EntryProxy& nativeEntry = *nativeEntryPointer;
  
  const char* nativeKey = env->GetStringUTFChars(key, 0);
  const char* nativeValue = nativeEntry->getValue(nativeKey, returnCode);
  env->ReleaseStringUTFChars(key, nativeKey);
  
  if (returnCode != ReturnCode::SUCCESS)
  {
    JNIHelper::throwJavaException(env, entry, returnCode);
    return 0;
  }
  return env->NewStringUTF(nativeValue);
}

JNIEXPORT jobjectArray JNICALL Java_android_speech_recognition_impl_EntryImpl_keysProxy
  (JNIEnv * env, jobject entry, jlong nativeObj)
{
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EntryImpl_keysProxy");

  NBestRecognitionResult::EntryProxy* nativeEntryPointer = (NBestRecognitionResult::EntryProxy*)nativeObj;
      
  if (!nativeEntryPointer)
  {
    JNIHelper::throwJavaException(env, entry, ReturnCode::INVALID_STATE);
    return 0;
  }
  NBestRecognitionResult::EntryProxy& nativeEntry = *nativeEntryPointer;

  const char* const* nativeKeys = nativeEntry->getKeys();
  ARRAY_LIMIT length = nativeEntry->getKeyCount();
  if( nativeKeys == 0 || length == 0 )
  {
    UAPI_INFO(fn, "No entries available\n");
    return 0;
  }

  jclass stringClass = env->FindClass("java/lang/String");
  jobjectArray result = env->NewObjectArray(length, stringClass, 0);
  env->DeleteLocalRef(stringClass);
  if( result == 0 )
  {
    UAPI_ERROR(fn, "Failed to create NewObjectArray\n");
    return 0;
  }
  
  for (ARRAY_LIMIT i = 0; i < length; ++i)
  {
    jstring value = env->NewStringUTF(nativeKeys[i]);
    env->SetObjectArrayElement(result, i, value);
    env->DeleteLocalRef(value);
  }
  return result;
}
