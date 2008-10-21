/*---------------------------------------------------------------------------*
 *  WorkerQueue.h                                                            *
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

#ifndef __UAPI__WORKERQUEUE_
#define __UAPI__WORKERQUEUE_

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
        class Task;
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
         * This class is a queue of Task(s) that are invoked later. When dequeued the
         * worker queue calls run() on the Task. The worker queue supports plain Tasks
         * and ScheduledTasks. On some platforms the dequeue will happen in a dedicated
         * thread. This is not guaranteed on all the platforms.
         */
        class UAPI_EXPORT WorkerQueue
        {
          public:
            /**
             * destructor
             */
            virtual ~WorkerQueue()
            {}
            
            /**
             * Queues a task for later execution.
             *
             * Note: Enqueuing a task transfers its ownership to the worker queue.
             *
             * @param task the task to execute
             * @param returnCode ILLEGAL_ARGUMENT if the task is null
             */
            virtual void enqueue(Task* task, ReturnCode::Type& returnCode) = 0;
            
            /**
             * Removes a task that was queued for later execution if the task was
             * not already dequeued and processed.
             *
             * @param task the task to execute
             * @param returnCode ILLEGAL_ARGUMENT if the task is null
             */
            virtual void remove(Task* task, ReturnCode::Type& returnCode) = 0;
            
            /**
             * Indicates if the worker queue is running.
             *
             * @param returnCode SUCCESS unless a fatal error occurs
             * @return true if the worker queue is running
             */
            virtual bool isRunning(ReturnCode::Type& returnCode) = 0;
          protected:
            WorkerQueue()
            {}
        };
      }
    }
  }
}

#endif
