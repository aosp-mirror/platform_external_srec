/*---------------------------------------------------------------------------*
 *  Task.h                                                                   *
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

#ifndef __UAPI__TASK__
#define __UAPI__TASK__

#include "exports.h"
#include "types.h"
#include "TimeInstant.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class Mutex;
        class WorkerQueueImpl;
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
         * Represents an executable unit of work.
         */
        class Task
        {
          public:
            /**
             * Creates a new Task.
             *
             * @param taskName name of the task.
             */
            UAPI_EXPORT Task(const char* taskName);
            
            /**
             * destructor.
             */
            UAPI_EXPORT virtual ~Task();
            
            
            /**
             * Executes the unit of work.
             *
             * If this method is ever invoked, it is guaranteed to get invoked at most once.
             * Because this method is not guaranteed to get invoked, all cleanup code should
             * be placed in the destructor.
             */
            UAPI_EXPORT virtual void run() = 0;
            
            /**
             * Only ScheduledTask can have a timeout. Simple Tasks will always have a
             * timeout of 0. This method returns the value of the timeout when this
             * object was constructed. It is there mainly to be called by the internal
             * worker queue implementation.
             * @return the value of the timeout in msec
             */
            UAPI_EXPORT UINT32 getTimeout() const;
           
            /**
             * Get the name of this Task. 
             */
            const char * getName() const { return m_name; }
          protected:
            /**
             * Creates a new Task. It can be called only by the ScheduledTask.
             *
             * @param timeout_msec when should this task be run in mili-seconds.
             * @param taskName name of the task.
             */
            UAPI_EXPORT Task(UINT32 timeout_msec, const char* taskName);
            
            /**
             * the timeout in mili-seconds
             */
            UINT32 m_timeout;
            
          private:
            /**
             * Prevent copying.
             */
            UAPI_EXPORT Task& operator=(Task& other);
            /**
             * Internal use only. This is needed by the WorkerQueueImpl to know
             * when this task will expire. When a Task expires, it is ready to be
             * "Run". By adding this member variable, we avoid having to wrap the Task
             * object into another class that contains m_expire. In that case we would
             * have to allocate memory twice, for the Task and for the wrapper.
             */
            TimeInstant m_expire;
            /**
             * name of this task. Mainly used for debugging.
             */
            const char* m_name;

            friend class WorkerQueueImpl;
        };
        
        /**
         * Same thing as a Task, except that this Task is scheduled. This means that
         * the run() method will get called only once the timeout has expired.
         */
        class ScheduledTask: public Task
        {
          public:
            /**
             * Creates a new ScheduledTask.
             *
             * @param timeout_msec When should this task execute in mili-seconds.
             * @param taskName name of the task.
             */
            UAPI_EXPORT ScheduledTask(UINT32 timeout_msec, const char * taskName): Task(timeout_msec, taskName) {}
            
            /**
             * destructor
             */
            UAPI_EXPORT virtual ~ScheduledTask();
          private:
            /**
             * Prevent copying.
             */
            ScheduledTask& operator=(ScheduledTask& other);
        };
        
        
      }
    }
  }
}

#endif
