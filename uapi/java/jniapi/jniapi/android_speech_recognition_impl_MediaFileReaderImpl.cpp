/*---------------------------------------------------------------------------*
 *  android_speech_recognition_impl_MediaFileReaderImpl.cpp                                        *
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
#include <assert.h>
#include "jniapi.h"
#include "android_speech_recognition_impl_MediaFileReaderImpl.h"
#include "CWrapperMediaFileReaderListener.h"
#include "JNIHelper.h"
#include "AudioStream.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;


JNIEXPORT void JNICALL Java_android_speech_recognition_impl_MediaFileReaderImpl_deleteNativeObject(JNIEnv*,
    jobject , jlong nativeObj)
{
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MediaFileReaderImpl_deleteNativeObject");
  delete(MediaFileReaderProxy*) nativeObj;
}

JNIEXPORT jlong JNICALL Java_android_speech_recognition_impl_MediaFileReaderImpl_createMediaFileReaderProxy
(JNIEnv* env,
 jobject mediaFileReader,
 jstring filename,
 jobject listener)
{
  ReturnCode::Type returnCode = ReturnCode::UNKNOWN;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MediaFileReaderImpl_createMediaFileReaderProxy");
  
  // Create the native listener
  MediaFileReaderListenerProxy nativeListener;
  if (listener != 0)
  {
    JavaVM* jvm = JNIHelper::getJVM(env, mediaFileReader);
    if (env->ExceptionCheck())
      return 0;
    CWrapperMediaFileReaderListener* temp = new CWrapperMediaFileReaderListener(jvm, listener, returnCode);
    if (!temp)
    {
      JNIHelper::throwJavaException(env, mediaFileReader, ReturnCode::OUT_OF_MEMORY);
      return 0;
    }
    else if (returnCode)
    {
      JNIHelper::throwJavaException(env, mediaFileReader, returnCode);
      return 0;
    }
    nativeListener = MediaFileReaderListenerProxy(temp);
    if (!nativeListener)
    {
      delete temp;
      JNIHelper::throwJavaException(env, mediaFileReader, ReturnCode::OUT_OF_MEMORY);
      return 0;
    }
  }
    
  const char* nativeFilename = env->GetStringUTFChars(filename, 0);
  MediaFileReaderProxy nativeMediaFileReader = MediaFileReader::create(nativeFilename, nativeListener, returnCode);
  env->ReleaseStringUTFChars(filename, nativeFilename);
  
  if (returnCode != ReturnCode::SUCCESS)
  {
    JNIHelper::throwJavaException(env, mediaFileReader, returnCode);
    return 0;
  }
  MediaFileReaderProxy* result = new MediaFileReaderProxy(nativeMediaFileReader);
  if (!result || !*result)
  {
    delete result;
    returnCode = ReturnCode::OUT_OF_MEMORY;
    JNIHelper::throwJavaException(env, mediaFileReader, returnCode);
    return 0;
  }
  return (jlong) result;
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_MediaFileReaderImpl_setModeProxy(JNIEnv* env,
    jobject mediaFileReader,
    jlong nativeObj,
    jobject mode)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MediaFileReaderImpl_setModeProxy");
  
  MediaFileReaderProxy* nativeReaderPointer = (MediaFileReaderProxy*) nativeObj;
  if (!nativeReaderPointer)
  {
    JNIHelper::throwJavaException(env, mediaFileReader, ReturnCode::INVALID_STATE);
    return;
  }
  MediaFileReaderProxy& nativeMediaFileReader = *nativeReaderPointer;
  
  MediaFileReader::ReadingMode nativeMode = JNIHelper::toNativeMediaFileReaderMode(env, mode);
  if (env->ExceptionCheck())
    return;
  nativeMediaFileReader->setReadingMode(nativeMode, returnCode);
  
  if (returnCode)
    JNIHelper::throwJavaException(env, mediaFileReader, returnCode);
}

JNIEXPORT jlong JNICALL Java_android_speech_recognition_impl_MediaFileReaderImpl_createAudioProxy(JNIEnv* env,
    jobject mediaFileReader,jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MediaFileReaderImpl_createAudioProxy");
  
  MediaFileReaderProxy* nativeReaderPointer = (MediaFileReaderProxy*) nativeObj;
  if (!nativeReaderPointer)
  {
    JNIHelper::throwJavaException(env, mediaFileReader, ReturnCode::INVALID_STATE);
    return 0;
  }
  MediaFileReaderProxy& nativeMediaFileReader = *nativeReaderPointer;
  
  AudioStreamProxy temp = nativeMediaFileReader->createAudio(returnCode);
  if (returnCode)
  {
    JNIHelper::throwJavaException(env, mediaFileReader, returnCode);
    return 0;
  }
  AudioStreamProxy* audio = new AudioStreamProxy(temp);
  if (!audio)
  {
    JNIHelper::throwJavaException(env, mediaFileReader, returnCode);
    return 0;
  }
  return (jlong) audio;
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_MediaFileReaderImpl_startProxy(JNIEnv* env,
    jobject mediaFileReader,jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MediaFileReaderImpl_startProxy");
  
  MediaFileReaderProxy* nativeReaderPointer = (MediaFileReaderProxy*) nativeObj;
  if (!nativeReaderPointer)
  {
    JNIHelper::throwJavaException(env, mediaFileReader, ReturnCode::INVALID_STATE);
    return;
  }
  MediaFileReaderProxy& nativeMediaFileReader = *nativeReaderPointer;
  nativeMediaFileReader->start(returnCode);
  
  if (returnCode)
    JNIHelper::throwJavaException(env, mediaFileReader, returnCode);
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_MediaFileReaderImpl_stopProxy(JNIEnv* env,
    jobject mediaFileReader,jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MediaFileReaderImpl_stopProxy");
  
  MediaFileReaderProxy* nativeReaderPointer = (MediaFileReaderProxy*) nativeObj;
  if (!nativeReaderPointer)
  {
    JNIHelper::throwJavaException(env, mediaFileReader, ReturnCode::INVALID_STATE);
    return;
  }
  MediaFileReaderProxy& nativeMediaFileReader = *nativeReaderPointer;
  nativeMediaFileReader->stop(returnCode);
  
  if (returnCode)
    JNIHelper::throwJavaException(env, mediaFileReader, returnCode);
}
