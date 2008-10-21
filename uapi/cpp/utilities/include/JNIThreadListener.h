/*---------------------------------------------------------------------------*
 *  JNIThreadListener.h                                                      *
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

#ifndef __UAPI__JNI_THREAD_LISTENER_
#define __UAPI__JNI_THREAD_LISTENER_

#include "exports.h"
#include "ReturnCode.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        /**
         * class used to listen for specific WorkerQueue events.
         */
        class JNIThreadListener
        {
          public:
            /**
             * destructor
             */
            virtual ~JNIThreadListener()
            {}
            
            /**
             * Called when a native thread is started.
             */
            virtual void onThreadStarted() = 0;
            
            /**
             * Called when a native thread is stopped.
             */
            virtual void onThreadStopped() = 0;
            
            /**
             * Called when a native thread is waken up from its waiting state, i.e.
             * it was told to start executing some work.
             */
            virtual void onThreadActive() = 0;
            
            /**
             * Called when a native thread is not processing anything, i.e. it is
             * in a waiting state.
             */
            virtual void onThreadInactive() = 0;
        };
      }
    }
  }
}

#endif
