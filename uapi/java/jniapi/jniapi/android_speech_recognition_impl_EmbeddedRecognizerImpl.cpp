/*---------------------------------------------------------------------------*
 *  android_speech_recognition_impl_EmbeddedRecognizerImpl.cpp                                     *
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

#include <string.h>
#include <assert.h>
#include "jniapi.h"
#include "android_speech_recognition_impl_EmbeddedRecognizerImpl.h"
#include "CWrapperRecognizerListener.h"
#include "CWrapperSrecGrammarListener.h"
#include "CWrapperEmbeddedGrammarListener.h"
#include "JNIHelper.h"
#include "EmbeddedRecognizer.h"
#include "Grammar.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_deleteNativeObject
(JNIEnv* ,
 jobject ,
 jlong nativeObj)
{
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_deleteNativeObject");
  delete(EmbeddedRecognizerProxy*) nativeObj;
}

JNIEXPORT jlong JNICALL Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_getInstanceProxy(JNIEnv* env,
    jobject recognizer)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_getInstanceProxy");
  
  EmbeddedRecognizerProxy nativeRecognizer = EmbeddedRecognizer::getInstance(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    JNIHelper::throwJavaException(env, recognizer, returnCode);
    return 0;
  }
  EmbeddedRecognizerProxy* result = new EmbeddedRecognizerProxy(nativeRecognizer);
  if (!result || !*result)
  {
    delete result;
    returnCode = ReturnCode::OUT_OF_MEMORY;
    JNIHelper::throwJavaException(env, recognizer, returnCode);
    return 0;
  }
  else if (returnCode)
  {
    JNIHelper::throwJavaException(env, recognizer, returnCode);
    return 0;
  }
  return (jlong) result;
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_configureProxy(JNIEnv* env,
    jobject recognizer,
    jlong nativeObj,
    jstring config)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_configureProxy");
  
  EmbeddedRecognizerProxy* nativeRecognizerPointer = (EmbeddedRecognizerProxy*) nativeObj;
  if (!nativeRecognizerPointer)
  {
    JNIHelper::throwJavaException(env, recognizer, ReturnCode::INVALID_STATE);
    return;
  }
  EmbeddedRecognizerProxy& nativeRecognizer = *nativeRecognizerPointer;
  
  const char* nativeConfig = env->GetStringUTFChars(config, 0);
  nativeRecognizer->configure(nativeConfig, returnCode);
  env->ReleaseStringUTFChars(config, nativeConfig);
  if (returnCode)
  {
    JNIHelper::throwJavaException(env, recognizer, returnCode);
    return;
  }
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_setListenerProxy(JNIEnv* env,
    jobject recognizer,
    jlong nativeObj,
    jobject listener)
{
  ReturnCode::Type returnCode = ReturnCode::UNKNOWN;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_setListenerProxy");
  
  EmbeddedRecognizerProxy* nativeRecognizerPointer = (EmbeddedRecognizerProxy*) nativeObj;
  if (!nativeRecognizerPointer)
  {
    JNIHelper::throwJavaException(env, recognizer, ReturnCode::INVALID_STATE);
    return;
  }
  EmbeddedRecognizerProxy& nativeRecognizer = *nativeRecognizerPointer;
  
  RecognizerListenerProxy nativeListener;
  if (listener != 0)
  {
    JavaVM* jvm = JNIHelper::getJVM(env, recognizer);
    if (env->ExceptionCheck())
      return;
    CWrapperRecognizerListener* temp = new CWrapperRecognizerListener(jvm, listener, returnCode);
    if (!temp)
    {
      JNIHelper::throwJavaException(env, recognizer, ReturnCode::OUT_OF_MEMORY);
      return;
    }
    else if (returnCode)
    {
      JNIHelper::throwJavaException(env, recognizer, returnCode);
      return;
    }
    nativeListener = RecognizerListenerProxy(temp);
    if (!nativeListener)
    {
      delete temp;
      JNIHelper::throwJavaException(env, recognizer, ReturnCode::OUT_OF_MEMORY);
      return;
    }
  }
  
  nativeRecognizer->setListener(nativeListener, returnCode);
  if (returnCode)
  {
    JNIHelper::throwJavaException(env, recognizer, returnCode);
    return;
  }
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_recognizeProxy(JNIEnv* env,
    jobject recognizer,
    jlong nativeObj,
    jlong audioObj,
    jlongArray grammars)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_recognizeProxy");
    
  EmbeddedRecognizerProxy* nativeRecognizerPointer = (EmbeddedRecognizerProxy*) nativeObj;
  if (!nativeRecognizerPointer)
  {
    JNIHelper::throwJavaException(env, recognizer, ReturnCode::INVALID_STATE);
    return;
  }
  EmbeddedRecognizerProxy& nativeRecognizer = *nativeRecognizerPointer;
  
  AudioStreamProxy* nativeAudioPointer = (AudioStreamProxy*) audioObj;

  if (!nativeAudioPointer)
  {
    JNIHelper::throwJavaException(env, recognizer, ReturnCode::INVALID_STATE);
    return;
  }
  AudioStreamProxy& nativeAudio = *nativeAudioPointer;
  
  jsize count = env->GetArrayLength(grammars);
  assert(count > 0);

  // copy grammars
  jlong *_Grammars = env->GetLongArrayElements(grammars,0);
  GrammarProxy* nativeGrammars = new GrammarProxy[count];
  for (jsize i = 0; i < count; ++i)
    nativeGrammars[i] = *((GrammarProxy*)_Grammars[i]);
  nativeRecognizer->recognize(nativeAudio, nativeGrammars, count, returnCode);
  env->ReleaseLongArrayElements(grammars,(jlong*)_Grammars,0);
  delete[] nativeGrammars;
  nativeGrammars =NULL;
 
  if (returnCode)
    JNIHelper::throwJavaException(env, recognizer, returnCode);
}

JNIEXPORT jlong JNICALL Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_createEmbeddedGrammarProxy
(JNIEnv* env,
 jobject recognizer,
 jlong nativeObj,
 jstring url,
 jobject listener)
{
  ReturnCode::Type returnCode = ReturnCode::UNKNOWN;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_createEmbeddedGrammarProxy");
  
  EmbeddedRecognizerProxy* nativeRecognizerPointer = (EmbeddedRecognizerProxy*) nativeObj;
  if (!nativeRecognizerPointer)
  {
    JNIHelper::throwJavaException(env, recognizer, ReturnCode::INVALID_STATE);
    return 0;
  }
  EmbeddedRecognizerProxy& nativeRecognizer = *nativeRecognizerPointer;
 

  EmbeddedGrammarListenerProxy nativeListener;
  if (listener != 0)
  {
    JavaVM* jvm = JNIHelper::getJVM(env, recognizer);
    if (env->ExceptionCheck())
      return 0;

    jclass EmbeddedGrammarListenerClass = JNIHelper::loadClass(env, listener, "android.speech.recognition.EmbeddedGrammarListener");
    assert(EmbeddedGrammarListenerClass != 0);
    jclass SrecGrammarListenerClass = JNIHelper::loadClass(env, listener, "android.speech.recognition.SrecGrammarListener");
    assert(SrecGrammarListenerClass != 0);

    if (env->IsInstanceOf(listener, SrecGrammarListenerClass))
    { 
      env->DeleteLocalRef(SrecGrammarListenerClass);
      env->DeleteLocalRef(EmbeddedGrammarListenerClass);
      CWrapperSrecGrammarListener* temp = new CWrapperSrecGrammarListener(jvm, listener, returnCode);
      if (!temp)
      {
        JNIHelper::throwJavaException(env, recognizer, ReturnCode::OUT_OF_MEMORY);
        return 0;
      }
      else if (returnCode)
      {
        JNIHelper::throwJavaException(env, recognizer, returnCode);
        return 0;
      }
      nativeListener = SrecGrammarListenerProxy(temp);
      if (!nativeListener)
      {
        delete temp;
        JNIHelper::throwJavaException(env, recognizer, ReturnCode::OUT_OF_MEMORY);
        return 0;
      }
    }
    else if (env->IsInstanceOf(listener, EmbeddedGrammarListenerClass))
    {
      env->DeleteLocalRef(SrecGrammarListenerClass);
      env->DeleteLocalRef(EmbeddedGrammarListenerClass);
      CWrapperEmbeddedGrammarListener* temp = new CWrapperEmbeddedGrammarListener(jvm, listener, returnCode);
      if (!temp)
      {
        JNIHelper::throwJavaException(env, recognizer, ReturnCode::OUT_OF_MEMORY);
        return 0;
      }
      else if (returnCode)
      {
        JNIHelper::throwJavaException(env, recognizer, returnCode);
        return 0;
      }
      nativeListener = EmbeddedGrammarListenerProxy(temp);
      if (!nativeListener)
      {
        delete temp;
        JNIHelper::throwJavaException(env, recognizer, ReturnCode::OUT_OF_MEMORY);
        return 0;
      }
    }
  }
   
  const char* nativeURL = env->GetStringUTFChars(url, 0);
  
  GrammarProxy nativeGrammar = nativeRecognizer->createGrammar(nativeURL, nativeListener, returnCode);
  env->ReleaseStringUTFChars(url, nativeURL);
  if (returnCode)
  {
    JNIHelper::throwJavaException(env, recognizer, returnCode);
    return 0;
  }
  
  GrammarProxy* result = new GrammarProxy(nativeGrammar);
  if (!result || !*result)
  {
    delete result;
    returnCode = ReturnCode::OUT_OF_MEMORY;
    JNIHelper::throwJavaException(env, recognizer, returnCode);
    return 0;
  }
  
  return (jlong) result;
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_stopProxy(JNIEnv* env,
    jobject recognizer,
    jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_stopProxy");
    
  EmbeddedRecognizerProxy* nativeRecognizerPointer = (EmbeddedRecognizerProxy*) nativeObj;

  if (!nativeRecognizerPointer)
  {
    JNIHelper::throwJavaException(env, recognizer, ReturnCode::INVALID_STATE);
    return;
  }
  EmbeddedRecognizerProxy& nativeRecognizer = *nativeRecognizerPointer;
  
  nativeRecognizer->stop(returnCode);
  if (returnCode)
    JNIHelper::throwJavaException(env, recognizer, returnCode);
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_setParametersProxy
(JNIEnv* env,
 jobject recognizer,
 jlong nativeObj,
 jobject parameters)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_setParametersProxy");
  
  EmbeddedRecognizerProxy* nativeRecognizerPointer = (EmbeddedRecognizerProxy*) nativeObj;
  if (!nativeRecognizerPointer)
  {
    JNIHelper::throwJavaException(env, recognizer, ReturnCode::INVALID_STATE);
    return;
  }
  EmbeddedRecognizerProxy& nativeRecognizer = *nativeRecognizerPointer;
  
  jclass hashtableClass = JNIHelper::loadClass(env, parameters, "java.util.Hashtable");
  assert(hashtableClass != 0);
  jclass EnumerationClass = JNIHelper::loadClass(env, parameters, "java.util.Enumeration");
  assert(EnumerationClass != 0);

  jmethodID hashtableSize = env->GetMethodID(hashtableClass, "size", "()I");
  assert(hashtableSize != 0);
  jint size = env->CallIntMethod(parameters, hashtableSize);
        
  if (env->ExceptionCheck())
  {
    env->DeleteLocalRef(hashtableClass);
    env->DeleteLocalRef(EnumerationClass);
    return;
  }
  const char** keys = new const char*[size];
  if (!keys)
  {
    env->DeleteLocalRef(hashtableClass);
    env->DeleteLocalRef(EnumerationClass);
    JNIHelper::throwJavaException(env, recognizer, ReturnCode::OUT_OF_MEMORY);
    return;
  }
  for (jint i = 0; i < size; ++i)
    keys[i] = 0;
  const char** values = new const char*[size];
  if (!values)
  {
    delete[] keys;
    env->DeleteLocalRef(hashtableClass);
    env->DeleteLocalRef(EnumerationClass);
    JNIHelper::throwJavaException(env, recognizer, ReturnCode::OUT_OF_MEMORY);
    return;
  }
  for (jint i = 0; i < size; ++i)
    values[i] = 0;
    
  jmethodID hashtableHasMoreElements = env->GetMethodID(EnumerationClass, "hasMoreElements",
                                       "()Z");
  assert(hashtableHasMoreElements != 0);
  jmethodID hashtableNextElement = env->GetMethodID(EnumerationClass, "nextElement",
                                   "()Ljava/lang/Object;");
  assert(hashtableNextElement != 0);
  
  jmethodID hashtableKeys = env->GetMethodID(hashtableClass, "keys", "()Ljava/util/Enumeration;");
  assert(hashtableKeys != 0);
  jobject keysEnumeration = env->CallObjectMethod(parameters, hashtableKeys);
  jmethodID hashtableElements;
  jobject valuesEnumeration;
  if (env->ExceptionCheck())
    goto CLEANUP;
  assert(keysEnumeration != 0);
  
  for (jsize i = 0; env->CallBooleanMethod(keysEnumeration, hashtableHasMoreElements); ++i)
  {
    if (env->ExceptionCheck())
      goto CLEANUP;
    jstring key = (jstring) env->CallObjectMethod(keysEnumeration, hashtableNextElement);
    if (env->ExceptionCheck())
      goto CLEANUP;
    keys[i] = new char[env->GetStringLength(key)+1];
    if (!keys[i])
    {
      env->DeleteLocalRef(key);
      JNIHelper::throwJavaException(env, recognizer, ReturnCode::OUT_OF_MEMORY);
      goto CLEANUP;
    }
    const char* keyString = env->GetStringUTFChars(key, 0);
    strcpy((char*)keys[i], keyString);
    env->ReleaseStringUTFChars(key, keyString);
    env->DeleteLocalRef(key);
  }
  
  hashtableElements = env->GetMethodID(hashtableClass, "elements", "()Ljava/util/Enumeration;");
  assert(hashtableElements != 0);
  valuesEnumeration = env->CallObjectMethod(parameters, hashtableElements);
  if (env->ExceptionCheck())
    goto CLEANUP;
  assert(valuesEnumeration != 0);
  
  for (jsize i = 0; env->CallBooleanMethod(valuesEnumeration, hashtableHasMoreElements); ++i)
  {
    if (env->ExceptionCheck())
    {
      env->DeleteLocalRef(valuesEnumeration);
      goto CLEANUP;
    }
    jstring value = (jstring) env->CallObjectMethod(valuesEnumeration, hashtableNextElement);
    if (env->ExceptionCheck())
    {
      env->DeleteLocalRef(valuesEnumeration);
      goto CLEANUP;
    }
    values[i] = new char[env->GetStringLength(value)+1];
    if (!values[i])
    {
      env->DeleteLocalRef(value);
      env->DeleteLocalRef(valuesEnumeration);
      JNIHelper::throwJavaException(env, recognizer, ReturnCode::OUT_OF_MEMORY);
      goto CLEANUP;
    }
    const char* valueString = env->GetStringUTFChars(value, 0);
    strcpy((char*)values[i], valueString);
    env->ReleaseStringUTFChars(value, valueString);
    env->DeleteLocalRef(value);
  }
  
  nativeRecognizer->setParameters(keys, values, size, returnCode);
  env->DeleteLocalRef(hashtableClass);
  env->DeleteLocalRef(EnumerationClass);
  env->DeleteLocalRef(keysEnumeration);
  env->DeleteLocalRef(valuesEnumeration);
  for (jint i = 0; i < size; ++i)
  {
    delete[] keys[i];
    delete[] values[i];
  }
  delete[] keys;
  delete[] values;
  
  if (returnCode != ReturnCode::SUCCESS)
    JNIHelper::throwJavaException(env, recognizer, returnCode);
  return;
CLEANUP:
  env->DeleteLocalRef(hashtableClass);
  env->DeleteLocalRef(EnumerationClass);
  env->DeleteLocalRef(keysEnumeration);
  for (jint i = 0; i < size; ++i)
  {
    delete[] keys[i];
    delete[] values[i];
  }
  delete[] keys;
  delete[] values;
}


JNIEXPORT void JNICALL Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_getParametersProxy
(JNIEnv *env,
 jobject recognizer,
 jlong nativeObj,
 jobject parameters)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_getParametersProxy");
    
  EmbeddedRecognizerProxy* nativeRecognizerPointer = (EmbeddedRecognizerProxy*) nativeObj;
  if (!nativeRecognizerPointer)
  {
    JNIHelper::throwJavaException(env, recognizer, ReturnCode::INVALID_STATE);
    return;
  }
  EmbeddedRecognizerProxy& nativeRecognizer = *nativeRecognizerPointer;
  
  jclass vectorClass = JNIHelper::loadClass(env, parameters, "java.util.Vector");
  assert(vectorClass != 0);
  jmethodID vectorSize = env->GetMethodID(vectorClass, "size", "()I");
  assert(vectorSize != 0);
  jint size = env->CallIntMethod(parameters, vectorSize);
  if (env->ExceptionCheck())
  {
    env->DeleteLocalRef(vectorClass);
    return;
  }
  const char** keys = new const char*[size];
  if (!keys)
  {
    env->DeleteLocalRef(vectorClass);
    JNIHelper::throwJavaException(env, recognizer, ReturnCode::OUT_OF_MEMORY);
    return;
  }
  for (jint i = 0; i < size; ++i)
    keys[i] = 0;
    
  jmethodID vectorGet = env->GetMethodID(vectorClass, "get", "(I)Ljava/lang/Object;");
  assert(vectorGet != 0);
  env->DeleteLocalRef(vectorClass);

  for (jint i = 0; i < size; ++i)
  {
    jstring key = (jstring) env->CallObjectMethod(parameters, vectorGet, i);
    if (env->ExceptionCheck())
    {
      for (jint i = 0; i < size; ++i)
        delete[] keys[i];
      delete[] keys;
      return;
    }
    keys[i] = new char[env->GetStringLength(key)+1];
    if (!keys[i])
    {
      env->DeleteLocalRef(key);
      for (jint i = 0; i < size; ++i)
        delete[] keys[i];
      delete[] keys;
      JNIHelper::throwJavaException(env, recognizer, ReturnCode::OUT_OF_MEMORY);
      return;
    }
    const char* keyString = env->GetStringUTFChars(key, 0);
    strcpy((char*)keys[i], keyString);
    env->ReleaseStringUTFChars(key, keyString);
    env->DeleteLocalRef(key);
  }
  
  nativeRecognizer->getParameters(keys, size, returnCode);
  for (jint i = 0; i < size; ++i)
    delete[] keys[i];
  delete[] keys;
  
  if (returnCode != ReturnCode::SUCCESS)
    JNIHelper::throwJavaException(env, recognizer, returnCode);
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_resetAcousticStateProxy
(JNIEnv* env,
 jobject recognizer,
 jlong nativeObj)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_EmbeddedRecognizerImpl_resetAcousticStateProxy");
    
  EmbeddedRecognizerProxy* nativeRecognizerPointer = (EmbeddedRecognizerProxy*) nativeObj;
  if (!nativeRecognizerPointer)
  {
    JNIHelper::throwJavaException(env, recognizer, ReturnCode::INVALID_STATE);
    return;
  }
  EmbeddedRecognizerProxy& nativeRecognizer = *nativeRecognizerPointer;
  
  nativeRecognizer->resetAcousticState(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    JNIHelper::throwJavaException(env, recognizer, returnCode);
  }
}
