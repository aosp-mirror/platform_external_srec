/*---------------------------------------------------------------------------*
 *  Singleton.h                                                              *
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

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include "SmartProxy.h"


/**
 * Base class used to register Singleton classes with the System class.
 */
namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class Singleton
      {
        protected:
          virtual ~Singleton()
          {}
          
          /**
           * Returns the SmartProxy root associated with the singleton.
           *
           * @return the SmartProxy root associated with the singleton
           */
          virtual android::speech::recognition::SmartProxy::Root* getRoot() = 0;
          
          /**
           * Invoked when the singleton has to shutdown.
           *
           * @param returnCode the return code.
           */
          virtual void shutdown(ReturnCode::Type& returnCode) = 0;
          
          friend class System;
      };
    }
  }
}

#endif
