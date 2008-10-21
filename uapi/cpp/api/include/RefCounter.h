/*---------------------------------------------------------------------------*
 *  RefCounter.h                                                             *
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

#ifndef __UAPI__REFCOUNTER
#define __UAPI__REFCOUNTER

#include "exports.h"
#include "types.h"
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
         * A class that monitors the references to a particular object. This
         * class is not thread safe.
         */
        class UAPI_EXPORT RefCounter
        {
          public:
            /**
             * Creates a new RefCounter with an initial value of one.
             *
             * @param object the underlying object to wrap
             * @param loggingAllowed true if the object and its dependencies are allowed to log
             * @param returnCode SUCCESS unless a fatal error has occured
             */
            RefCounter(void* object, bool loggingAllowed, ReturnCode::Type& returnCode);
            virtual ~RefCounter();
            
            /**
             * This function increases the reference count by one.
             *
             * @param returnCode SUCCESS unless a fatal error has occured
             * @return the resulting reference count
             */
            virtual ARRAY_LIMIT increment(ReturnCode::Type& returnCode);
            
            /**
             * This function decreases the reference count by 1.
             *
             * @param returnCode SUCCESS unless a fatal error has occured
             * @return the resulting reference count
             */
            virtual ARRAY_LIMIT decrement(ReturnCode::Type& returnCode);
            
            /**
             * Returns the number of references to the object.
             *
             * @return the number of references to the object
             */
            ARRAY_LIMIT getCount() const;
            
            /**
             * Returns the object whose references are being monitored.
             */
            void* getObject() const;
            
            /**
             * Returns true if the object and its dependencies are allowed to log.
             *
             * @return true if the object and its dependencies are allowed to log
             */
            bool isLoggingAllowed() const;
          protected:
            /**
             * The number of references to the underlying object.
             */
            ARRAY_LIMIT count;
            /**
             * The underlying object.
             */
            void* object;
            
            bool loggingAllowed;
        };
      }
    }
  }
}

#endif
