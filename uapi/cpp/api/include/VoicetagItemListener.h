/*---------------------------------------------------------------------------*
 *  VoicetagItemListener.h                                                *
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

#ifndef __UAPI__VOICETAGITEMLISTENER
#define __UAPI__VOICETAGITEMLISTENER

#include "exports.h"
#include "ReturnCode.h"
#include "SmartProxy.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      /**
       * Listens for Voicetag events.
       */
      class VoicetagItemListener
      {
        public:

         /**
           * Invoked after the Voicetag is saved.
           *
           * @param path the path the voicetag was saved to
           */
          virtual void onSaved(const char* path) = 0;
          
           /**
            * Invoked after the Voicetag is loaded.
            */
          virtual void onLoaded() = 0;
          
          /**
           * Invoked when an unexpected error occurs.
           *
           * @param returnCode READ_ERROR if the Voicetag could not be loaded.<br/>
           * WRITE_ERROR if the Voicetag could not be saved.
           */
          virtual void onError(ReturnCode::Type error) = 0;

        protected:
          /**
           * Prevent construction.
           */
          UAPI_EXPORT VoicetagItemListener();
          /**
           * Prevent destruction.
           */
          UAPI_EXPORT virtual ~VoicetagItemListener();
          
          friend class VoicetagItemListenerProxy;
      };
      
      /*
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, VoicetagItemListenerProxy, SmartProxy,
                         VoicetagItemListener)
    }
  }
}

#endif
