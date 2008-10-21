/*---------------------------------------------------------------------------*
 *  CWrapperSrecGrammarListener.h                                            *
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

#ifndef __UAPI__CWRAPPERSRECGRAMMARLISTENER
#define __UAPI__CWRAPPERSRECGRAMMARLISTENER

#include <jni.h>
#include "SrecGrammarListener.h"
#include "CWrapperEmbeddedGrammarListener.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace jni
      {
        class CWrapperSrecGrammarListener: public SrecGrammarListener
        {
          public:
            CWrapperSrecGrammarListener(JavaVM* jvm, jobject listener, ReturnCode::Type& returnCode);
            ~CWrapperSrecGrammarListener();
            
            virtual void onLoaded();
            virtual void onUnloaded();
            virtual void onSaved(const char* path);
            virtual void onError(jthrowable exception);
            virtual void onError(ReturnCode::Type returnCode);
            virtual void onCompileAllSlots();
            virtual void onResetAllSlots();
            virtual bool isEmbeddedGrammarListener();
            virtual bool isSrecGrammarListener();
            virtual void onAddItemList();
            virtual void onAddItemListFailure(int index, ReturnCode::Type returnCode);
            virtual void onLoadedVoicetag();
            virtual void onSavedVoicetag();

          private:
            /**
             * The underlying listener.
             */
            CWrapperEmbeddedGrammarListener delegate;
        };
      }
    }
  }
}

#endif
