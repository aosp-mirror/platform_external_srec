/*---------------------------------------------------------------------------*
 *  WorkerQueueImpl.h                                                        *
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

#ifndef __UAPI__WORKERQUEUEIMPL_
#define __UAPI__WORKERQUEUEIMPL_

#if !defined (UAPI_LINUX) && !defined(UAPI_WIN32)
# error unsupported platform
#endif


#include "WorkerQueue.h"
#include "Runnable.h"
#include "DoublyLinkedList.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class ConditionVariable;
        class JNIThreadListener;
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
         * Windows, linux implementation of the WorkerQueue.
         */
        class WorkerQueueImpl: public WorkerQueue, public Runnable
        {
          public:
            /**
             * destructor.
             */
            virtual ~WorkerQueueImpl();
            
            /**
             * Use this method to create an instance of this class.
             *
             * @param jniThreadListener the JNI thread listener, set to 0 if not
             * integrated with the JNI wrapper.
             * @param returnCode the return code.
             */
            static WorkerQueueImpl* create(utilities::JNIThreadListener* jniThreadListener,
                                           ReturnCode::Type& returnCode);
                                           
            /**
             * Add Task entries to the queue that will be invoked later in the Run
             * method.
             *
             * Note: the WorkerQueue becomes owner of the Task, i.e. task will
             * be deleted "delete" once the Task is dequeued and run() has been called.
             *
             * @param task a pointer to a Task object to enqueue
             * @param returnCode the return code
             */
            virtual void enqueue(Task* task, ReturnCode::Type& returnCode);
            
            /**
             * Removes a task that was queued for later execution if the task was
             * not already dequeued and processed.
             *
             * @param task the task to execute
             * @param returnCode ILLEGAL_ARGUMENT if the task is null
             */
            virtual void remove(Task* task, ReturnCode::Type& returnCode);
            
            /**
             * Stops the worker queue. It will stop execution of the thread
             * dequeuing tasks. It will not execute all the pending tasks before
             * returning. It will execute the current task (if any) and return
             * right after. The pending tasks will simply be deleted.
             *
             * @param returnCode the return code
             */
            virtual void shutdownNow(ReturnCode::Type& returnCode);
            
            /**
             * Called by the JNI layer (optionally used) when the JNI layer is used
             * for the first time. The JNI layer may use these callbacks to change
             * the property of the native threads, e.g. attachThreadAsDeamon when
             * onThreadStarted() is called.
             */
            virtual void setJNIThreadListener(utilities::JNIThreadListener* jniThreadListener);
            
            /**
             * Indicates if the worker queue is running.
             *
             * @param returnCode SUCCESS unless a fatal error occurs
             * @return true if the worker queue is running
             */
            virtual bool isRunning(ReturnCode::Type& returnCode);
          protected:
            /**
             * function called when the Runnable (thread) is started.
             */
            virtual ReturnCode::Type runThread();
          private:
            /**
             * Starts the worker queue. It will start the thread responsible to
             * dequeuing tasks. Upon return, the thread is running.
             *
             * PRECONDITION: The current thread must be locked over the Runnable mutex.
             *
             * @param returnCode the return code
             */
            virtual void start(ReturnCode::Type& returnCode);
            
            /**
             * constructor. Use WorkerQueueImpl::create to create an instance of this
             * class.
             *
             * @param jniThreadListener the JNI thread listener, set to 0 if not
             * integrated with the JNI wrapper.
             * @param mutex the mutex used to synchronize access to the thread state
             * @param returnCode the returnCode
             */
            WorkerQueueImpl(utilities::JNIThreadListener* jniThreadListener, Mutex* mutex,
                            ReturnCode::Type& returnCode);
                            
            /**
            * Prevent copying.
            */
            WorkerQueueImpl& operator=(WorkerQueueImpl& other);
            
            /**
             * Utility function used to clean the content of the queues.
             */
            void cleanupTaskQueue();
            /**
             * debug utility function. it prints the content of the scheduled list.
             */
            void printScheduledListContent();
            
            /**
             * Invoked when the worker queue starts up.
             */
            void onStartup();
            
            /**
             * Invoked when the worker queue shuts down.
             */
            void onShutdown();
            
            /**
             * Invoked when the worker queue transitions from being active to being idle.
             */
            void onIdle();
            
            /**
             * Invoked when the worker queue transitions from being idle to being active.
             */
            void onActive();
            
            /**
             * Flag that indicates if the thread has been requested to shut down.
             */
            bool shutdownRequested;
            /**
             * Indicates when the queue is idle.
             */
            bool idle;
            /**
             * The list of ScheduledTask(s) (that have to execute later), view this as
             * "std::list<ScheduledTask*>"
             */
            DoublyLinkedList scheduledTaskList;
            /**
             * Signaled when there is a task on the queue ready to be processed.
             */
            ConditionVariable* taskReady;
            /**
             * Indicates that the thread is running.
             */
            ConditionVariable* threadStarted;
            /**
             * JNI Thread listener. This is only used when the cpp code is used
             * by the JNI adaptor.
             */
            JNIThreadListener* jniThreadListener;
            
            char threadId[20];
            char szPreviousTaskName[128];
            
            friend class WorkerQueue;
        };
      }
    }
  }
}

#endif
