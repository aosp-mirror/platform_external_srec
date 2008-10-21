/*---------------------------------------------------------------------------*
 *  AudioSourceListener.h                                                    *
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

#ifndef __UAPI__AUDIOSOURCELISTER
#define __UAPI__AUDIOSOURCELISTER

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
       * Listens for AudioSource events.
       */
      class AudioSourceListener
      {
        public:
          /**
           * Invoked when reading begins.
           */
          virtual void onStarted() = 0;
          
          /**
           * Invoked when the reading ends (either normally or due to an error).
           */
          virtual void onStopped() = 0;
          
          /**
           * Invoked when an unexpected error occurs. This is normally followed by
           * onStopped() if the component shuts down successfully.
           *
           * @param error specific error code.
           */
          virtual void onError(ReturnCode::Type error) = 0;
        protected:
          /**
           * Prevent construction.
           */
          UAPI_EXPORT AudioSourceListener();
          /**
           * Prevent destruction.
           */
          UAPI_EXPORT virtual ~AudioSourceListener();
          
          friend class AudioSourceListenerProxy;
      };
      
      /*
       * @see android::speech::recognition::SmartProxy
       */
      
      DECLARE_SMARTPROXY(UAPI_EXPORT, AudioSourceListenerProxy, android::speech::recognition::SmartProxy,
                         AudioSourceListener)
    }
  }
}

#endif
