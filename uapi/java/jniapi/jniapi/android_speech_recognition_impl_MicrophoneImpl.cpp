/*---------------------------------------------------------------------------*
 *  android_speech_recognition_impl_MicrophoneImpl.cpp                                             *
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
#include "android_speech_recognition_impl_MicrophoneImpl.h"
#include "CWrapperMicrophoneListener.h"
#include "JNIHelper.h"
#include "Codec.h"
#include "Microphone.h"
#include "AudioStream.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;


JNIEXPORT void JNICALL Java_android_speech_recognition_impl_MicrophoneImpl_deleteNativeObject
(JNIEnv* ,
 jobject ,
 jlong nativeObj)
{
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MicrophoneImpl_deleteNativeObject");
  delete(MicrophoneProxy*) nativeObj;
}

JNIEXPORT jlong JNICALL Java_android_speech_recognition_impl_MicrophoneImpl_initNativeObject(JNIEnv* env,
    jobject microphone)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MicrophoneImpl_initNativeObject");
    
  MicrophoneProxy nativeMicrophone = Microphone::getInstance(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    JNIHelper::throwJavaException(env, microphone, returnCode);
    
    return 0;
  }
  MicrophoneProxy* result = new MicrophoneProxy(nativeMicrophone);
  if (!result || !*result)
  {
    delete result;
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return 0;
  }
  
  return (jlong) result;
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_MicrophoneImpl_setCodecProxy
(JNIEnv *env,
 jobject microphone,
 jlong nativeObj,
 jobject codec)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MicrophoneImpl_setCodecProxy");
  
  MicrophoneProxy* nativeMicrophonePointer = (MicrophoneProxy*) nativeObj;
  if (!nativeMicrophonePointer)
  {
    JNIHelper::throwJavaException(env, microphone, ReturnCode::INVALID_STATE);
    return;
  }
  MicrophoneProxy& nativeMicrophone = *nativeMicrophonePointer;
  
  Codec::Type nativeCodec = JNIHelper::toNativeCodec(env, codec);
  if (env->ExceptionCheck())
    return;
  nativeMicrophone->setCodec(nativeCodec, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
    JNIHelper::throwJavaException(env, microphone, returnCode);
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_MicrophoneImpl_setListenerProxy
(JNIEnv *env,
 jobject microphone,
 jlong nativeObj,
 jobject listener)
{
  ReturnCode::Type returnCode = ReturnCode::UNKNOWN;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MicrophoneImpl_setListenerProxy");
  
  MicrophoneProxy* nativeMicrophonePointer = (MicrophoneProxy*) nativeObj;
  if (!nativeMicrophonePointer)
  {
    JNIHelper::throwJavaException(env, microphone, ReturnCode::INVALID_STATE);
    return;
  }
  MicrophoneProxy& nativeMicrophone = *nativeMicrophonePointer;
  
  // Create the native listener
  MicrophoneListenerProxy nativeListener;
  if (listener != 0)
  {
    JavaVM* jvm = JNIHelper::getJVM(env, microphone);
    if (env->ExceptionCheck())
      return;
    CWrapperMicrophoneListener* temp = new CWrapperMicrophoneListener(jvm, listener, returnCode);
    if (!temp)
    {
      JNIHelper::throwJavaException(env, microphone, ReturnCode::OUT_OF_MEMORY);
      return;
    }
    else if (returnCode)
    {
      JNIHelper::throwJavaException(env, microphone, returnCode);
      return;
    }
    nativeListener = MicrophoneListenerProxy(temp);
    if (!nativeListener)
    {
      delete temp;
      JNIHelper::throwJavaException(env, microphone, ReturnCode::OUT_OF_MEMORY);
      return;
    }
  }
  
  nativeMicrophone->setListener(nativeListener, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
    JNIHelper::throwJavaException(env, microphone, returnCode);
}

JNIEXPORT jlong JNICALL Java_android_speech_recognition_impl_MicrophoneImpl_createAudioProxy(JNIEnv *env,
    jobject microphone,
    jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MicrophoneImpl_createAudioProxy");
    
  MicrophoneProxy* nativeMicrophonePointer = (MicrophoneProxy*)nativeObj;
  if (!nativeMicrophonePointer)
  {
    JNIHelper::throwJavaException(env, microphone, ReturnCode::INVALID_STATE);
    return 0;
  }
  MicrophoneProxy& nativeMicrophone = *nativeMicrophonePointer;
  
  AudioStreamProxy nativeAudio = nativeMicrophone->createAudio(returnCode);
  if (returnCode)
  {
    JNIHelper::throwJavaException(env, microphone, returnCode);
    return 0;
  }
  
  AudioStreamProxy* result = new AudioStreamProxy(nativeAudio);
  if (!result || !*result)
  {
    delete result;
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return 0;
  }
  return (jlong) result;
}


JNIEXPORT void JNICALL Java_android_speech_recognition_impl_MicrophoneImpl_startProxy(JNIEnv* env,
    jobject microphone,
    jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MicrophoneImpl_startProxy");
  
  MicrophoneProxy* nativeMicrophonePointer = (MicrophoneProxy*) nativeObj;
  if (!nativeMicrophonePointer)
  {
    JNIHelper::throwJavaException(env, microphone, ReturnCode::INVALID_STATE);
    return;
  }
  MicrophoneProxy& nativeMicrophone = *nativeMicrophonePointer;
  nativeMicrophone->start(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
    JNIHelper::throwJavaException(env, microphone, returnCode);
}


JNIEXPORT void JNICALL Java_android_speech_recognition_impl_MicrophoneImpl_stopProxy(JNIEnv *env,
    jobject microphone,
    jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_MicrophoneImpl_stopProxy");
  
  MicrophoneProxy* nativeMicrophonePointer = (MicrophoneProxy*) nativeObj;
  if (!nativeMicrophonePointer)
  {
    JNIHelper::throwJavaException(env, microphone, ReturnCode::INVALID_STATE);
    return;
  }
  MicrophoneProxy& nativeMicrophone = *nativeMicrophonePointer;
  nativeMicrophone->stop(returnCode);
  
  if (returnCode)
    JNIHelper::throwJavaException(env, microphone, returnCode);
}
