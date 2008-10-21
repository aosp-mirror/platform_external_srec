/*---------------------------------------------------------------------------*
 *  android_speech_recognition_impl_System.cpp                                                     *
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
 * android_speech_recognition_impl_System.cpp
 */
#include <assert.h>
#include "jniapi.h"
#include "android_speech_recognition_impl_System.h"
#include "System.h"
#include "JNIHelper.h"
#include "WorkerQueueFactory.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;
using namespace android::speech::recognition::utilities;


JNIEXPORT jstring JNICALL Java_android_speech_recognition_impl_System_getAPIVersion
  (JNIEnv *env, jclass)
{
    return env->NewStringUTF("1.0");
}


JNIEXPORT jlong JNICALL Java_android_speech_recognition_impl_System_initNativeObject(JNIEnv* env,
    jclass system)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_System_initNativeObject");
  
#ifdef ENABLE_JNI_KEEP_ALIVE
  //make sure we create the KeepAlive singleton here.
  JVMKeepAlive* keepalive = JVMKeepAlive::getInstance(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to get the instance of the JVMKeepAlive\n");
    JNIHelper::throwJavaException(env, 0, returnCode);
    return 0;
  }
#endif
  
  JavaVM* jvm = JNIHelper::getJVM(env, system);
  if (env->ExceptionCheck())
    return 0;
#ifdef ENABLE_JNI_KEEP_ALIVE
  keepalive->setJVM(jvm);
#endif
  
  //make sure we create the NativeThreadWatcher singleton here.
  NativeThreadWatcher* threadWatcher = NativeThreadWatcher::getInstance(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Could not get singleton instance of the native thread watcher\n");
    JNIHelper::throwJavaException(env, 0, returnCode);
    return 0;
  }
  threadWatcher->setJVM(jvm);
  
  // Create the native System
  System* nativeSystem = System::getInstance(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    JNIHelper::throwJavaException(env, 0, returnCode);
    return 0;
  }
  
  // The WorkerQueueFactory must know about the JNIThreadListener. Pass it the
  // pointer.
  WorkerQueueFactory* workerFactory = WorkerQueueFactory::getInstance(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    JNIHelper::throwJavaException(env, 0, returnCode);
    return 0;
  }
  workerFactory->setJNIThreadListener(threadWatcher);
  
  return (jlong) nativeSystem;
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_System_disposeProxy(JNIEnv*,
    jclass)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_System_disposeProxy");
    
  System* nativeSystem = System::getInstance(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
    return;
  nativeSystem->dispose(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
    return;
  delete nativeSystem;
}
