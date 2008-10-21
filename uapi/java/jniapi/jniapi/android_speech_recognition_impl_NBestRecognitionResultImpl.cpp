/*---------------------------------------------------------------------------*
 *  android_speech_recognition_impl_NBestRecognitionResultImpl.cpp                                 *
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
#include "android_speech_recognition_impl_NBestRecognitionResultImpl.h"
#include "NBestRecognitionResult.h"
#include "VoicetagItem.h"
#include "CWrapperVoicetagListener.h"
#include "JNIHelper.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;

 JNIEXPORT jlong JNICALL Java_android_speech_recognition_impl_NBestRecognitionResultImpl_createVoicetagItemProxy
 (JNIEnv *env, jobject result, jlong nativeObj, jstring VoicetagId, jobject listener)
{
  ReturnCode::Type returnCode = ReturnCode::SUCCESS;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_NBestRecognitionResultImpl_createVoicetagItemProxy");
    
  NBestRecognitionResultProxy* nativeResultPointer = (NBestRecognitionResultProxy*) nativeObj;
      
  if (!nativeResultPointer)
  {
    JNIHelper::throwJavaException(env, result, ReturnCode::INVALID_STATE);
    return 0;
  }
  NBestRecognitionResultProxy& nativeResult = *nativeResultPointer;

  const char* nativeVoicetagId = env->GetStringUTFChars(VoicetagId, 0);
  
  // Create the native listener
  VoicetagItemListenerProxy nativeListener;
  if (listener != 0)
  {
    JavaVM* jvm = JNIHelper::getJVM(env, result);
    if (env->ExceptionCheck())
      return 0;
    CWrapperVoicetagListener* temp = new CWrapperVoicetagListener(jvm, listener, returnCode);
    if (!temp)
    {
      JNIHelper::throwJavaException(env, result, ReturnCode::OUT_OF_MEMORY);
      return 0;
    }
    else if (returnCode)
    {
      JNIHelper::throwJavaException(env, result, returnCode);
      return 0;
    }
    nativeListener = VoicetagItemListenerProxy(temp);
    if (!nativeListener)
    {
      delete temp;
      JNIHelper::throwJavaException(env, result, ReturnCode::OUT_OF_MEMORY);
      return 0;
    }
  }

  VoicetagItemProxy nativeVoicetag = nativeResult->createVoicetagItem(nativeVoicetagId,nativeListener,returnCode);

  env->ReleaseStringUTFChars(VoicetagId, nativeVoicetagId);

  if (returnCode != ReturnCode::SUCCESS)
  {
    JNIHelper::throwJavaException(env, result, returnCode);
    return 0;
  }
  VoicetagItemProxy* resultVoicetag = new VoicetagItemProxy(nativeVoicetag);

  if (!resultVoicetag || !*resultVoicetag)
  {
    delete resultVoicetag;
    returnCode = ReturnCode::OUT_OF_MEMORY;
    JNIHelper::throwJavaException(env, result, returnCode);
    return 0;
  }
  return (jlong) resultVoicetag;
}

JNIEXPORT jint JNICALL Java_android_speech_recognition_impl_NBestRecognitionResultImpl_getSizeProxy(JNIEnv* env,
    jobject result, jlong nativeObj)
{
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_NBestRecognitionResultImpl_getSizeProxy");
    
  NBestRecognitionResultProxy* nativeResultPointer = (NBestRecognitionResultProxy*) nativeObj;
      
  if (!nativeResultPointer)
  {
    JNIHelper::throwJavaException(env, result, ReturnCode::INVALID_STATE);
    return 0;
  }
  NBestRecognitionResultProxy& nativeResult = *nativeResultPointer;
  return (jint) nativeResult->getSize();
}


JNIEXPORT jlong JNICALL Java_android_speech_recognition_impl_NBestRecognitionResultImpl_getEntryProxy(JNIEnv* env,
    jobject recognitionResult,
    jlong nativeObj,
    jint index)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_NBestRecognitionResultImpl_getEntryProxy");
    
  NBestRecognitionResultProxy* nativeResultPointer = (NBestRecognitionResultProxy*) nativeObj;
      
  if (!nativeResultPointer)
  {
    JNIHelper::throwJavaException(env, recognitionResult, ReturnCode::INVALID_STATE);
    return 0;
  }
  NBestRecognitionResultProxy& nativeResult = *nativeResultPointer;
  
  NBestRecognitionResult::EntryProxy nativeEntry = nativeResult->getEntry(index, returnCode);
  if (returnCode)
  {
    JNIHelper::throwJavaException(env, recognitionResult, returnCode);
    return 0;
  }
  if (!nativeEntry)
  {
    // getEntry() returns null if all GrammarConfiguration.grammarToMeaning() returns null
    return 0;
  }
  NBestRecognitionResult::EntryProxy* result = new NBestRecognitionResult::EntryProxy(nativeEntry);
  if (!result || !*result)
  {
    delete result;
    returnCode = ReturnCode::OUT_OF_MEMORY;
    JNIHelper::throwJavaException(env, recognitionResult, returnCode);
    return 0;
  }
  return (jlong) result;
}
