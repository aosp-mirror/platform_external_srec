/*---------------------------------------------------------------------------*
 *  WorkerQueueFactoryImpl.h                                                 *
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

#ifndef __UAPI__WORKERQUEUE_FACTORY_IMPL
#define __UAPI__WORKERQUEUE_FACTORY_IMPL

#include "WorkerQueueFactory.h"
#include "DoublyLinkedList.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class Task;
        class Mutex;
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
      namespace utilities
      {
        /**
         * Implementation of a lass responsible giving access to WorkerQueue. It
         * acts as a pool of WorkerQueue(s). One can get an instance of a
         * WorkerQueue by accessing the getWorkerQueue method. There is no
         * guarantee that the returned WorkerQueue will not be shared. For this
         * reason, Tasks executed on a WorkerQueue should never block for a long
         * period of time.
         */
        class WorkerQueueFactoryImpl: public WorkerQueueFactory
        {
          public:
            /**
             * destructor
             */
            virtual ~WorkerQueueFactoryImpl();
            
            
            /**
             * Returns a WorkerQueue implementation.
             *
             * @param returnCode the return code. SUCCESS unless a fatal error has occured.
             * THREAD_ERROR on failure.
             * @return a instance of a WorkerQueue. The object returned is owned by
             * the WorkerQueueFactory. This means that the application MUST NOT
             * called delete on the returned object.
             */
            virtual WorkerQueue* getWorkerQueue(ReturnCode::Type& returnCode);
            
            /**
             * Set the number of worker queues that will be part of the pool. This
             * parameter will decide if a new WorkerQueue instance is created or
             * "re-used" when getWorkerQueue is called.
             *
             * @param numWorkerQueue the maximum number of WorkerQueue that the
             * pool should contain.
             * @param returnCode the return code. SUCCESS unless a fatal error has occured.
             */
            virtual void setPoolSize(UINT8 numWorkerQueue, ReturnCode::Type& returnCode);
            
            /**
             * Called by the JNI layer (optionally used) when the JNI layer is used
             * for the first time. The JNI layer may use these callbacks to change
             * the property of the native threads, e.g. attachThreadAsDeamon when
             * onThreadStarted() is called.
             */
            virtual void setJNIThreadListener(utilities::JNIThreadListener* jniThreadListener);
            
            /**
             * Called by the WorkerQueue when they are ready to send the callback
             * to the JNIThreadListener.
             *
             * @return the JNI thread listener.
             */
            virtual JNIThreadListener* getJNIThreadListener();
            
          protected:
            /**
             * Prevent construction.
             */
            WorkerQueueFactoryImpl();
            
          private:
            /**
             * Initializes the component when the library is loaded.
             */
            class ComponentInitializer
            {
              public:
                ComponentInitializer();
                ~ComponentInitializer();
                
                ReturnCode::Type returnCode;
            };
            
            static ComponentInitializer componentInitializer;
            
            /**
             * Synchronizes access to the singleton.
             */
            static Mutex* mutex;
            
            DoublyLinkedList workerQueueList;
            
            /**
             * maximum number of worker queues that this class will construct, i.e.
             * add to _workerQueueList and return in getWorkerQueue.
             */
            UINT8 maxNumWorkerQueues;
            UINT8 indexToReturn;
            /**
             * JNI Thread listener. This is only used when the cpp code is used
             * by the JNI adaptor.
             */
            JNIThreadListener* jniThreadListener;
            /**
             * singleton instance
             */
            static WorkerQueueFactoryImpl* instance;
            
            
            friend class WorkerQueueFactory;
        };
      }
    }
  }
}

#endif
