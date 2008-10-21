/*---------------------------------------------------------------------------*
 *  android_speech_recognition_impl_SrecGrammarImpl.cpp                                            *
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
#include "jniapi.h"
#include "android_speech_recognition_impl_SrecGrammarImpl.h"
#include "JNIHelper.h"
#include "SrecGrammar.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::jni;


JNIEXPORT void JNICALL Java_android_speech_recognition_impl_SrecGrammarImpl_addItemProxy(JNIEnv* env,
    jobject grammar,
    jlong nativeObj,
    jstring slotName,
    jlong slotItemObj,
    jint weight,
    jstring semanticMeaning)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_SrecGrammarImpl_addItemProxy");
  
 SrecGrammarProxy* nativeGrammarPointer = (SrecGrammarProxy*) nativeObj;
  if (!nativeGrammarPointer)
  {
    JNIHelper::throwJavaException(env, grammar, ReturnCode::INVALID_STATE);
    return;
  }
  SrecGrammarProxy& nativeGrammar = *nativeGrammarPointer;
  
  SlotItemProxy* nativeItemPointer = (SlotItemProxy*) slotItemObj;
  if (!nativeItemPointer)
  {
    JNIHelper::throwJavaException(env, grammar, ReturnCode::INVALID_STATE);
    return;
  }
  SlotItemProxy& nativeItem = *nativeItemPointer;
  
  const char* nativeSlotName = env->GetStringUTFChars(slotName, 0);
  const char* nativeSemanticMeaning = env->GetStringUTFChars(semanticMeaning, 0);
  nativeGrammar->addItem(nativeSlotName, nativeItem, weight, nativeSemanticMeaning, returnCode);
  env->ReleaseStringUTFChars(semanticMeaning, nativeSemanticMeaning);
  env->ReleaseStringUTFChars(slotName, nativeSlotName);
  
  if (returnCode)
    JNIHelper::throwJavaException(env, grammar, returnCode);
}

JNIEXPORT void JNICALL Java_android_speech_recognition_impl_SrecGrammarImpl_addItemListProxy
  (JNIEnv *env, 
   jobject grammar, 
   jlong nativeObj,
   jstring slotName, 
   jlongArray Items, 
   jintArray weights,
   jobjectArray meanings)
{
  ReturnCode::Type returnCode = ReturnCode::SUCCESS;
  UAPI_FN_SCOPE("Java_android_speech_recognition_impl_SrecGrammarImpl_addItemListProxy");

  SrecGrammarProxy* nativeGrammarPointer = (SrecGrammarProxy*) nativeObj;
  if (!nativeGrammarPointer)
  {
    JNIHelper::throwJavaException(env, grammar, ReturnCode::INVALID_STATE);
    return;
  }
  SrecGrammarProxy& nativeGrammar = *nativeGrammarPointer;

  jsize itemCount = env->GetArrayLength(Items);
  jlong *itemsCopy = 0;

  const char** semanticMeaningsCopy = 0;
  int* weightsCopy = 0;
  jsize i = 0;

  if (itemCount>0)
  {
    // Copy java arguments 
    const char* slotNameCopy = env->GetStringUTFChars(slotName, 0); 
    // copy items
    itemsCopy = env->GetLongArrayElements(Items,0);
    SlotItemProxy **nativeSlotItems = new SlotItemProxy*[itemCount];
    // create array for semantic meaning
    semanticMeaningsCopy = new const char*[itemCount];
    // copy weights
    weightsCopy = (int*) env->GetIntArrayElements(weights,0);
   
    const char* text = NULL;
    int size = 0;
    for (i=0;i<itemCount;++i)
    {
        // copy items
        nativeSlotItems[i] = (SlotItemProxy *)itemsCopy[i];
        // copy semantic meanings
        jstring semantic = (jstring) env->GetObjectArrayElement(meanings,i);
        text = env->GetStringUTFChars(semantic,0);
        size = (int)strlen(text);
        semanticMeaningsCopy[i] = new char[size+1];
        strcpy((char*)semanticMeaningsCopy[i],text);
        env->ReleaseStringUTFChars(semantic,text);
        env->DeleteLocalRef(semantic);
    }

    nativeGrammar->addItemList(slotNameCopy,
                             nativeSlotItems,
                             weightsCopy,
                             semanticMeaningsCopy,
                             itemCount,
                             returnCode);
    // Release memory
    env->ReleaseIntArrayElements(weights,(jint*)weightsCopy,0);
    env->ReleaseStringUTFChars(slotName, slotNameCopy);
    env->ReleaseLongArrayElements(Items,(jlong*)itemsCopy,0);
    delete[] nativeSlotItems;
    nativeSlotItems =NULL;
    for (i=0;i<itemCount;++i)
        if ( semanticMeaningsCopy[i] !=NULL) delete  semanticMeaningsCopy[i];
    delete[] semanticMeaningsCopy; 
  }
  else
   returnCode = ReturnCode::INVALID_STATE;
   
  if (returnCode)
    JNIHelper::throwJavaException(env, grammar, returnCode);
}
