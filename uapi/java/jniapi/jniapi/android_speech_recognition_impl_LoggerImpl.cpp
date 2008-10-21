/*---------------------------------------------------------------------------*
 *  android_speech_recognition_impl_LoggerImpl.cpp                                                 *
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
 * android_speech_recognition_impl_LoggerImpl.cpp
 */
#include "jniapi.h"
#include "android_speech_recognition_impl_LoggerImpl.h"
#include "Logger.h"
#include "JNIHelper.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;


JNIEXPORT void JNICALL Java_android_speech_recognition_impl_LoggerImpl_deleteNativeObject
(JNIEnv* ,
 jobject ,
 jlong nativeObj)
{
#ifdef UAPI_LOGGING_ENABLED

  // Allocating a LoggerProxy at this point will result in the parent logger getting destroyed
  // before its children. As such, we don't create a parent logger.
  delete(LoggerProxy*) nativeObj;
#endif
}


JNIEXPORT jlong JNICALL Java_android_speech_recognition_impl_LoggerImpl_initNativeObject(JNIEnv* env,
    jobject logger)
{
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_LoggerImpl_initNativeObject");
  
#ifdef UAPI_LOGGING_ENABLED
  ReturnCode::Type returnCode;
  LoggerProxy nativeLogger = Logger::getInstance(returnCode);
  if (returnCode)
  {
    JNIHelper::throwJavaException(env, logger, returnCode);
    return 0;
  }
  LoggerProxy* result = new LoggerProxy(nativeLogger);
  if (!result || !*result)
  {
    delete result;
    returnCode = ReturnCode::OUT_OF_MEMORY;
    JNIHelper::throwJavaException(env, logger, returnCode);
    return 0;
  }
  return (jlong) result;
#else
  return 0;
#endif
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_LoggerImpl_setLoggingLevelProxy
(JNIEnv* env,
 jobject logger,
 jlong nativeObj,
 jobject loglevel)
{
#ifdef UAPI_LOGGING_ENABLED
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_LoggerImpl_SetLoggingLevelProxy");
  
  LoggerProxy* nativeLoggerPointer = (LoggerProxy*) nativeObj;
  if (!nativeLoggerPointer)
  {
    JNIHelper::throwJavaException(env, logger, ReturnCode::INVALID_STATE);
    return;
  }
  LoggerProxy& nativeLogger = *nativeLoggerPointer;
  
  Logger::LogLevel nativeLogLevel = JNIHelper::toNativeLogLevel(env, loglevel);
  if (env->ExceptionCheck())
    return;
  nativeLogger->setLoggingLevel(nativeLogLevel, returnCode);
  if (returnCode)
  {
    JNIHelper::throwJavaException(env, logger, returnCode);
    return;
  }
#endif
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_LoggerImpl_setPathProxy
(JNIEnv *env,
 jobject logger,
 jlong nativeObj,
 jstring path)
{
#ifdef UAPI_LOGGING_ENABLED
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_LoggerImpl_setPathProxy");
  
  LoggerProxy* nativeLoggerPointer = (LoggerProxy*) nativeObj;
  if (!nativeLoggerPointer)
  {
    JNIHelper::throwJavaException(env, logger, ReturnCode::INVALID_STATE);
    return;
  }
  LoggerProxy& nativeLogger = *nativeLoggerPointer;
  
  const char* nativePath = env->GetStringUTFChars(path, 0);
  nativeLogger->setPath(nativePath, returnCode);
  env->ReleaseStringUTFChars(path, nativePath);
  
  if (returnCode)
    JNIHelper::throwJavaException(env, logger, returnCode);
#endif
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_LoggerImpl_errorProxy
(JNIEnv* env,
 jobject logger,
 jlong nativeObj,
 jstring message)
{
#ifdef UAPI_LOGGING_ENABLED
  LoggerProxy* nativeLoggerPointer = (LoggerProxy*) nativeObj;
  if (!nativeLoggerPointer)
  {
    JNIHelper::throwJavaException(env, logger, ReturnCode::INVALID_STATE);
    return;
  }
  LoggerProxy& nativeLogger = *nativeLoggerPointer;
  
  const char* nativeMessage = env->GetStringUTFChars(message, 0);
  nativeLogger->error("Java", "%s\n", nativeMessage);
  env->ReleaseStringUTFChars(message, nativeMessage);
#endif
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_LoggerImpl_warnProxy
(JNIEnv *env,
 jobject logger,
 jlong nativeObj,
 jstring message)
{
#ifdef UAPI_LOGGING_ENABLED
  LoggerProxy* nativeLoggerPointer = (LoggerProxy*) nativeObj;
  if (!nativeLoggerPointer)
  {
    JNIHelper::throwJavaException(env, logger, ReturnCode::INVALID_STATE);
    return;
  }
  LoggerProxy& nativeLogger = *nativeLoggerPointer;
  
  const char* nativeMessage = env->GetStringUTFChars(message, 0);
  nativeLogger->warn("Java", "%s\n", nativeMessage);
  env->ReleaseStringUTFChars(message, nativeMessage);
#endif
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_LoggerImpl_infoProxy
(JNIEnv *env,
 jobject logger,
 jlong nativeObj,
 jstring message)
{
#ifdef UAPI_LOGGING_ENABLED
  LoggerProxy* nativeLoggerPointer = (LoggerProxy*) nativeObj;
  if (!nativeLoggerPointer)
  {
    JNIHelper::throwJavaException(env, logger, ReturnCode::INVALID_STATE);
    return;
  }
  LoggerProxy& nativeLogger = *nativeLoggerPointer;
  
  const char* nativeMessage = env->GetStringUTFChars(message, 0);
  nativeLogger->info("Java", "%s\n", nativeMessage);
  env->ReleaseStringUTFChars(message, nativeMessage);
#endif
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_LoggerImpl_traceProxy
(JNIEnv *env,
 jobject logger,
 jlong nativeObj,
 jstring message)
{
#ifdef UAPI_LOGGING_ENABLED
  LoggerProxy* nativeLoggerPointer = (LoggerProxy*) nativeObj;
  if (!nativeLoggerPointer)
  {
    JNIHelper::throwJavaException(env, logger, ReturnCode::INVALID_STATE);
    return;
  }
  LoggerProxy& nativeLogger = *nativeLoggerPointer;
  
  const char* nativeMessage = env->GetStringUTFChars(message, 0);
  nativeLogger->trace("Java", "%s\n", nativeMessage);
  env->ReleaseStringUTFChars(message, nativeMessage);
#endif
}
