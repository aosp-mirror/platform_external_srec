/*---------------------------------------------------------------------------*
 *  RefCounted.h                                                             *
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

#ifndef __UAPI__REFCOUNTED_
#define __UAPI__REFCOUNTED_

#include "exports.h"
#include "types.h"
#include "ReturnCode.h"
#include "Mutex.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        /**
         * Class that need to have a ref count can derive from this class. Once the
         * count goes down to zero (when Release() is called), the implementation of
         * RefCounted::Relase will call delete this.
         *
         * NOTE: In order to save memory, this class can have a maximum of 256 users,
         * it uses an UAPI_UINT8.
         */
        class UAPI_EXPORT RefCounted
        {
          public:
            /**
             * Default constructor. It initialize the count of the object to 1.
             *
             * @param returnCode SUCCESS unless a fatal error has occured.
             */
            RefCounted(ReturnCode::Type& returnCode);
            
            /**
             * Use this constructor if you want to object to be initialize with an
             * initial count greater than 1.
             *
             * @param initial_count The initial reference count of the object.
             * @param returnCode SUCCESS unless a fatal error has occured
             */
            RefCounted(UINT8 initial_count, ReturnCode::Type& returnCode);
            
            /**
             * destructor
             */
            virtual ~RefCounted();
            
            /**
             * This function increases the reference count by 1.
             *
             * @param returnCode ILLEGAL_ARGUMENT if mutex handle is null
             * @return the new reference count after the AddRef.
             */
            virtual UINT8 addRef(ReturnCode::Type& returnCode);
            
            /**
             * This function decreases the reference count by 1. If the count goes down
             * to 0, the object is deleted ("delete this;").
             * NOTE: make sure you don't use the object after ->Release is invoked. If
             * this Release makes the count go down to zero, the memory will be deleted
             * and referencing it could cause a core dump.
             *
             * @param returnCode SUCCESS unless a fatal error has occured.
             * @return the new reference count after the Release.
             */
            virtual UINT8 removeRef(ReturnCode::Type& returnCode);
            
            /**
             * Use this to get the current reference count of the object.
             */
            UINT8 getCount() const;
            
          protected:
          
            /**
             * variable that hold the count
             */
            UINT8 m_nRefCount;
            
            /**
             * protect from being called from different threads.
             */
            Mutex* m_mutex;
          private:
            /**
             * Implementation shared by constuctors.
             */
            void init(ReturnCode::Type& returnCode);
        };
      }
    }
  }
}

#endif
