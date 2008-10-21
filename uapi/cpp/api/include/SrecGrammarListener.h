/*---------------------------------------------------------------------------*
 *  SrecGrammarListener.h                                                    *
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

#ifndef __UAPI__SRECGRAMMARLISTENER
#define __UAPI__SRECGRAMMARLISTENER

#include "EmbeddedGrammarListener.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      /**
       * Listens for SrecGrammar events.
       */
      class SrecGrammarListener: public EmbeddedGrammarListener
      {
        public:
          UAPI_EXPORT virtual ~SrecGrammarListener();
          
            /**
            * Invokes after all items of the list have been added.
            */
            virtual void onAddItemList() = 0;
               
            /**
            * Invoked when adding a SlotItem from a list fails. 
            * This callback will be trigger for each element in the list that fails to be
            * add in the slot, unless there is a grammar fail operation, which will be
            * reported in the onError callback.
            * 
            * @param index the item index that cause the failure.
            * @param e the cause of the failure.
            */
            virtual void onAddItemListFailure(int index, ReturnCode::Type returnCode)=0;

            /**
            * Invoked after a Voicetag has been loaded.
            */
            virtual void onLoadedVoicetag()=0;
   
            /**
            * Invoked after a Voicetag has been saved.
            */
            virtual void onSavedVoicetag()=0;
      };
      /**
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, SrecGrammarListenerProxy,EmbeddedGrammarListenerProxy, SrecGrammarListener)
    }
  }
}

#endif
