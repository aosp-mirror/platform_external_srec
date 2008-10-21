/*---------------------------------------------------------------------------*
 *  CWrapperSrecGrammarListener.cpp                                          *
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
#include "CWrapperSrecGrammarListener.h"
#include "JNIHelper.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;


CWrapperSrecGrammarListener::CWrapperSrecGrammarListener(JavaVM* jvm, jobject listener,
    ReturnCode::Type& returnCode):
    delegate(jvm, listener, returnCode)
{
}

CWrapperSrecGrammarListener::~CWrapperSrecGrammarListener()
{}

void CWrapperSrecGrammarListener::onLoaded()
{
    delegate.onLoaded();
}

void CWrapperSrecGrammarListener::onUnloaded()
{
    delegate.onUnloaded();
}

void CWrapperSrecGrammarListener::onError(ReturnCode::Type returnCode)
{
    delegate.onError(returnCode);
}

void CWrapperSrecGrammarListener::onError(jthrowable exception)
{
  delegate.onError(exception);
}

void CWrapperSrecGrammarListener::onSaved(const char* path)
{
  delegate.onSaved(path);
}

void CWrapperSrecGrammarListener::onCompileAllSlots()
{
    delegate.invokeMethod("onCompileAllSlots");
}

void CWrapperSrecGrammarListener::onResetAllSlots()
{
    delegate.invokeMethod("onResetAllSlots");
}

bool CWrapperSrecGrammarListener::isEmbeddedGrammarListener()
{
  return true;
}

bool CWrapperSrecGrammarListener::isSrecGrammarListener()
{
  return true;
}

void CWrapperSrecGrammarListener::onAddItemList()
{
    delegate.invokeMethod("onAddItemList");
}

void CWrapperSrecGrammarListener::onAddItemListFailure(int index, ReturnCode::Type returnCode)
{
    UAPI_FN_SCOPE("CWrapperSrecGrammarListener::onAddItemListFailure");
    // Get enviromnent pointer for the current thread
    JNIEnv* env = JNIHelper::getEnv(delegate.getJVM());
  
    jclass cls = env->GetObjectClass(delegate.getListener());
    jmethodID mid = env->GetMethodID(cls, "onAddItemListFailure", "(ILjava/lang/Exception;)V");
    env->DeleteLocalRef(cls);
    assert(mid != 0);

    jclass clazz = JNIHelper::loadClass(env, delegate.getListener(), "java.lang.RuntimeException");
    jmethodID constructor = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;)V");
    assert(constructor != 0);
    jstring jrc = env->NewStringUTF(ReturnCode::toString(returnCode));
    jobject exception = env->NewObject(clazz, constructor, jrc);
    env->DeleteLocalRef(jrc);
    env->DeleteLocalRef(clazz);

    env->CallVoidMethod(delegate.getListener(), mid,(jint) index, exception);
    if (env->ExceptionCheck())
    {
        jthrowable exc = env->ExceptionOccurred();
        env->ExceptionClear();
        env->DeleteLocalRef(exc);
    }
    env->DeleteLocalRef(exception);
}

void CWrapperSrecGrammarListener::onLoadedVoicetag()
{
    delegate.invokeMethod("onLoadedVoicetag");
}

void CWrapperSrecGrammarListener::onSavedVoicetag()
{
    delegate.invokeMethod("onSavedVoicetag");
}
