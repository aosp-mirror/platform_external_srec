/*---------------------------------------------------------------------------*
 *  android_speech_recognition_impl_MediaFileWriterImpl.cpp                                        *
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
#include "android_speech_recognition_impl_MediaFileWriterImpl.h"
#include "JNIHelper.h"
#include "CWrapperMediaFileWriterListener.h"
#include "MediaFileWriter.h"

#include <assert.h>

using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;


JNIEXPORT void JNICALL Java_android_speech_recognition_impl_MediaFileWriterImpl_deleteNativeObject(JNIEnv*,
    jobject,
    jlong nativeObj)
{
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MediaFileWriterImpl_deleteNativeObject");
  delete(MediaFileWriterProxy*) nativeObj;
}

JNIEXPORT jlong JNICALL Java_android_speech_recognition_impl_MediaFileWriterImpl_createMediaFileWriterProxy(JNIEnv* env,
    jobject mediaFileWriter,
    jobject listener)
{
  ReturnCode::Type returnCode = ReturnCode::UNKNOWN;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MediaFileWriterImpl_createMediaFileWriterProxy");
  
  // Create the native listener
  MediaFileWriterListenerProxy nativeListener;
  if (listener != 0)
  {
    JavaVM* jvm = JNIHelper::getJVM(env, mediaFileWriter);
    if (env->ExceptionCheck())
      return 0;
    CWrapperMediaFileWriterListener* temp = new CWrapperMediaFileWriterListener(jvm, listener, returnCode);
    if (!temp)
    {
      JNIHelper::throwJavaException(env, mediaFileWriter, ReturnCode::OUT_OF_MEMORY);
      return 0;
    }
    else if (returnCode)
    {
      JNIHelper::throwJavaException(env, mediaFileWriter, returnCode);
      return 0;
    }
    nativeListener = MediaFileWriterListenerProxy(temp);
    if (!nativeListener)
    {
      delete temp;
      JNIHelper::throwJavaException(env, mediaFileWriter, ReturnCode::OUT_OF_MEMORY);
      return 0;
    }
  }
  
  // Create the native microphone
  MediaFileWriterProxy nativeMediaFileWriter = MediaFileWriter::create(nativeListener, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    JNIHelper::throwJavaException(env, mediaFileWriter, returnCode);
    return 0;
  }
  MediaFileWriterProxy* result = new MediaFileWriterProxy(nativeMediaFileWriter);
  if (!result || !*result)
  {
    delete result;
    returnCode = ReturnCode::OUT_OF_MEMORY;
    JNIHelper::throwJavaException(env, mediaFileWriter, returnCode);
    return 0;
  }
  
  return (jlong) result;
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_MediaFileWriterImpl_saveProxy(JNIEnv *env,
    jobject mediaFileWriter,
    jlong nativeObj,
    jlong audioObj,
    jstring filename)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MediaFileWriterImpl_saveProxy");
  
  MediaFileWriterProxy* nativeWriterPointer = (MediaFileWriterProxy*) nativeObj;
  if (!nativeWriterPointer)
  {
    JNIHelper::throwJavaException(env, mediaFileWriter, ReturnCode::INVALID_STATE);
    return;
  }
  MediaFileWriterProxy& nativeMediaFileWriter = *nativeWriterPointer;
  
  AudioStreamProxy* nativeAudioPointer = (AudioStreamProxy*) audioObj;
  if (!nativeAudioPointer)
  {
    JNIHelper::throwJavaException(env, mediaFileWriter, ReturnCode::INVALID_STATE);
    return;
  }
  AudioStreamProxy& nativeAudio = *nativeAudioPointer;
  
  const char* nativeFilename = env->GetStringUTFChars(filename, 0);
  nativeMediaFileWriter->save(nativeAudio, nativeFilename, returnCode);
  env->ReleaseStringUTFChars(filename, nativeFilename);
  
  switch (returnCode)
  {
    case ReturnCode::SUCCESS:
      return;
    case ReturnCode::ILLEGAL_ARGUMENT:
    case ReturnCode::AUDIO_ALREADY_IN_USE:
    case ReturnCode::END_OF_STREAM:
    {
      jclass clazz = JNIHelper::loadClass(env, mediaFileWriter, "java.lang.IllegalStateException");
      jmethodID constructor = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;)V");
      jthrowable exception;
      if (constructor != 0)
        exception = (jthrowable) env->NewObject(clazz, constructor, env->NewStringUTF(ReturnCode::toString(returnCode)));
      else
      {
        // Try the default constructor instead
        constructor = env->GetMethodID(clazz, "<init>", "()V");
        assert(constructor != 0);
        exception = (jthrowable) env->NewObject(clazz, constructor);
      }
      env->DeleteLocalRef(clazz);
      env->Throw(exception);
      env->DeleteLocalRef(exception);
      break;
    }
    default:
      JNIHelper::throwJavaException(env, mediaFileWriter, returnCode);
  }
}
