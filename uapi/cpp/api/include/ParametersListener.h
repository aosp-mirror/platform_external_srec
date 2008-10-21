/*---------------------------------------------------------------------------*
 *  ParametersListener.h                                                     *
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

#ifndef __PARAMETERS_LISTNER_H_
#define __PARAMETERS_LISTNER_H_


#include "exports.h"
#include "types.h"
#include "ReturnCode.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      /**
       * Listens for Parameters events.
       */
      class ParametersListener
      {
        public:
          /**
           * Destructor
           */
          UAPI_EXPORT virtual ~ParametersListener();
          
          /**
           * Invoked if setting parameters has failed.
           *
           * @param keys the parameter keys that could not be set
           * @param values the parameter values associated with the keys
           * @param count the number of parameters
           * @param returnCode the return code
           */
          virtual void onParametersSetError(const char** keys, const char** values, ARRAY_LIMIT count,
                                            ReturnCode::Type returnCode) = 0;
                                            
          /**
           * Invoked if retrieving parameters has failed.
           *
           * @param keys the parameter keys that could not be set
           * @param count the number of parameters
           * @param returnCode the return code
           */
          virtual void onParametersGetError(const char** keys, ARRAY_LIMIT count,
                                            ReturnCode::Type returnCode) = 0;
                                            
          /**
           * This method is called when the parameters specified in setParameters have
           * successfully been set. This method is guaranteed to be invoked after onParametersSetError,
           * even if count==0.
           *
           * @param keys the list of parameter keys that were set
           * @param values the list of parameter values that were set
           * @param count the number of parameters
           */
          virtual void onParametersSet(const char** keys, const char** values, ARRAY_LIMIT count) = 0;
          
          /**
           * This method is called when the parameters specified in getParameters have
           * successfully been retrieved. This method is guaranteed to be invoked after onParametersGetError,
           * even if count==0.
           *
           * @param keys the list of parameter keys that were retrieved
           * @param values the list of parameter values that retrieved
           * @param count the number of parameters
           */
          virtual void onParametersGet(const char** keys, const char** values, ARRAY_LIMIT count) = 0;
      };
    }
  }
}

#endif
