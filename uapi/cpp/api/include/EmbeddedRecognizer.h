/*---------------------------------------------------------------------------*
 *  EmbeddedRecognizer.h                                                     *
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

#ifndef __UAPI__EMBEDDEDRECOGNIZER
#define __UAPI__EMBEDDEDRECOGNIZER

#include <assert.h>
#include "exports.h"
#include "ReturnCode.h"
#include "Recognizer.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class RecognizerListenerProxy;
      namespace utilities
      {
        class LibraryLoader;
      }
      namespace impl
      {
        class RedirectToLibrary;
      }
    }
  }
}


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class EmbeddedRecognizerProxy;
      /**
       * Embedded recognizer.
       */
      class EmbeddedRecognizer: public Recognizer
      {
        public:
          /**
           * Creates a new embedded recognizer.
           *
           * @param returnCode the return code
           * @return returns a new embedded recognizer instance
           */
          UAPI_EXPORT static EmbeddedRecognizerProxy getInstance(ReturnCode::Type& returnCode);
          
          /**
           * Configures the embedded recognizer.
           *
           * @param config recognizer configuration file
           * @param returnCode ILLEGAL_ARGUMENT if config is null.<br/>
           * OPEN_ERROR, or READ_ERROR if the recognizer configuration, acoustic model, or
           * vocabulary files could not be opened or read.
           */
          virtual void configure(const char* config, ReturnCode::Type& returnCode) = 0;
                                             
          /**
           * The recognition accuracy improves over time as the recognizer adapts to the surrounding
           * environment. This method enables developers to reset the adaptation when the environment
           * is known to have changed.
           *
           * @param returnCode the return code
           */
          virtual void resetAcousticState(ReturnCode::Type& returnCode) = 0;
        protected:
          /**
           * Prevent construction.
           */
          UAPI_EXPORT EmbeddedRecognizer();
          /**
           * Prevent destruction.
           */
          UAPI_EXPORT virtual ~EmbeddedRecognizer();
          
          friend class impl::RedirectToLibrary;
          friend class EmbeddedRecognizerProxy;
      };
      
      /*
       * @see android::speech::recognition::SmartProxy
       */
      DECLARE_SMARTPROXY(UAPI_EXPORT, EmbeddedRecognizerProxy, RecognizerProxy, EmbeddedRecognizer)
      
      /**
       * Invokes EmbeddedRecognizer::configure() on the remote instance.
       */
      typedef EmbeddedRecognizerProxy*(*ConfigureEmbeddedRecognizer)(const char* config,
          ReturnCode::Type& returnCode);
    }
  }
}

#endif
