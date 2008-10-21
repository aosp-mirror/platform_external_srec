/*---------------------------------------------------------------------------*
 *  JNIHelper.cpp                                                            *
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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "JNIHelper.h"
#include "System.h"
#include "jniapi.h"
#include "Mutex.h"
#include "CWrapperEmbeddedGrammarListener.h"
#include "CWrapperSrecGrammarListener.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::utilities;
using namespace android::speech::recognition::jni;

#ifdef ENABLE_JNI_KEEP_ALIVE
JVMKeepAlive::ComponentInitializer::ComponentInitializer()
{
  mutex = Mutex::create(false, returnCode);
  if (returnCode)
  {
    fprintf(stderr, "Could not create JNIKeepAlive::mutex\n");
    return;
  }
}

JVMKeepAlive::ComponentInitializer::~ComponentInitializer()
{
  delete mutex;
  returnCode = ReturnCode::UNKNOWN;
}
#endif

Codec::Type JNIHelper::toNativeCodec(JNIEnv* env, jobject codec)
{
  UAPI_FN_SCOPE("JNIHelper::toNativeCodec");
  
  const char codecClassName[] = "Landroid/speech/recognition/Codec;";
  jclass codecClass = loadClass(env, codec, "android.speech.recognition.Codec");
  
  jfieldID PCM_16BIT_8K_field = env->GetStaticFieldID(codecClass, "PCM_16BIT_8K", codecClassName);
  jobject PCM_16BIT_8K = env->GetStaticObjectField(codecClass, PCM_16BIT_8K_field);
  
  jfieldID PCM_16BIT_11K_field = env->GetStaticFieldID(codecClass, "PCM_16BIT_11K", codecClassName);
  jobject PCM_16BIT_11K = env->GetStaticObjectField(codecClass, PCM_16BIT_11K_field);
  
  jfieldID PCM_16BIT_22K_field = env->GetStaticFieldID(codecClass, "PCM_16BIT_22K", codecClassName);
  jobject PCM_16BIT_22K = env->GetStaticObjectField(codecClass, PCM_16BIT_22K_field);

  jfieldID ULAW_8BIT_8K_field = env->GetStaticFieldID(codecClass, "ULAW_8BIT_8K", codecClassName);
  jobject ULAW_8BIT_8K = env->GetStaticObjectField(codecClass, ULAW_8BIT_8K_field);
  
  Codec::Type nativeCodec;
  if (env->IsSameObject(codec, PCM_16BIT_8K))
    nativeCodec = Codec::PCM_16BIT_8K;
  else if (env->IsSameObject(codec, PCM_16BIT_11K))
    nativeCodec = Codec::PCM_16BIT_11K;
  else if (env->IsSameObject(codec, PCM_16BIT_22K))
    nativeCodec = Codec::PCM_16BIT_22K;
  else if (env->IsSameObject(codec, ULAW_8BIT_8K))
    nativeCodec = Codec::ULAW_8BIT_8K;
  else
  {
    env->DeleteLocalRef(PCM_16BIT_8K);
    env->DeleteLocalRef(PCM_16BIT_11K);
    env->DeleteLocalRef(PCM_16BIT_22K);
    env->DeleteLocalRef(ULAW_8BIT_8K);
    char message[256] = "Unknown codec: ";
    jmethodID toStringMethod = env->GetMethodID(codecClass, "toString", "()Ljava/lang/String;");
    env->DeleteLocalRef(codecClass);
    assert(toStringMethod != 0);
    jstring toStringValue = (jstring) env->CallObjectMethod(codec, toStringMethod);
    if (env->ExceptionCheck())
    {
        env->ExceptionClear();
      // Return value is undefined because we're throwing an exception
      return Codec::PCM_16BIT_8K;
    }
    const char* codecString = env->GetStringUTFChars(toStringValue, 0);
    strcat(message, codecString);
    env->ReleaseStringUTFChars(toStringValue, codecString);
    env->DeleteLocalRef(toStringValue);
    
    throwRuntimeJavaException(env, codec, message);
    // Return value is undefined because we're throwing an exception
    return Codec::PCM_16BIT_8K;
  }
  env->DeleteLocalRef(codecClass);
  env->DeleteLocalRef(PCM_16BIT_8K);
  env->DeleteLocalRef(PCM_16BIT_11K);
  env->DeleteLocalRef(PCM_16BIT_22K);
  env->DeleteLocalRef(ULAW_8BIT_8K);
  return nativeCodec;
}


MediaFileReader::ReadingMode JNIHelper::toNativeMediaFileReaderMode(JNIEnv* env, jobject mode)
{
  UAPI_FN_SCOPE("JNIHelper::toNativeMediaFileReaderMode");
  
  const char modeClassName[] = "Landroid/speech/recognition/MediaFileReader$Mode;";
  jclass modeClass = loadClass(env, mode, "android.speech.recognition.MediaFileReader$Mode");
  assert(modeClass != 0);
  
  jfieldID REAL_TIME_field = env->GetStaticFieldID(modeClass, "REAL_TIME", modeClassName);
  assert(REAL_TIME_field != 0);
  jobject REAL_TIME = env->GetStaticObjectField(modeClass, REAL_TIME_field);
  
  jfieldID ALL_AT_ONCE_field = env->GetStaticFieldID(modeClass, "ALL_AT_ONCE", modeClassName);
  assert(ALL_AT_ONCE_field != 0);
  jobject ALL_AT_ONCE = env->GetStaticObjectField(modeClass, ALL_AT_ONCE_field);
  
  MediaFileReader::ReadingMode nativeMode;
  if (env->IsSameObject(mode, REAL_TIME))
    nativeMode = MediaFileReader::REAL_TIME;
  else if (env->IsSameObject(mode, ALL_AT_ONCE))
    nativeMode = MediaFileReader::ALL_AT_ONCE;
  else
  {
    env->DeleteLocalRef(REAL_TIME);
    env->DeleteLocalRef(ALL_AT_ONCE);
    char message[256] = "Unknown reading mode: ";
    jmethodID toStringMethod = env->GetMethodID(modeClass, "toString", "()Ljava/lang/String;");
    env->DeleteLocalRef(modeClass);
    assert(toStringMethod != 0);
    jstring toStringValue = (jstring) env->CallObjectMethod(mode, toStringMethod);
    if (env->ExceptionCheck())
    {
      env->ExceptionClear();
      // Return value is undefined because we're throwing an exception
      return MediaFileReader::REAL_TIME;
    }
    const char* modeString = env->GetStringUTFChars(toStringValue, 0);
    strcat(message, modeString);
    env->ReleaseStringUTFChars(toStringValue, modeString);
    env->DeleteLocalRef(toStringValue);
    
    throwRuntimeJavaException(env, mode, message);
    
    // Return value is undefined because we're throwing an exception
    return MediaFileReader::REAL_TIME;
  }
  env->DeleteLocalRef(modeClass);
  env->DeleteLocalRef(REAL_TIME);
  env->DeleteLocalRef(ALL_AT_ONCE);
  return nativeMode;
}

Logger::LogLevel JNIHelper::toNativeLogLevel(JNIEnv* env, jobject loglevel)
{
  UAPI_FN_SCOPE("JNIHelper::toNativeLogLevel");
  
  const char loggerClassName[] = "Landroid/speech/recognition/Logger$LogLevel;";
  jclass loglevelClass = loadClass(env, loglevel, "android.speech.recognition.Logger$LogLevel");
  assert(loglevelClass != 0);
  
  jfieldID LEVEL_NONE_field = env->GetStaticFieldID(loglevelClass, "LEVEL_NONE", loggerClassName);
  assert(LEVEL_NONE_field != 0);
  jobject LEVEL_NONE = env->GetStaticObjectField(loglevelClass, LEVEL_NONE_field);
  
  jfieldID LEVEL_ERROR_field = env->GetStaticFieldID(loglevelClass, "LEVEL_ERROR", loggerClassName);
  assert(LEVEL_ERROR_field != 0);
  jobject LEVEL_ERROR = env->GetStaticObjectField(loglevelClass, LEVEL_ERROR_field);
  
  jfieldID LEVEL_WARN_field = env->GetStaticFieldID(loglevelClass, "LEVEL_WARN", loggerClassName);
  assert(LEVEL_WARN_field != 0);
  jobject LEVEL_WARN = env->GetStaticObjectField(loglevelClass, LEVEL_WARN_field);
  
  jfieldID LEVEL_INFO_field = env->GetStaticFieldID(loglevelClass, "LEVEL_INFO", loggerClassName);
  assert(LEVEL_INFO_field != 0);
  jobject LEVEL_INFO = env->GetStaticObjectField(loglevelClass, LEVEL_INFO_field);
  
  jfieldID LEVEL_TRACE_field = env->GetStaticFieldID(loglevelClass, "LEVEL_TRACE", loggerClassName);
  assert(LEVEL_TRACE_field != 0);
  jobject LEVEL_TRACE = env->GetStaticObjectField(loglevelClass, LEVEL_TRACE_field);
  
  
  Logger::LogLevel nativeLogLevel;
  if (env->IsSameObject(loglevel, LEVEL_NONE))
    nativeLogLevel = Logger::LEVEL_NONE;
  else if (env->IsSameObject(loglevel, LEVEL_ERROR))
    nativeLogLevel = Logger::LEVEL_ERROR;
  else if (env->IsSameObject(loglevel, LEVEL_WARN))
    nativeLogLevel = Logger::LEVEL_WARN;
  else if (env->IsSameObject(loglevel, LEVEL_INFO))
    nativeLogLevel = Logger::LEVEL_INFO;
  else if (env->IsSameObject(loglevel, LEVEL_TRACE))
    nativeLogLevel = Logger::LEVEL_TRACE;
  else
  {
    env->DeleteLocalRef(LEVEL_NONE);
    env->DeleteLocalRef(LEVEL_ERROR);
    env->DeleteLocalRef(LEVEL_WARN);
    env->DeleteLocalRef(LEVEL_INFO);
    env->DeleteLocalRef(LEVEL_TRACE);
    char message[256] = "Unknown log level: ";
    jmethodID toStringMethod = env->GetMethodID(loglevelClass, "toString", "()Ljava/lang/String;");
    env->DeleteLocalRef(loglevelClass);
    assert(toStringMethod != 0);
    jstring toStringValue = (jstring) env->CallObjectMethod(loglevel, toStringMethod);
    if (env->ExceptionCheck())
    {
      env->ExceptionClear();
      // Return value is undefined because we're throwing an exception
      return Logger::LEVEL_NONE;
    }
    const char* modeString = env->GetStringUTFChars(toStringValue, 0);
    strcat(message, modeString);
    env->ReleaseStringUTFChars(toStringValue, modeString);
    env->DeleteLocalRef(toStringValue);
    
    throwRuntimeJavaException(env, loglevel, message);
    
    // Return value is undefined because we're throwing an exception
    return Logger::LEVEL_NONE;
  }
  env->DeleteLocalRef(loglevelClass);
  env->DeleteLocalRef(LEVEL_NONE);
  env->DeleteLocalRef(LEVEL_ERROR);
  env->DeleteLocalRef(LEVEL_WARN);
  env->DeleteLocalRef(LEVEL_INFO);
  env->DeleteLocalRef(LEVEL_TRACE);
  return nativeLogLevel;
}

JavaVM* JNIHelper::getJVM(JNIEnv* env, jobject reference)
{
  JavaVM* result;
  if (env->GetJavaVM(&result) < 0)
  {
    throwRuntimeJavaException(env, reference, "GetJavaVM() failed");
    return 0;
  }
  return result;
}

GrammarListenerProxy JNIHelper::getNativeGrammarListener(JNIEnv* env, jobject listener)
{
  ReturnCode::Type returnCode = ReturnCode::UNKNOWN;
  UAPI_FN_SCOPE("getNativeGrammarListener");
    
  JavaVM* jvm = getJVM(env, listener);
  if (env->ExceptionCheck())
  {
      env->ExceptionClear();
      return GrammarListenerProxy();
  } 
  jclass EmbeddedGrammarListenerClass = loadClass(env, listener,
                                        "android.speech.recognition.EmbeddedGrammarListener");
  assert(EmbeddedGrammarListenerClass != 0);
  jclass SrecGrammarListenerClass = loadClass(env, listener,
                                        "android.speech.recognition.SrecGrammarListener");
  assert(SrecGrammarListenerClass != 0);

  if (env->IsInstanceOf(listener, SrecGrammarListenerClass))
  { 
    env->DeleteLocalRef(SrecGrammarListenerClass);
    CWrapperSrecGrammarListener* temp = new CWrapperSrecGrammarListener(jvm, listener, returnCode);
    if (!temp)
    {
      throwJavaException(env, listener, ReturnCode::OUT_OF_MEMORY);
      return GrammarListenerProxy();
    }
    else if (returnCode)
    {
      throwJavaException(env, listener, returnCode);
      return GrammarListenerProxy();
    }
    else if (env->ExceptionCheck())
    {
      env->ExceptionClear();
      delete temp;
      return GrammarListenerProxy();
    }
    SrecGrammarListenerProxy result = SrecGrammarListenerProxy(temp);
    if (!result)
    {
      throwJavaException(env, listener, ReturnCode::OUT_OF_MEMORY);
      return GrammarListenerProxy();
    }
    return result;
  }
  else if (env->IsInstanceOf(listener, EmbeddedGrammarListenerClass))
  {
    env->DeleteLocalRef(EmbeddedGrammarListenerClass);
    CWrapperEmbeddedGrammarListener* temp = new CWrapperEmbeddedGrammarListener(jvm, listener, returnCode);
    if (!temp)
    {
      throwJavaException(env, listener, ReturnCode::OUT_OF_MEMORY);
      return GrammarListenerProxy();
    }
    else if (returnCode)
    {
      throwJavaException(env, listener, returnCode);
      return GrammarListenerProxy();
    }
    else if (env->ExceptionCheck())
    {
      env->ExceptionClear();
      delete temp;
      return GrammarListenerProxy();
    }
    EmbeddedGrammarListenerProxy result = EmbeddedGrammarListenerProxy(temp);
    if (!result)
    {
      throwJavaException(env, listener, ReturnCode::OUT_OF_MEMORY);
      return GrammarListenerProxy();
    }
    return result;
  }
  else
  {
    env->DeleteLocalRef(EmbeddedGrammarListenerClass);
    CWrapperGrammarListener* temp = new CWrapperGrammarListener(jvm, listener, returnCode);
    if (!temp)
    {
      throwJavaException(env, listener, ReturnCode::OUT_OF_MEMORY);
      return GrammarListenerProxy();
    }
    else if (returnCode)
    {
      throwJavaException(env, listener, returnCode);
      return GrammarListenerProxy();
    }
    else if (env->ExceptionCheck())
    {
      env->ExceptionClear();
      delete temp;
      return GrammarListenerProxy();
    }
    GrammarListenerProxy result = GrammarListenerProxy(temp);
    if (!result)
    {
      throwJavaException(env, listener, ReturnCode::OUT_OF_MEMORY);
      return GrammarListenerProxy();
    }
    return result;
  }
}

jthrowable JNIHelper::getJavaException(JNIEnv* env, jobject obj, ReturnCode::Type returnCode)
{
  UAPI_FN_SCOPE("getJavaException");
    
  jclass clazz;
  switch (returnCode)
  {
    case ReturnCode::END_OF_STREAM:
      clazz = loadClass(env, obj, "java.io.EOFException");
      break;
    case ReturnCode::UNSUPPORTED_CODEC:
    case ReturnCode::ILLEGAL_ARGUMENT:
      clazz = loadClass(env, obj, "java.lang.IllegalArgumentException");
      break;
    case ReturnCode::INVALID_STATE:
      clazz = loadClass(env, obj, "java.lang.IllegalStateException");
      break;
    case ReturnCode::NOT_SUPPORTED:
      clazz = loadClass(env, obj, "java.lang.UnsupportedOperationException");
      break;
    case ReturnCode::OUT_OF_MEMORY:
      clazz = loadClass(env, obj, "java.lang.OutOfMemoryError");
      break;
    case ReturnCode::FILE_NOT_FOUND:
    case ReturnCode::PENDING_DATA:
    case ReturnCode::SOCKET_IO_ERROR:
    case ReturnCode::OPEN_ERROR:
      clazz = loadClass(env, obj, "java.io.FileNotFoundException");
      break;
    case ReturnCode::READ_ERROR:
    case ReturnCode::WRITE_ERROR:
    case ReturnCode::SOCKET_CLOSED:
      clazz = loadClass(env, obj, "java.io.IOException");
      break;
    case ReturnCode::MAXIMUM_BOUND:
    case ReturnCode::UNKNOWN:
    case ReturnCode::HOMONYM_COLLISION:
      clazz = loadClass(env, obj, "java.lang.Exception");
      break;
    case ReturnCode::TIMEOUT:
      clazz = loadClass(env, obj, "java.util.concurrent.TimeoutException");
      break;
    case ReturnCode::UNKNOWN_MODULE:
    case ReturnCode::UNKNOWN_SYMBOL:
      clazz = loadClass(env, obj, "java.lang.UnsatisfiedLinkError");
      break;
    case ReturnCode::ARRAY_INDEX_OUT_OF_BOUNDS:
      clazz = loadClass(env, obj, "java.lang.ArrayIndexOutOfBoundsException");
      break;
    case ReturnCode::THREAD_ERROR:
    case ReturnCode::ALREADY_LOCKED:
      clazz = loadClass(env, obj, "java.lang.IllegalThreadStateException");
      break;
      // User-defined exceptions
    case ReturnCode::AUDIO_ALREADY_IN_USE:
      clazz = loadClass(env, obj, "android.speech.recognition.AudioAlreadyInUseException");
      break;
    case ReturnCode::AUDIO_DRIVER_ERROR:
      clazz = loadClass(env, obj, "android.speech.recognition.AudioDriverErrorException");
      break;
    case ReturnCode::GRAMMAR_SLOT_FULL:
      clazz = loadClass(env, obj, "android.speech.recognition.GrammarOverflowException");
      break;
    case ReturnCode::OVERFLOW_ERROR:
      clazz = loadClass(env, obj, "java.nio.BufferOverflowException");
      break;
    case ReturnCode::UNDERFLOW_ERROR:
      clazz = loadClass(env, obj, "java.nio.BufferUnderflowException");
      break;
    case ReturnCode::PARSE_ERROR:
      clazz = loadClass(env, obj, "android.speech.recognition.ParseErrorException");
      break;
   
    case ReturnCode::INVALID_URL:
      clazz = loadClass(env, obj, "android.speech.recognition.InvalidURLException");
      break;
    case ReturnCode::GRAMMAR_LOAD_FAILURE:
    case ReturnCode::GRAMMAR_NOT_DEFINED:
      clazz = loadClass(env, obj, "android.speech.recognition.GrammarErrorException");
      break;
    case ReturnCode::INVALID_PARAMETER_NAME:
    case ReturnCode::INVALID_PARAMETER_VALUE:
      clazz = loadClass(env, obj, "android.speech.recognition.ParameterErrorException");
      break;
    default:
      clazz = loadClass(env, obj, "java.lang.Exception");
  }
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;)V");
  if (constructor != 0)
  {
    jstring jrc = env->NewStringUTF(ReturnCode::toString(returnCode));
    jobject result = env->NewObject(clazz, constructor, jrc);
    env->DeleteLocalRef(jrc);
    env->DeleteLocalRef(clazz);
    return (jthrowable) result;
  }
  else
  {
    // Try the default constructor instead
    constructor = env->GetMethodID(clazz, "<init>", "()V");
    assert(constructor != 0);
    jobject result = env->NewObject(clazz, constructor);
    env->DeleteLocalRef(clazz);
    return (jthrowable) result;
  }
}

void JNIHelper::throwRuntimeJavaException(JNIEnv* env, jobject obj, const char* msg)
{
  jclass clazz = loadClass(env, obj, "java.lang.RuntimeException");
  if (clazz != 0)
    env->ThrowNew(clazz, msg);

  env->DeleteLocalRef(clazz);
}

void JNIHelper::throwJavaException(JNIEnv* env, jobject obj, ReturnCode::Type returnCode)
{
  UAPI_FN_SCOPE("JNIHelper::throwJavaException");
  jobject result = getJavaException(env, obj, returnCode);
  if (result == 0)
  {
    jclass clazz = loadClass(env, obj, "java.lang.Exception");
    if (clazz == 0)
    {
      // Give up
      env->DeleteLocalRef(result);
      returnCode = ReturnCode::OUT_OF_MEMORY;
      return;
    }
    jmethodID constructor = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;)V");
    assert(constructor != 0);
    jstring jrc = env->NewStringUTF(ReturnCode::toString(returnCode));
    result = (jobject) env->NewObject(clazz, constructor,  jrc);
    env->DeleteLocalRef(jrc);
    env->DeleteLocalRef(clazz);
  }
  env->Throw((jthrowable)result);
  env->DeleteLocalRef(result);
}

JNIEnv* JNIHelper::getEnv(JavaVM* jvm)
{
  JNIEnv* result;
  JavaVMAttachArgs args = {0, 0, 0};
  args.version = JNI_VERSION_1_4;
  args.name = "JNI Thread";
#if defined(NO_ATTACHCURRENTTHREAD_CAST_VOIDPP)
  if (jvm->AttachCurrentThreadAsDaemon(&result, &args))
    return 0;
#else
  if (jvm->AttachCurrentThreadAsDaemon((void**) &result, &args))
    return 0;
#endif
  return result;
}

void JNIHelper::attachJVM(ReturnCode::Type& returnCode)
{
#ifdef ENABLE_JNI_KEEP_ALIVE
  UAPI_FN_NAME("JNIHelper::attachJVM");
    
  JVMKeepAlive* jvm = JVMKeepAlive::getInstance(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"JVMKeepAlive::getInstance() failed\n");
    return;
  }
  jvm->attach(returnCode);
#else
  returnCode = ReturnCode::SUCCESS;
#endif
}

void JNIHelper::detachJVM(ReturnCode::Type& returnCode)
{
#ifdef ENABLE_JNI_KEEP_ALIVE
  UAPI_FN_NAME("JNIHelper::detachJVM");
    
    
  JVMKeepAlive* jvm = JVMKeepAlive::getInstance(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"JVMKeepAlive::getInstance() failed\n");
    return;
  }
  jvm->detach(returnCode);
#else
  returnCode = ReturnCode::SUCCESS;
#endif
}

jclass JNIHelper::loadClass(JNIEnv* env, jobject object, const char* name)
{
  UAPI_FN_NAME("JNIHelper::loadClass");
    
  jobject classLoader = 0;
  if (object != 0)
  {
    // object.getClass()
    UAPI_INFO(fn,"GetObjectClass(object == %p)\n", object);
    jclass objectClass = env->GetObjectClass(object);
    jmethodID method = env->GetMethodID(objectClass, "getClass", "()Ljava/lang/Class;");
    env->DeleteLocalRef(objectClass);
    UAPI_INFO(fn,"method == %p\n", method);
    assert(method != 0);
    
    // Object of type Class
    jobject classObject = env->CallObjectMethod(object, method);
    assert(!env->ExceptionCheck() && classObject != 0);
    
    // Class.class
    UAPI_INFO(fn,"GetObjectClass(classObject == %p)\n", classObject);
    jclass classClass = env->GetObjectClass(classObject);
    UAPI_INFO(fn,"classClass == %p)\n", classClass);
    // Get the ClassLoader
    jmethodID getClassLoader = env->GetMethodID(classClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
    env->DeleteLocalRef(classClass);
    UAPI_INFO(fn,"getClassLoader == %p)\n", getClassLoader);
    assert(getClassLoader != 0);
    
    // object.getClass().getClassLoader()
    classLoader = env->CallObjectMethod(classObject, getClassLoader);
    if (env->ExceptionCheck())
    {
      env->ExceptionDescribe();
      env->ExceptionClear();
      env->DeleteLocalRef(classObject);
      return 0;
    }
    env->DeleteLocalRef(classObject);
  }
  if (classLoader == 0)
  {
    jclass classLoaderClass = env->FindClass("java/lang/ClassLoader");
    jmethodID getSystemClassLoader = env->GetStaticMethodID(classLoaderClass, "getSystemClassLoader",
                                     "()Ljava/lang/ClassLoader;");
    assert(getSystemClassLoader != 0);
    //env->DeleteLocalRef(classLoader); // delete local ref before replacing with another classLoader
    classLoader = env->CallStaticObjectMethod(classLoaderClass, getSystemClassLoader);
    assert(!env->ExceptionCheck() && classLoader != 0);
    env->DeleteLocalRef(classLoaderClass);
  }
  UAPI_INFO(fn,"GetObjectClass(classLoader == %p)\n", classLoader);
  jclass classLoaderClass = env->GetObjectClass(classLoader);
  UAPI_INFO(fn,"classLoaderClass == %p)\n", classLoaderClass);
  
  // Load the class
  jmethodID loadClass = env->GetMethodID(classLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
  env->DeleteLocalRef(classLoaderClass);
  UAPI_INFO(fn,"loadClass == %p)\n", loadClass);
  assert(loadClass != 0);
  
  jstring jname = env->NewStringUTF(name);
  jobject result = env->CallObjectMethod(classLoader, loadClass, jname);
  env->DeleteLocalRef(jname);
  env->DeleteLocalRef(classLoader);
  
  return (jclass) result;
}


NativeThreadWatcher* NativeThreadWatcher::instance;

NativeThreadWatcher* NativeThreadWatcher::getInstance(ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("NativeThreadWatcher::getInstance");
    
  //don't have to protect instance, NativeThreadWatcher::getInstance() will
  //be called only once when System_init is called.
  if (instance == 0)
  {
    instance = new NativeThreadWatcher();
    if (instance == 0)
    {
      UAPI_ERROR(fn,"Could not create JVMKeepAlive, OUT_OF_MEMORY\n");
      returnCode = ReturnCode::OUT_OF_MEMORY;
      return 0;
    }
  }
  returnCode = ReturnCode::SUCCESS;
  return instance;
}

NativeThreadWatcher::NativeThreadWatcher():
    jvm(0)
{}

void NativeThreadWatcher::onThreadStarted()
{
  UAPI_FN_NAME("NativeThreadWatcher::onThreadStarted");
    
  if (jvm == 0)
  {
    UAPI_ERROR(fn,"jvm is null\n");
    return;
  }
  
  //Attach JVM to current thread
  JNIEnv* env;
  JavaVMAttachArgs args = {0, 0, 0};
  args.version = JNI_VERSION_1_4;
  args.name = "WorkerQueue";
#if defined(NO_ATTACHCURRENTTHREAD_CAST_VOIDPP)
  if (jvm->AttachCurrentThreadAsDaemon(&env, &args))
  {
    UAPI_ERROR(fn,"AttachCurrentThreadAsDaemon failed.\n");
    return;
  }
#else
  if (jvm->AttachCurrentThreadAsDaemon((void**) &env, &args))
  {
    UAPI_ERROR(fn,"AttachCurrentThreadAsDaemon failed.\n");
    return;
  }
#endif
}

void NativeThreadWatcher::onThreadStopped()
{
  UAPI_FN_NAME("NativeThreadWatcher::onThreadStopped");
    
  if (jvm == 0)
  {
    UAPI_ERROR(fn,"jvm is null\n");
    return;
  }
  UAPI_INFO(fn,"jvm->DetachCurrentThread(), jvm is %p\n", jvm);
  jint rc = jvm->DetachCurrentThread();
  if (rc != 0)
    UAPI_ERROR(fn,"DetachCurrentThread failed: %d\n", rc);
  UAPI_INFO(fn,"jvm->DetachCurrentThread() returns %d\n", rc);
}

void NativeThreadWatcher::onThreadActive()
{
  ReturnCode::Type returnCode;
  UAPI_FN_NAME("NativeThreadWatcher::onThreadActive");
    
  JNIHelper::attachJVM(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to attach to JVM\n");
    return;
  }
}

void NativeThreadWatcher::onThreadInactive()
{
  ReturnCode::Type returnCode;
  UAPI_FN_NAME("NativeThreadWatcher::onThreadInactive");
    
  JNIHelper::detachJVM(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to detach from JVM\n");
    return;
  }
}

void NativeThreadWatcher::setJVM(JavaVM* _jvm)
{
  jvm = _jvm;
}

#ifdef ENABLE_JNI_KEEP_ALIVE
JVMKeepAlive* JVMKeepAlive::instance = 0;
Mutex* JVMKeepAlive::mutex = 0;
JVMKeepAlive::ComponentInitializer JVMKeepAlive::componentInitializer;

JVMKeepAlive::JVMKeepAlive(ConditionVariable* _threadStarted,
                           ConditionVariable* _shutdownRequested,
                           ConditionVariable* _onShutdown):
    threadStarted(_threadStarted),
    shutdownRequested(_shutdownRequested),
    onShutdown(_onShutdown),
    worker(0),
    jvm(0),
    jvmReferences(0)
{
  UAPI_FN_NAME("JVMKeepAlive::JVMKeepAlive");
    
  UAPI_TRACE(fn,"this=%p\n", this);
}

JVMKeepAlive* JVMKeepAlive::getInstance(ReturnCode::Type& returnCode)
{
  UAPI_FN_NAME("JNIKeepAlive::getInstance");
  if (componentInitializer.returnCode)
  {
    returnCode = componentInitializer.returnCode;
    return 0;
  }
  
  //we have to protect the construction of "instance". Make sure we don't have
  //multiple threads calling getInstance() while "instance" is equal to 0.
  LockScope lock(mutex, returnCode);
  if (returnCode)
  {
    UAPI_ERROR(fn,"Failed to create LockScope of mutex\n");
    return 0;
  }
  
  //it's now safe to check if instance == 0
  if (instance == 0)
  {
    ConditionVariable* threadStarted = ConditionVariable::create(JVMKeepAlive::mutex,
                                       returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"ConditionVariable::create() failed\n");
      return 0;
    }
    ConditionVariable* shutdownRequested = ConditionVariable::create(JVMKeepAlive::mutex,
                                           returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"ConditionVariable::create() failed\n");
      delete threadStarted;
      return 0;
    }
    ConditionVariable* onShutdown = ConditionVariable::create(JVMKeepAlive::mutex,
                                    returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"ConditionVariable::create() failed\n");
      delete threadStarted;
      delete shutdownRequested;
      return 0;
    }
    instance = new JVMKeepAlive(threadStarted, shutdownRequested, onShutdown);
    if (instance == 0)
    {
      UAPI_ERROR(fn,"Could not create JVMKeepAlive, OUT_OF_MEMORY\n");
      returnCode = ReturnCode::OUT_OF_MEMORY;
      delete threadStarted;
      delete shutdownRequested;
      delete onShutdown;
      return 0;
    }
  }
  returnCode = ReturnCode::SUCCESS;
  return instance;
}

JVMKeepAlive::~JVMKeepAlive()
{
  UAPI_FN_NAME("JVMKeepAlive::~JVMKeepAlive");
    
  UAPI_TRACE(fn,"this=%p\n", this);
  
  delete threadStarted;
  delete shutdownRequested;
  instance = 0;
}

void JVMKeepAlive::setJVM(JavaVM* _jvm)
{
  jvm = _jvm;
}

void JVMKeepAlive::attach(ReturnCode::Type& returnCode)
{
  UAPI_FN_NAME("JVMKeepAlive::attach");
    
    
  //make sure attach and detach are not called at the same time.
  LockScope ls(mutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"failed to lock attachDetachLock mutex\n");
    return;
  }
  UAPI_INFO(fn,"jvmReferences [%d -> %d]\n", jvmReferences, jvmReferences + 1);
  if (jvmReferences >= UINT8_MAX)
  {
    UAPI_ERROR(fn,"jvmReferences overflow\n");
    returnCode = ReturnCode::OVERFLOW_ERROR;
    return;
  }
  ++jvmReferences;
  if (jvmReferences == 1)
  {
    if (jvm == 0)
    {
      UAPI_ERROR(fn,"jvm is null\n");
      returnCode = ReturnCode::INVALID_STATE;
      return;
    }
    
    if (worker)
    {
      // JVMKeepAlive in the middle of shutting down
      onShutdown->wait(returnCode);
      if (returnCode)
      {
        UAPI_ERROR(fn,"onShutdown->wait() failed\n");
        return;
      }
    }
    worker = new JVMKeepAlive::Worker(this, mutex);
    if (returnCode)
    {
      UAPI_ERROR(fn,"JVMKeepAlive: Failed to create a new Runnable\n");
      return;
    }
    worker->start(returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"worker->start() failed\n");
      return;
    }
    
    //wait for the thread to tell us that it is running (it will call
    //signal).
    //wait, will unlock the threadStarted mutex.
    threadStarted->wait(returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"threadStarted->wait() failed\n");
      return;
    }
    
    //we we are here, the thread is running we have have re-acquired the
    //lock. The ConditionVariableLockScope will unlock the mutex for us.
    returnCode = ReturnCode::SUCCESS;
  }
  else
    returnCode = ReturnCode::SUCCESS;
  UAPI_INFO(fn,"jvmReferences on ++exit=%d\n", jvmReferences);
}

void JVMKeepAlive::detach(ReturnCode::Type& returnCode)
{
  UAPI_FN_NAME("JVMKeepAlive::detach");
    
    
  LockScope ls(mutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"failed to lock attachDetachLock mutex\n");
    return;
  }
  
  if (jvmReferences == 0)
  {
    returnCode = ReturnCode::UNDERFLOW_ERROR;
    UAPI_ERROR(fn,"Tried decrementing jvmReferences below 0\n");
    return;
  }
  UAPI_INFO(fn,"jvmReferences [%d -> %d]\n", jvmReferences, jvmReferences - 1);
  if (jvmReferences <= UINT8_MIN)
  {
    UAPI_ERROR(fn,"jvmReferences underflow\n");
    returnCode = ReturnCode::UNDERFLOW_ERROR;
    return;
  }
  --jvmReferences;
  if (jvmReferences == 0)
  {
    //signal the thread. It will shutdown.
    shutdownRequested->signal(returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"shutdownRequested->signal() failed\n");
      return;
    }
    
    ls.cancel(returnCode);
    if (returnCode)
    {
      UAPI_ERROR(fn,"LockScope->cancel() failed\n");
      return;
    }
    
    worker->join(returnCode);
    if (returnCode)
    {
      UAPI_ERROR(fn,"worker->join() failed\n");
      return;
    }
    
    LockScope ls2(mutex, returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"failed to lock attachDetachLock mutex\n");
      return;
    }
    onShutdown->signal(returnCode);
    if (returnCode)
    {
      UAPI_ERROR(fn,"onShutdown->signal() failed\n");
      return;
    }
    worker = 0;
    
    UAPI_INFO(fn,"jvmReferences on --exit=%d\n", jvmReferences);
  }
  else
    UAPI_INFO(fn,"jvmReferences on --exit=%d\n", jvmReferences);
  returnCode = ReturnCode::SUCCESS;
}


JVMKeepAlive::Worker::Worker(JVMKeepAlive* _parent, Mutex* mutex):
    Runnable(mutex),
    parent(_parent)
{}

ReturnCode::Type JVMKeepAlive::Worker::runThread()
{
  ReturnCode::Type returnCode;
  UAPI_FN_NAME("JVMKeepAlive::Worker::runThread");
    
  UAPI_INFO(fn,"started\n");
  
  // Attach JVM to current thread
  JNIEnv* env;
  JavaVMAttachArgs args = {0, 0, 0};
  args.version = JNI_VERSION_1_4;
  args.name = "JNI KeepAlive";
#if defined(NO_ATTACHCURRENTTHREAD_CAST_VOIDPP)
  if (parent->jvm->AttachCurrentThread(&env, &args))
    return ReturnCode::UNKNOWN;
#else
  if (parent->jvm->AttachCurrentThread((void**) &env, &args))
    return ReturnCode::UNKNOWN;
#endif
    
  LockScope ls(mutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"failed to lock threadStarted mutex\n");
    return returnCode;
  }
  parent->threadStarted->signal(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"threadStarted->signal() failed\n");
    return returnCode;
  }
  
  //we will lock back to wait for detach to be called. By calling wait, the
  //mutex will be unlocked.
  parent->shutdownRequested->wait(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"threadStarted->wait() failed\n");
    return returnCode;
  }
  //when we come back from wait, mutex is locked.
  
  // detach JVM from current thread
  UAPI_INFO(fn,"jvm->DetachCurrentThread(), jvm is %p\n", parent->jvm);
  jint rc = parent->jvm->DetachCurrentThread();
  if (rc != 0)
    UAPI_ERROR(fn,"DetachCurrentThread failed: %d.\n", rc);
  UAPI_INFO(fn,"jvm->DetachCurrentThread() returns %d\n", rc);
  return ReturnCode::SUCCESS;
}
#endif //ENABLE_JNI_KEEP_ALIVE
