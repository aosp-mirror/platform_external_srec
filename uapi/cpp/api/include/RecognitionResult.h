/*---------------------------------------------------------------------------*
 *  RecognitionResult.h                                                      *
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

#ifndef __UAPI__RECOGNITIONRESULT
#define __UAPI__RECOGNITIONRESULT

#include "exports.h"
#include "ReturnCode.h"
#include "types.h"
#include "SmartProxy.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      /**
       * Recognition results.
       */
      class RecognitionResult
      {
        public:
          /**
           * Returns true if the recognition result is an n-best list.
           */
          virtual bool isNBestList() const = 0;

          /**
           * Returns true if the recognition result are coming from an
           * application server.
           */
          virtual bool isAppServerResult() const = 0;
          
        protected:
          /**
           * Prevent construction.
           */
          UAPI_EXPORT RecognitionResult();
          /**
           * Prevent destruction.
           */
          UAPI_EXPORT virtual ~RecognitionResult();
          
          friend class RecognitionResultProxy;
      };
      
      /*
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, RecognitionResultProxy, android::speech::recognition::SmartProxy,
                         RecognitionResult)
    }
  }
}

#endif
