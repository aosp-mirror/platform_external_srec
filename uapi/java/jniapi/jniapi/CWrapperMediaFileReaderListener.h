/*---------------------------------------------------------------------------*
 *  CWrapperMediaFileReaderListener.h                                        *
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

#ifndef __UAPI__CWRAPPERMEDIAFILEREADERLISTENER
#define __UAPI__CWRAPPERMEDIAFILEREADERLISTENER

#include <jni.h>
#include "MediaFileReaderListener.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace jni
      {
        class CWrapperMediaFileReaderListener: public MediaFileReaderListener
        {
          public:
            CWrapperMediaFileReaderListener(JavaVM* jvm, jobject listener, ReturnCode::Type& returnCode);
            ~CWrapperMediaFileReaderListener();
            
            void onStarted();
            void onStopped();
            void onError(jthrowable exception);
            void onError(ReturnCode::Type returnCode);
          protected:
            /**
            * Invokes the method with the specified name.
            */
            void invokeMethod(const char* methodName);
          private:
            JavaVM* jvm;
            jobject listener;
        };
      }
    }
  }
}

#endif
