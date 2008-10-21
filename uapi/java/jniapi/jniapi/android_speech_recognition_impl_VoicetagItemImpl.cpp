/*---------------------------------------------------------------------------*
 *  android_speech_recognition_VoicetagItem.cpp                                                    *
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
 * android_speech_recognition_VoicetagItem.cpp
 */
#include <assert.h>
#include "jniapi.h"
#include "android_speech_recognition_impl_VoicetagItemImpl.h"
#include "JNIHelper.h"
#include "VoicetagItem.h"
#include "SrecVoicetagItemImpl.h"
#include "CWrapperVoicetagListener.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;
using namespace android::speech::recognition::srec;

JNIEXPORT jlong JNICALL Java_android_speech_recognition_impl_VoicetagItemImpl_createVoicetagProxy
  (JNIEnv *env, jclass , jstring filename, jobject listener)
{
    ReturnCode::Type returnCode = ReturnCode::UNKNOWN; 
    UAPI_FN_SCOPE("Java_android_speech_recognition_impl_VoicetagItemImpl_createVoicetagProxy");
    const char* nativeFilename = env->GetStringUTFChars(filename, 0);
    // Create the native listener
    VoicetagItemListenerProxy nativeListener;
    if (listener != 0)
    {
        JavaVM* jvm = 0;
        if (env->GetJavaVM(&jvm) < 0)
        {
            JNIHelper::throwRuntimeJavaException(env,0,"GetJavaVM() failed");
            return 0;
        }
        if (env->ExceptionCheck())
            return 0;
        CWrapperVoicetagListener* temp = new CWrapperVoicetagListener(jvm, listener, returnCode);
        if (!temp)
        {
            JNIHelper::throwJavaException(env,0,ReturnCode::OUT_OF_MEMORY);
            return 0;
        }
        else if (returnCode)
        {
            JNIHelper::throwJavaException(env,0,returnCode);
            return 0;
        }
        nativeListener = VoicetagItemListenerProxy(temp);
        if (!nativeListener)
        {
            delete temp;
            JNIHelper::throwJavaException(env,0,ReturnCode::OUT_OF_MEMORY);
            return 0;
        }
    }
    VoicetagItemProxy nativeVoicetag = SrecVoicetagItemImpl::create(nativeFilename,nativeListener,returnCode);
    env->ReleaseStringUTFChars(filename, nativeFilename);
    if (returnCode != ReturnCode::SUCCESS)
    {
        JNIHelper::throwJavaException(env,0,returnCode);
        return 0;
    }
    VoicetagItemProxy* resultVoicetag = new VoicetagItemProxy(nativeVoicetag);
    if (!resultVoicetag || !*resultVoicetag)
    {
        delete resultVoicetag;
        returnCode = ReturnCode::OUT_OF_MEMORY;
        JNIHelper::throwJavaException(env,0,returnCode);
        return 0;
    }
    return (jlong) resultVoicetag;
}

