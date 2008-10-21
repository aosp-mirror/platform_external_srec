/*---------------------------------------------------------------------------*
 *  CWrapperRecognizerListener.cpp                                           *
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

#include <assert.h>
#include "CWrapperRecognizerListener.h"
#include "jniapi.h"
#include "JNIHelper.h"
#include "Logger.h"


using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;


CWrapperRecognizerListener::CWrapperRecognizerListener(JavaVM* _jvm, jobject _listener,
    ReturnCode::Type& returnCode):
    jvm(_jvm),
    listener(0)
{
  JNIEnv* env = JNIHelper::getEnv(jvm);
  listener = env->NewGlobalRef(_listener);
  if (!listener)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return;
  }
  returnCode = ReturnCode::SUCCESS;
}

CWrapperRecognizerListener::~CWrapperRecognizerListener()
{
  UAPI_FN_NAME("CWrapperRecognizerListener::~CWrapperRecognizerListener");
    
  UAPI_TRACE(fn,"this=%p\n", this);
  
  // Delete global reference to the Java listener
  JNIEnv* env = JNIHelper::getEnv(jvm);
  env->DeleteGlobalRef(listener);
}

void CWrapperRecognizerListener::invokeMethod(const char* methodName)
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::invokeMethod");
    
  JNIEnv* env = JNIHelper::getEnv(jvm);
  jclass listenerClass = env->GetObjectClass(listener);
  jmethodID mid = env->GetMethodID(listenerClass, methodName, "()V");
  env->DeleteLocalRef(listenerClass);
  assert(mid != 0);
  if (mid != 0)
    env->CallVoidMethod(listener, mid, 0);
  
  if (env->ExceptionCheck())
  {
    jthrowable exc = env->ExceptionOccurred();
    env->ExceptionClear();
    onError(exc);
    env->DeleteLocalRef(exc);
  }
}

void CWrapperRecognizerListener::onStarted()
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::onStarted");
    
  invokeMethod("onStarted");
}

void CWrapperRecognizerListener::onBeginningOfSpeech()
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::onBeginningOfSpeech");
    
  invokeMethod("onBeginningOfSpeech");
}

void CWrapperRecognizerListener::onEndOfSpeech()
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::onEndOfSpeech");
    
  invokeMethod("onEndOfSpeech");
}


void CWrapperRecognizerListener::onAcousticStateReset()
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::onAcousticStateReset");
    
  invokeMethod("onAcousticStateReset");
}


void CWrapperRecognizerListener::onStopped()
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::onStopped");
    
  invokeMethod("onStopped");
}

void CWrapperRecognizerListener::onRecognitionFailure(FailureReason reason)
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::onRecognitionFailure");
    
  JNIEnv* env = JNIHelper::getEnv(jvm);
  
  // Get object class
  assert(listener != 0);
  jclass cls = env->GetObjectClass(listener);
  
  // Get method id of the Java Callback member function
  jmethodID mid = env->GetMethodID(cls, "onRecognitionFailure",
                                   "(Landroid/speech/recognition/RecognizerListener$FailureReason;)V");
  env->DeleteLocalRef(cls);
    
  assert(mid != 0);
  
  // We create now a Java ExpectedErrorType
  jclass clsJavaFailureReason = JNIHelper::loadClass(env, listener,
                                "android.speech.recognition.RecognizerListener$FailureReason");
  assert(clsJavaFailureReason != 0);
  
  jfieldID failureReasonMethodId;
  // If we found the class proceed
  switch (reason)
  {
    case NO_MATCH:
      failureReasonMethodId = env->GetStaticFieldID(clsJavaFailureReason, "NO_MATCH",
                              "Landroid/speech/recognition/RecognizerListener$FailureReason;");
      break;
    case SPOKE_TOO_SOON:
      failureReasonMethodId = env->GetStaticFieldID(clsJavaFailureReason, "SPOKE_TOO_SOON",
                              "Landroid/speech/recognition/RecognizerListener$FailureReason;");
      break;
    case BEGINNING_OF_SPEECH_TIMEOUT:
      failureReasonMethodId = env->GetStaticFieldID(clsJavaFailureReason, "BEGINNING_OF_SPEECH_TIMEOUT",
                              "Landroid/speech/recognition/RecognizerListener$FailureReason;");
      break;
    case RECOGNITION_TIMEOUT:
      failureReasonMethodId = env->GetStaticFieldID(clsJavaFailureReason, "RECOGNITION_TIMEOUT",
                              "Landroid/speech/recognition/RecognizerListener$FailureReason;");
      break;
    case TOO_MUCH_SPEECH:
      failureReasonMethodId = env->GetStaticFieldID(clsJavaFailureReason, "TOO_MUCH_SPEECH",
                              "Landroid/speech/recognition/RecognizerListener$FailureReason;");
      break;
    case RECOGNITION_3RD_PARTY_ERROR:
      failureReasonMethodId = env->GetStaticFieldID(clsJavaFailureReason, "RECOGNITION_3RD_PARTY_ERROR",
                              "Landroid/speech/recognition/RecognizerListener$FailureReason;");
      break;
    case SPEECH_SERVER_UNAVAILABLE:
      failureReasonMethodId = env->GetStaticFieldID(clsJavaFailureReason, "SPEECH_SERVER_UNAVAILABLE",
                              "Landroid/speech/recognition/RecognizerListener$FailureReason;");
      break;
    case UNKNOWN:
      failureReasonMethodId = env->GetStaticFieldID(clsJavaFailureReason, "UNKNOWN",
                              "Landroid/speech/recognition/RecognizerListener$FailureReason;");
      break;
    default:
      assert(false);
      return;
  }
  
  // Create the object
  jobject jobjFailureReason = env->GetStaticObjectField(clsJavaFailureReason,
                              failureReasonMethodId);
                              
  // Call JAVA member function of method object
  env->CallVoidMethod(listener, mid, jobjFailureReason);
  if (env->ExceptionCheck())
  {
    jthrowable exc = env->ExceptionOccurred();
    env->ExceptionClear();
    env->DeleteLocalRef(exc);
  }
  env->DeleteLocalRef(jobjFailureReason);        
  env->DeleteLocalRef(clsJavaFailureReason);
}

void CWrapperRecognizerListener::onError(jthrowable exception)
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::onError thru exception");
    
  JNIEnv* env = JNIHelper::getEnv(jvm);
  jclass listenerClass = env->GetObjectClass(listener);
  jmethodID onError = env->GetMethodID(listenerClass, "onError", "(Ljava/lang/Exception;)V");
  env->DeleteLocalRef(listenerClass);
  assert(onError != 0);
  env->CallVoidMethod(listener, onError, exception);
  if (env->ExceptionCheck())
  {
    jthrowable exc = env->ExceptionOccurred();
    env->ExceptionClear();
    env->DeleteLocalRef(exc);
  }
}

void CWrapperRecognizerListener::onError(ReturnCode::Type error)
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::onError");
    
  JNIEnv* env = JNIHelper::getEnv(jvm);
  jclass listenerClass = env->GetObjectClass(listener);
  jmethodID onError = env->GetMethodID(listenerClass, "onError", "(Ljava/lang/Exception;)V");
  env->DeleteLocalRef(listenerClass);
  assert(onError != 0);
  jobject exception = JNIHelper::getJavaException(env, listener, error);
  env->CallVoidMethod(listener, onError, exception);
  if (env->ExceptionCheck())
  {
    jthrowable exc = env->ExceptionOccurred();
    env->ExceptionClear();
    env->DeleteLocalRef(exc);
  }
  env->DeleteLocalRef(exception);
}

void CWrapperRecognizerListener::onRecognitionSuccess(RecognitionResultProxy& result)
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::onRecognitionSuccess");
    
  JNIEnv* env = JNIHelper::getEnv(jvm);
  
  assert(listener != 0);
  jclass listenerClass = env->GetObjectClass(listener);
  jobject recognitionResult = 0;

  // Cast to a valid object (because RecognitionResult is an interface)
    NBestRecognitionResultProxy& nbest = (NBestRecognitionResultProxy&) result;

    // We create now a Java NBestRecognitionResultImpl
    jclass recognitionResultClass = JNIHelper::loadClass(env, listener,
                                    "android.speech.recognition.impl.NBestRecognitionResultImpl");
                                    
    jmethodID recognitionResultConstructor = env->GetMethodID(recognitionResultClass, "<init>", "(J)V");
    assert(recognitionResultConstructor != 0);

    // Create the object
    recognitionResult = env->NewObject(recognitionResultClass,
                    recognitionResultConstructor, (jlong)((void*)&nbest));
    env->DeleteLocalRef(recognitionResultClass);
    assert(recognitionResult != 0);
  
  jmethodID onRecognitionSuccess = env->GetMethodID(listenerClass, "onRecognitionSuccess",
                                   "(Landroid/speech/recognition/RecognitionResult;)V");
  env->DeleteLocalRef(listenerClass);
  assert(onRecognitionSuccess != 0);
  
  env->CallVoidMethod(listener, onRecognitionSuccess, recognitionResult);
 
  if (env->ExceptionCheck())
  {
    jthrowable exc = env->ExceptionOccurred();
    env->ExceptionClear();
    onError(exc);
    env->DeleteLocalRef(exc);
  }
  env->DeleteLocalRef(recognitionResult);
}

jobject CWrapperRecognizerListener::pairsToHashtable(const char** keys, const char** values,
    ARRAY_LIMIT count)
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::pairsToHashtable");
    
  JNIEnv* env = JNIHelper::getEnv(jvm);
  jclass hashtableClass = JNIHelper::loadClass(env, listener, "java.util.Hashtable");
  assert(hashtableClass != 0);
  
  jmethodID hashtableConstructor = env->GetMethodID(hashtableClass, "<init>", "()V");
  assert(hashtableConstructor != 0);
  
  jobject result = env->NewObject(hashtableClass, hashtableConstructor);
  assert(result != 0);
  
  jmethodID hashtablePut = env->GetMethodID(hashtableClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
  env->DeleteLocalRef(hashtableClass);
  assert(hashtablePut != 0);
  
  for (ARRAY_LIMIT i = 0; i < count; ++i)
  {
    jstring key = env->NewStringUTF(keys[i]);
    if (!key)
    {
      env->DeleteLocalRef(result);
      JNIHelper::throwJavaException(env, listener, ReturnCode::OUT_OF_MEMORY);
      return 0;
    }
    jstring value = env->NewStringUTF((values[i] != 0 ? values[i]:""));
    if (!value)
    {
      env->DeleteLocalRef(result);
      env->DeleteLocalRef(key);
      JNIHelper::throwJavaException(env, listener, ReturnCode::OUT_OF_MEMORY);
      return 0;
    }
    
    jobject previousObject = env->CallObjectMethod(result, hashtablePut, key, value);
    if (env->ExceptionCheck())
    {
        jthrowable exc = env->ExceptionOccurred();
        env->ExceptionClear();
        env->DeleteLocalRef(exc);
        env->DeleteLocalRef(previousObject);
        env->DeleteLocalRef(value);
        env->DeleteLocalRef(key);
        return 0;
    }
    env->DeleteLocalRef(previousObject);
    env->DeleteLocalRef(value);
    env->DeleteLocalRef(key);
  }
  return result;
}

jobject CWrapperRecognizerListener::keysToVector(const char** keys, ARRAY_LIMIT count)
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::onParametersGetError");
    
  JNIEnv* env = JNIHelper::getEnv(jvm);
  jclass vectorClass = JNIHelper::loadClass(env, listener, "java.util.Vector");
  assert(vectorClass != 0);
  
  jmethodID vectorConstructor = env->GetMethodID(vectorClass, "<init>", "()V");
  assert(vectorConstructor != 0);
  
  jobject result = env->NewObject(vectorClass, vectorConstructor);
  assert(result != 0);
  
  jmethodID vectorAdd = env->GetMethodID(vectorClass, "add", "(Ljava/lang/Object;)Z");
  env->DeleteLocalRef(vectorClass);
  assert(vectorAdd != 0);
  
  for (ARRAY_LIMIT i = 0; i < count; ++i)
  {
    jstring key = env->NewStringUTF(keys[i]);
    if (!key)
    {
      env->DeleteLocalRef(result);
      JNIHelper::throwJavaException(env, listener, ReturnCode::OUT_OF_MEMORY);
      return 0;
    }
    
    env->CallBooleanMethod(result, vectorAdd, key);
    if (env->ExceptionCheck())
    {
        jthrowable exc = env->ExceptionOccurred();
        env->ExceptionClear();
        env->DeleteLocalRef(exc);
        env->DeleteLocalRef(key);
        return 0;
    }
    env->DeleteLocalRef(key);
  }
  return result;
}

void CWrapperRecognizerListener::onParametersGetError(const char** keys, ARRAY_LIMIT count,
    ReturnCode::Type returnCode)
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::onParametersGetError");
    
  JNIEnv* env = JNIHelper::getEnv(jvm);
  jclass clazz = env->GetObjectClass(listener);
  
  jmethodID onParametersGetError = env->GetMethodID(clazz, "onParametersGetError",
                                   "(Ljava/util/Vector;Ljava/lang/Exception;)V");
  env->DeleteLocalRef(clazz);
  assert(onParametersGetError != 0);
  
  jobject parameters = keysToVector(keys, count);
  assert(parameters != 0);
  
  jobject exception = JNIHelper::getJavaException(env, listener, returnCode);
  assert(exception != 0);
  
  env->CallVoidMethod(listener, onParametersGetError, parameters, exception);
  if (env->ExceptionCheck())
  {
    jthrowable exc = env->ExceptionOccurred();
    env->ExceptionClear();
    env->DeleteLocalRef(exc);
  }
  env->DeleteLocalRef(exception);
  env->DeleteLocalRef(parameters);
}

void CWrapperRecognizerListener::onParametersSetError(const char** keys, const char** values,
    ARRAY_LIMIT count, ReturnCode::Type returnCode)
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::onParametersSetError");
    
  JNIEnv* env = JNIHelper::getEnv(jvm);
  jclass clazz = env->GetObjectClass(listener);
  
  jmethodID onParametersSetError = env->GetMethodID(clazz, "onParametersSetError",
                                   "(Ljava/util/Hashtable;Ljava/lang/Exception;)V");
  env->DeleteLocalRef(clazz);
  assert(onParametersSetError != 0);
  
  jobject parameters = pairsToHashtable(keys, values, count);
  assert(parameters != 0);
  
  jobject exception = JNIHelper::getJavaException(env, listener, returnCode);
  assert(exception != 0);
  
  env->CallVoidMethod(listener, onParametersSetError, parameters, exception);
  if (env->ExceptionCheck())
  {
    jthrowable exc = env->ExceptionOccurred();
    env->ExceptionClear();
    env->DeleteLocalRef(exc);
  }
  env->DeleteLocalRef(exception);
  env->DeleteLocalRef(parameters);
}

void CWrapperRecognizerListener::onParametersSet(const char** keys, const char** values,
    ARRAY_LIMIT count)
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::onParametersSet");
    
  JNIEnv* env = JNIHelper::getEnv(jvm);
  
  jobject parameters = pairsToHashtable(keys, values, count);
  assert(parameters != 0);
  
  jclass clazz = env->GetObjectClass(listener);
  jmethodID methodID = env->GetMethodID(clazz, "onParametersSet", "(Ljava/util/Hashtable;)V");
  env->DeleteLocalRef(clazz);
  env->CallVoidMethod(listener, methodID, parameters);
  if (env->ExceptionCheck())
  {
    jthrowable exc = env->ExceptionOccurred();
    env->ExceptionClear();
    onError(exc);
    env->DeleteLocalRef(exc);
  }
  env->DeleteLocalRef(parameters);
}

void CWrapperRecognizerListener::onParametersGet(const char** keys, const char** values,
    ARRAY_LIMIT count)
{
  UAPI_FN_SCOPE("CWrapperRecognizerListener::onParametersGet");
    
  JNIEnv* env = JNIHelper::getEnv(jvm);
  
  jobject parameters = pairsToHashtable(keys, values, count);
  assert(parameters != 0);
  
  jclass clazz = env->GetObjectClass(listener);
  jmethodID methodID = env->GetMethodID(clazz, "onParametersGet", "(Ljava/util/Hashtable;)V");
  env->DeleteLocalRef(clazz);
  env->CallVoidMethod(listener, methodID, parameters);
  if (env->ExceptionCheck())
  {
    jthrowable exc = env->ExceptionOccurred();
    env->ExceptionClear();
    onError(exc);
    env->DeleteLocalRef(exc);
  }
  env->DeleteLocalRef(parameters);
}
