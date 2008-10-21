/*---------------------------------------------------------------------------*
 *  CWrapperEmbeddedGrammarListener.cpp                                      *
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
#include "jniapi.h"
#include "CWrapperEmbeddedGrammarListener.h"
#include "JNIHelper.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;


CWrapperEmbeddedGrammarListener::CWrapperEmbeddedGrammarListener(JavaVM* jvm, jobject listener,
    ReturnCode::Type& returnCode):
    delegate(jvm, listener, returnCode)
{
}

CWrapperEmbeddedGrammarListener::~CWrapperEmbeddedGrammarListener()
{}

JavaVM* CWrapperEmbeddedGrammarListener::getJVM()
{
  return  delegate.getJVM();
}

jobject CWrapperEmbeddedGrammarListener::getListener()
{
  return  delegate.getListener();
}

void CWrapperEmbeddedGrammarListener::invokeMethod(const char* methodName)
{
    return  delegate.invokeMethod(methodName);
}

void CWrapperEmbeddedGrammarListener::onLoaded()
{
  delegate.onLoaded();
}

void CWrapperEmbeddedGrammarListener::onUnloaded()
{
  delegate.onUnloaded();
}

void CWrapperEmbeddedGrammarListener::onError(ReturnCode::Type returnCode)
{
  delegate.onError(returnCode);
}

void CWrapperEmbeddedGrammarListener::onError(jthrowable exception)
{
   delegate.onError(exception);
}

void CWrapperEmbeddedGrammarListener::onSaved(const char* path)
{
  UAPI_FN_SCOPE("CWrapperEmbeddedGrammarListener::onSaved");
    
  // Get enviromnent pointer for the current thread
  JNIEnv* env = JNIHelper::getEnv(delegate.getJVM());
  
  jclass cls = env->GetObjectClass(delegate.getListener());
  jmethodID mid = env->GetMethodID(cls, "onSaved", "(Ljava/lang/String;)V");
  env->DeleteLocalRef(cls);
  assert(mid != 0);
  jstring jPath = env->NewStringUTF(path);
  env->CallVoidMethod(delegate.getListener(), mid, jPath);
  if (env->ExceptionCheck())
  {
    jthrowable exc = env->ExceptionOccurred();
    env->ExceptionClear();
    onError(exc);
    env->DeleteLocalRef(exc);
  }
  env->DeleteLocalRef(jPath);
}

void CWrapperEmbeddedGrammarListener::onCompileAllSlots()
{
  UAPI_FN_SCOPE("CWrapperEmbeddedGrammarListener::onCompileAllSlots");
    
  delegate.invokeMethod("onCompileAllSlots");
}

void CWrapperEmbeddedGrammarListener::onResetAllSlots()
{
  UAPI_FN_SCOPE("CWrapperEmbeddedGrammarListener::onResetAllSlots");
    
  delegate.invokeMethod("onResetAllSlots");
}

bool CWrapperEmbeddedGrammarListener::isEmbeddedGrammarListener()
{
  return true;
}
bool CWrapperEmbeddedGrammarListener::isSrecGrammarListener()
{
  return false;
}