JNIEXPORT jbyteArray JNICALL Java_android_speech_recognition_impl_VoicetagItemImpl_getAudioProxy
  (JNIEnv *env, jobject voicetag, jlong nativeObj)
{
  ReturnCode::Type returnCode; 
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_VoicetagItemImpl_getAudioProxy");
    
  VoicetagItemProxy* proxyVoicetag = (VoicetagItemProxy*)nativeObj;
  if (!proxyVoicetag)
  {
    JNIHelper::throwJavaException(env, voicetag, ReturnCode::INVALID_STATE);
    return 0;
  }
  VoicetagItemProxy& nativeVoicetag = *proxyVoicetag;

  const android::speech::recognition::INT16** _waveform = 0;
  ARRAY_LIMIT   _size = 0;

  nativeVoicetag->getAudio(_waveform,&_size,returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    JNIHelper::throwJavaException(env, voicetag, returnCode);
    return 0;
  }
  jbyteArray result = env->NewByteArray((jsize)_size);
  env->SetByteArrayRegion(result,0,(jsize)_size,(const jbyte*)_waveform);
  if (env->ExceptionCheck())
  {
      env->DeleteLocalRef(result);
      return 0;
  }
  jobject returnedObj = env->NewGlobalRef(result);
  env->DeleteLocalRef(result);
  return (jbyteArray)returnedObj;
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_VoicetagItemImpl_setAudioProxy
  (JNIEnv* env, jobject voicetag, jlong nativeObj, jbyteArray data)
{
  ReturnCode::Type returnCode; 
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_VoicetagItemImpl_setAudioProxy");
    
  VoicetagItemProxy* proxyVoicetag = (VoicetagItemProxy*)nativeObj;
  if (!proxyVoicetag)
  {
    JNIHelper::throwJavaException(env, voicetag, ReturnCode::INVALID_STATE);
    return;
  }
  VoicetagItemProxy& nativeVoicetag = *proxyVoicetag;

  jbyte* arrBytes = env->GetByteArrayElements(data,0);
   
  android::speech::recognition::INT16* _waveform = 0;
  ARRAY_LIMIT   _size =  env->GetArrayLength(data)/2; // By 2 because INT16
  _waveform = new android::speech::recognition::INT16[_size];
  if (!_waveform)
  {
     env->ReleaseByteArrayElements(data,arrBytes,0);
     JNIHelper::throwJavaException(env, voicetag, ReturnCode::OUT_OF_MEMORY);
     return;
  }
  jint j = 0;
  ARRAY_LIMIT i;
  android::speech::recognition::INT16 word;
  for(i=0;i<_size;i++)
  {
    j =i*2;
    word = arrBytes[j];
    _waveform[i] = (word<<8)+arrBytes[j+1];
  }
  env->ReleaseByteArrayElements(data,arrBytes,0);

  nativeVoicetag->setAudio(_waveform,_size,returnCode);

  if (returnCode != ReturnCode::SUCCESS)
  {
    delete[] _waveform;
    JNIHelper::throwJavaException(env, voicetag, returnCode);
  }
}
JNIEXPORT void JNICALL Java_android_speech_recognition_impl_VoicetagItemImpl_saveVoicetagProxy
  (JNIEnv *env, jobject voicetag, jlong nativeObj, jstring path)
{
  ReturnCode::Type returnCode; 
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_VoicetagItemImpl_saveVoicetagproxy");
    
  SrecVoicetagItemImplProxy* proxyVoicetag = (SrecVoicetagItemImplProxy*)nativeObj;
  if (!proxyVoicetag)
  {
    JNIHelper::throwJavaException(env, voicetag, ReturnCode::INVALID_STATE);
    return;
  }
  SrecVoicetagItemImplProxy& nativeVoicetag = *proxyVoicetag;
  const char* nativePath = env->GetStringUTFChars(path, 0);
  nativeVoicetag->save(nativePath,returnCode);
  env->ReleaseStringUTFChars(path, nativePath);
  if (returnCode != ReturnCode::SUCCESS)
  {
    JNIHelper::throwJavaException(env, voicetag, returnCode);
  }
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_VoicetagItemImpl_loadVoicetagProxy
  (JNIEnv *env, jobject voicetag,jlong nativeObj)
{
  ReturnCode::Type returnCode; 
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_VoicetagItemImpl_loadVoicetagProxy");
    
  SrecVoicetagItemImplProxy* proxyVoicetag = (SrecVoicetagItemImplProxy*)nativeObj;
  if (!proxyVoicetag)
  {
    JNIHelper::throwJavaException(env, voicetag, ReturnCode::INVALID_STATE);
    return;
  }
  SrecVoicetagItemImplProxy& nativeVoicetag = *proxyVoicetag;

  nativeVoicetag->load(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    JNIHelper::throwJavaException(env, voicetag, returnCode);
  }
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_VoicetagItemImpl_deleteNativeObject
  (JNIEnv *, jobject, jlong nativeObj)
{
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_VoicetagItemImpl_deleteNativeObject");
  delete(SrecVoicetagItemImplProxy*) nativeObj;
}
