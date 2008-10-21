/*---------------------------------------------------------------------------*
 *  android_speech_recognition_impl_DeviceSpeakerImpl.cpp                                          *
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
#include "android_speech_recognition_impl_DeviceSpeakerImpl.h"
#include "JNIHelper.h"
#include "CWrapperDeviceSpeakerListener.h"
#include "DeviceSpeaker.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;


JNIEXPORT void JNICALL Java_android_speech_recognition_impl_DeviceSpeakerImpl_deleteNativeObject
(JNIEnv* ,
 jobject ,
 jlong nativeObj)
{
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_DeviceSpeakerImpl_deleteNativeObject");
  delete(DeviceSpeakerProxy*) nativeObj;
}

JNIEXPORT jlong JNICALL Java_android_speech_recognition_impl_DeviceSpeakerImpl_initNativeObject(JNIEnv* env,
    jobject deviceSpeaker)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_DeviceSpeakerImpl_initNativeObject");
  
  // Create the native DeviceSpeaker
  DeviceSpeakerProxy nativeDeviceSpeaker = DeviceSpeaker::getInstance(returnCode);
  if (returnCode)
  {
    JNIHelper::throwJavaException(env, deviceSpeaker, returnCode);
    return 0;
  }
  
  DeviceSpeakerProxy* result = new DeviceSpeakerProxy(nativeDeviceSpeaker);
  if (!result || !*result)
  {
    delete result;
    returnCode = ReturnCode::OUT_OF_MEMORY;
    JNIHelper::throwJavaException(env, deviceSpeaker, returnCode);
    return 0;
  }
  return (jlong) result;
}


JNIEXPORT void JNICALL Java_android_speech_recognition_impl_DeviceSpeakerImpl_startProxy(JNIEnv* env,
    jobject deviceSpeaker,
    jlong nativeObj,
    jlong audioObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_DeviceSpeakerImpl_startProxy");
  
  DeviceSpeakerProxy* nativeSpeakerPointer = (DeviceSpeakerProxy*) nativeObj;
 
  if (!nativeSpeakerPointer)
  {
    JNIHelper::throwJavaException(env, deviceSpeaker, ReturnCode::INVALID_STATE);
    return;
  }
  DeviceSpeakerProxy& nativeDeviceSpeaker = *nativeSpeakerPointer;
  
  AudioStreamProxy* nativeAudioPointer = (AudioStreamProxy*) audioObj;

  if (!nativeAudioPointer)
  {
    JNIHelper::throwJavaException(env, deviceSpeaker, ReturnCode::INVALID_STATE);
    return;
  }
  AudioStreamProxy& nativeAudio = *nativeAudioPointer;
  nativeDeviceSpeaker->start(nativeAudio, returnCode);
  
  switch (returnCode)
  {
    case ReturnCode::SUCCESS:
      return;
    case ReturnCode::ILLEGAL_ARGUMENT:
    case ReturnCode::AUDIO_ALREADY_IN_USE:
    case ReturnCode::END_OF_STREAM:
    {
      jclass clazz = JNIHelper::loadClass(env, deviceSpeaker, "java.lang.IllegalStateException");
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
      JNIHelper::throwJavaException(env, deviceSpeaker, returnCode);
  }
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_DeviceSpeakerImpl_stopProxy
(JNIEnv* env,
 jobject deviceSpeaker,
 jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_DeviceSpeakerImpl_stopProxy");
                                   
  
  DeviceSpeakerProxy* nativeSpeakerPointer = (DeviceSpeakerProxy*) nativeObj;
    
  if (!nativeSpeakerPointer)
  {
    JNIHelper::throwJavaException(env, deviceSpeaker, ReturnCode::INVALID_STATE);
    return;
  }
  DeviceSpeakerProxy& nativeDeviceSpeaker = *nativeSpeakerPointer;
  nativeDeviceSpeaker->stop(returnCode);
  if (returnCode)
    JNIHelper::throwJavaException(env, deviceSpeaker, returnCode);
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_DeviceSpeakerImpl_setCodecProxy
(JNIEnv* env,
 jobject deviceSpeaker,
 jlong nativeObj,
 jobject codec)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_DeviceSpeakerImpl_setCodecProxy");
  
  DeviceSpeakerProxy* nativeSpeakerPointer = (DeviceSpeakerProxy*) nativeObj;
  
  if (!nativeSpeakerPointer)
  {
    JNIHelper::throwJavaException(env, deviceSpeaker, ReturnCode::INVALID_STATE);
    return;
  }
  DeviceSpeakerProxy& nativeDeviceSpeaker = *nativeSpeakerPointer;
  
  Codec::Type nativeCodec = JNIHelper::toNativeCodec(env, codec);
  if (env->ExceptionCheck())
    return;
    
  nativeDeviceSpeaker->setCodec(nativeCodec, returnCode);
  
  if (returnCode)
    JNIHelper::throwJavaException(env, deviceSpeaker, returnCode);
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_DeviceSpeakerImpl_setListenerProxy
(JNIEnv* env,
 jobject deviceSpeaker,
 jlong nativeObj,
 jobject listener)
{
  ReturnCode::Type returnCode = ReturnCode::UNKNOWN;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_DeviceSpeakerImpl_setListenerProxy");
  
  DeviceSpeakerProxy* nativeSpeakerPointer = (DeviceSpeakerProxy*) nativeObj;
     
  if (!nativeSpeakerPointer)
  {
    JNIHelper::throwJavaException(env, deviceSpeaker, ReturnCode::INVALID_STATE);
    return;
  }
  DeviceSpeakerProxy& nativeDeviceSpeaker = *nativeSpeakerPointer;
  
  // Create the native listener
  DeviceSpeakerListenerProxy nativeListener;
  if (listener != 0)
  {
    JavaVM* jvm = JNIHelper::getJVM(env, deviceSpeaker);
    if (env->ExceptionCheck())
      return;
    CWrapperDeviceSpeakerListener* temp = new CWrapperDeviceSpeakerListener(jvm, listener, returnCode);
    if (!temp)
    {
      JNIHelper::throwJavaException(env, deviceSpeaker, ReturnCode::OUT_OF_MEMORY);
      return;
    }
    else if (returnCode)
    {
      JNIHelper::throwJavaException(env, deviceSpeaker, returnCode);
      return;
    }
    nativeListener = DeviceSpeakerListenerProxy(temp);
    if (!nativeListener)
    {
      delete temp;
      JNIHelper::throwJavaException(env, deviceSpeaker, ReturnCode::OUT_OF_MEMORY);
      return;
    }
  }
  
  nativeDeviceSpeaker->setListener(nativeListener, returnCode);
  if (returnCode)
    JNIHelper::throwJavaException(env, deviceSpeaker, returnCode);
}
