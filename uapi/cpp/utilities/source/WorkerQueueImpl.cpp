/*---------------------------------------------------------------------------*
 *  WorkerQueueImpl.cpp                                                      *
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


//Memory leak detection
#if defined(_DEBUG) && defined(_WIN32)
#include "crtdbg.h"
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif

#ifdef UAPI_WIN32
// Do not warn on "while(true)"
#  pragma warning (disable: 4127)
#endif

#include "ReturnCode.h"
#include "WorkerQueueImpl.h"
#include "Task.h"
#include "LoggerImpl.h"
#include "TimeInstant.h"
#include "WorkerQueueFactory.h"
#include "JNIThreadListener.h"
#include "Mutex.h"
#include "ConditionVariable.h"

#include <assert.h>
#include <limits.h>
#include <string.h>


/**
 * If a task is executed cstMaxTimeoutDelay after it was supposed to, a log will
 * get generated. I have lowered the time out so we can see if this is occuring
 * at all. SteveR
 */
static const int cstMaxTimeoutDelay = 1000;

using namespace android::speech::recognition;
using namespace android::speech::recognition::utilities;


WorkerQueueImpl* WorkerQueueImpl::create(utilities::JNIThreadListener *jniThreadListener,
    ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("WorkerQueueImpl::create");
    
  Mutex* mutex = Mutex::create(returnCode);
  if (returnCode)
  {
    UAPI_ERROR(fn,"Failed to create mutex\n");
    return 0;
  }
  WorkerQueueImpl* result = new WorkerQueueImpl(jniThreadListener, mutex, returnCode);
  if (!result || returnCode)
  {
    delete mutex;
    delete result;
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return 0;
  }
  returnCode = ReturnCode::SUCCESS;
  return result;
}

WorkerQueueImpl::WorkerQueueImpl(utilities::JNIThreadListener* jniThreadListener, Mutex* mutex,
                                 ReturnCode::Type& returnCode):
    WorkerQueue(),
    Runnable(mutex),
    shutdownRequested(false),
    idle(true),
    taskReady(0),
    threadStarted(0),
    jniThreadListener(jniThreadListener)
{
  strcpy(threadId, "");
  strcpy(szPreviousTaskName,"");
  UAPI_FN_SCOPE("WorkerQueueImpl::WorkerQueueImpl");
    
  taskReady = ConditionVariable::create(mutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
    UAPI_ERROR(fn,"Failed to create taskReady\n");
  threadStarted = ConditionVariable::create(mutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
    UAPI_ERROR(fn,"Failed to create threadStarted\n");
}

WorkerQueueImpl::~WorkerQueueImpl()
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("WorkerQueueImpl::~WorkerQueueImpl");
  
  LockScope ls(mutex, returnCode);
  shutdownNow(returnCode);
  
  delete taskReady;
  delete threadStarted;
}

void WorkerQueueImpl::cleanupTaskQueue()
{
  UAPI_FN_SCOPE("WorkerQueueImpl::cleanupTaskQueue");
    
  FwdIterator it2(scheduledTaskList.begin(), scheduledTaskList.end());
  while (it2.hasNext())
  {
    ScheduledTask* task = (ScheduledTask*) it2.next();
    delete task;
  }
  scheduledTaskList.clear();
}

void WorkerQueueImpl::setJNIThreadListener(utilities::JNIThreadListener* jniThreadListener)
{
  jniThreadListener = jniThreadListener;
}

void WorkerQueueImpl::enqueue(Task* task, ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("WorkerQueueImpl::enqueue");
    
  if (task == 0)
  {
    UAPI_ERROR(fn,"A Task object could not be created\n");
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return;
  }
  
  LockScope ls(mutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to create condition variable lock scope\n");
    return;
  }
  if (shutdownRequested)
  {
    UAPI_INFO(fn,"Not queuing Task %p:\"%s\", we are shutting down\n",
              task, task->m_name);
    returnCode = ReturnCode::SUCCESS;
    return;
  }
  if (!running)
  {
    // Start WorkerQueue thread the first time a task is queued because that thread
    // locks on the System mutex and WorkerQueueFactoryImpl::getWorkerQueue() might
    // get invoked from a second thread which already holds the mutex, leading to a deadlock.
    start(returnCode);
    if (returnCode)
      return;
  }
  
  {
    //This is a ScheduledTask... to be executed later.
    task->m_expire = TimeInstant::now();
    task->m_expire = task->m_expire.plus(task->getTimeout());
    
    //we now need to find out where we can insert this new ScheduledTask inside
    //the list of ScheduledTask.
    DoublyLinkedNode* insertPosition = (DoublyLinkedNode*) scheduledTaskList.end();
    FwdIterator it(scheduledTaskList.begin(), scheduledTaskList.end());
    while (it.hasNext())
    {
      ScheduledTask* pCurrentTask = (ScheduledTask*) it.next();
      
      // find the insertion point
      // delta needs to be an int to handle wrapping
      INT32 delta = task->m_expire.minus(pCurrentTask->m_expire, returnCode);
      if (returnCode)
      {
        UAPI_INFO(fn,"TimeInstant.minus() error: %s\n", ReturnCode::toString(returnCode));
        continue;
      }
      if (delta < 0)
      {
        insertPosition = (DoublyLinkedNode*) it.current(); //current position
        break;
      }
      else if (delta >= 0)
        insertPosition = (DoublyLinkedNode*) it.current()->next;  //after current position
    }
    UAPI_INFO(fn,"Queuing ScheduledTask %p:\"%s\" timeout %ld\n",
              task, task->m_name, task->getTimeout());
    //insert before pCurrentTask
    scheduledTaskList.insert(insertPosition, task);
  }
  
  if (idle)
  {
    // runThread() sets idle=true. enqueue() sets idle=false
    idle = false;
    onActive();
  }
  
  // Wake up the thread if it's waiting for new tasks
  UAPI_INFO(fn,"Signaling the thread\n");
  taskReady->signal(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to signal taskReady\n");
    return;
  }
  returnCode = ReturnCode::SUCCESS;
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to signal waiting thread\n");
    return;
  }
  returnCode = ReturnCode::SUCCESS;
}

void WorkerQueueImpl::remove(Task* task, ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("WorkerQueueImpl::remove");
    
  LockScope ls(mutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to create condition variable lock scope\n");
    return;
  }
  
  //check in which queue the Task is stored.
  {
    FwdIterator it(scheduledTaskList.begin(), scheduledTaskList.end());
    while (it.hasNext())
    {
      Task* entry = (Task*) it.next();
      if (entry == task)
      {
        scheduledTaskList.remove(entry);
        delete entry;
        break;
      }
    }
  }
  
  returnCode = ReturnCode::SUCCESS;
}

void WorkerQueueImpl::start(ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("WorkerQueueImpl::start");
    
  Runnable::start(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to start worker queue: %s\n", ReturnCode::toString(returnCode));
    return;
  }
  
  //wait until the thread is completely started. We want to make sure
  //taskReady->wait() was called before we exit the return function. This
  //is to prevent enqueue to be called before the thread is in a waiting
  //state.
  threadStarted->wait(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed on threadStarted->wait(): %s\n", ReturnCode::toString(returnCode));
    return;
  }
}

void WorkerQueueImpl::shutdownNow(ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("WorkerQueueImpl::shutdown");
  UAPI_INFO(fn,"about to shutdown thread %s\n", threadId);
  
  LockScope ls(mutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to create mutex lock scope\n");
    return;
  }
  if (shutdownRequested)
  {
    returnCode = ReturnCode::SUCCESS;
    return;
  }
  shutdownRequested = true;
  
  // Wake up the thread if it's waiting for new tasks
  UAPI_INFO(fn,"Signaling the thread\n");
  taskReady->signal(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to signal taskReady\n");
    return;
  }
  
  // All thread to wake up
  ls.cancel(returnCode);
  if (returnCode)
  {
    UAPI_ERROR(fn,"Failed to cancel the mutex lock scope\n");
    return;
  }
  
  join(returnCode);
}

ReturnCode::Type WorkerQueueImpl::runThread()
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("WorkerQueueImpl::runThread");
    
  Runnable::getCurrentThreadId(threadId, 20, returnCode);
  if (returnCode)
  {
    UAPI_ERROR(fn,"Runnable::getcurrentThreadId() failed: %s\n", ReturnCode::toString(returnCode));
    return returnCode;
  }
  assert(!shutdownRequested);
  assert(scheduledTaskList.empty());
  onStartup();
  
  {
    LockScope ls(mutex, returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"Failed to create condition variable lock scope\n");
      onShutdown();
      return returnCode;
    }
    
    assert(idle); // Set by the constructor
    idle = false;
    onActive();
    
    threadStarted->signal(returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"Failed to signal threadStarted\n");
      onShutdown();
      return returnCode;
    }
  }
  
  while (true)
  {
    TimeInstant now = TimeInstant::now();
    
    //protect the queue and the list.
    LockScope ls(mutex, returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"Failed to create condition variable lock scope\n");
      onShutdown();
      return returnCode;
    }
    
    if (shutdownRequested)
    {
      onShutdown();
      return ReturnCode::SUCCESS;
    }
    
    //printScheduledListContent();
    
    ReturnCode::Type waitReturnCode;
    Task* scheduledTask = (Task*) scheduledTaskList.front();
    if (scheduledTask)
    {
      INT32 delta = now.minus(scheduledTask->m_expire, returnCode);
      if (returnCode)
      {
        UAPI_INFO(fn,"TimeInstant.minus() error: %s\n", ReturnCode::toString(returnCode));
        onShutdown();
        return returnCode;
      }

      if (delta >= 0)
      {
        // timer already expired!!!
        const bool timeoutExceeded = (delta > cstMaxTimeoutDelay);
        if (timeoutExceeded)
        {
          UAPI_WARN(fn,"calling %d ms ScheduledTask %s %d milliseconds too late. "
                       "Previous task was %s\n", scheduledTask->getTimeout(), 
                       scheduledTask->m_name, delta, szPreviousTaskName);
        }
        
        // remove from scheduled task list, it is the first item.
        scheduledTaskList.pop_front();
        
        //unlock the mutex
        ls.cancel(returnCode);
        if (returnCode != ReturnCode::SUCCESS)
        {
          UAPI_ERROR(fn,"Failed to cancel condition variable lock scope\n");
          onShutdown();
          return returnCode;
        }
        
        //call the Run function of the Task object
        UAPI_INFO(fn,"Running ScheduledTask %p:\"%s\"\n", scheduledTask, scheduledTask->m_name);

        //for debugging
        strncpy(szPreviousTaskName, scheduledTask->m_name, sizeof(szPreviousTaskName));

        //call the Run function of the Task object
        scheduledTask->run();
        delete scheduledTask;
        
        //go back and try to dequeue either an immediate task or the next
        //scheduled task.
        continue;
      }
      else
      {
        UINT32 delay = scheduledTask->m_expire.minus(now, returnCode);
        if (returnCode)
        {
          UAPI_INFO(fn,"TimeInstant.minus() error: %s\n", ReturnCode::toString(returnCode));
          onShutdown();
          return returnCode;
        }
        UAPI_TRACE(fn,"thread waiting for %ld msec\n", delay);
        
        // NOTE: we check for waitReturnCode below
        taskReady->wait(delay, waitReturnCode);
        if (waitReturnCode == ReturnCode::TIMEOUT)
        {
          //TimedWaitMs expired because the timeout expired. A ScheduledTask must
          //be ready to be run()
          
          //the real wait time
          UINT32 realWait = TimeInstant::now().minus(now, returnCode);
          if (returnCode)
          {
            UAPI_INFO(fn,"TimeInstant.minus() error: %s\n", ReturnCode::toString(returnCode));
            onShutdown();
            return returnCode;
          }
          if (realWait > delay + cstMaxTimeoutDelay)
          {
            UAPI_WARN(fn,"TimedWaitMs timeout in %u ms, too slow by %u ms\n", realWait,
                      (realWait - delay));
          }
        }
      }
    }
    else
    {
      // runThread() sets idle=true. enqueue() sets idle=false
      idle = true;
      onIdle();
      // NOTE: we check for waitReturnCode below
      taskReady->wait(waitReturnCode);
    }
    
    // If we've been told to stop, do it
    if (shutdownRequested)
    {
      onShutdown();
      return ReturnCode::SUCCESS;
    }
    
    //unlock the mutex we don't need to protect anything beyond this point.
    ls.cancel(returnCode);
    if (returnCode)
    {
      UAPI_ERROR(fn,"Failed to cancel condition variable lock scope\n");
      onShutdown();
      return returnCode;
    }
    
    if (waitReturnCode == ReturnCode::TIMEOUT)
    {
      // we will run() the ScheduledTask when we go back at the beginning of the
      // loop.
      continue;
    }
    else if (waitReturnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"TimedWaitMs failed!!! Getting out of the thread.\n");
      onShutdown();
      return ReturnCode::THREAD_ERROR;
    }
  } // while (true) loop
  
  onShutdown();
  return ReturnCode::SUCCESS;
}


void WorkerQueueImpl::printScheduledListContent()
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("WorkerQueueImpl::printScheduledListContent");
    
  UAPI_INFO(fn,"Scheduled task list has %ld items\n", scheduledTaskList.size());
  FwdIterator it(scheduledTaskList.begin(), scheduledTaskList.end());
  while (it.hasNext())
  {
    Task* pTask = (Task*) it.next();
    
    UINT32 delta = pTask->m_expire.minus(TimeInstant::now(), returnCode);
    if (returnCode)
    {
      UAPI_INFO(fn,"TimeInstant.minus() error: %s\n", ReturnCode::toString(returnCode));
      continue;
    }
    UAPI_INFO(fn,"Task %p name %s expires %ld milliseconds\n", pTask, pTask->m_name,
              delta);
  }
}

void WorkerQueueImpl::onStartup()
{
  UAPI_FN_SCOPE("WorkerQueueImpl::onStartup");
    
  if (jniThreadListener)
    jniThreadListener->onThreadStarted();
}

void WorkerQueueImpl::onShutdown()
{
  UAPI_FN_SCOPE("WorkerQueueImpl::onShutdown");
    
  cleanupTaskQueue();
  if (jniThreadListener)
    jniThreadListener->onThreadStopped();
    
  // Detach JNIKeepAlive if necessary
  if (!idle)
  {
    idle = true;
    onIdle();
  }
}

void WorkerQueueImpl::onActive()
{
  if (jniThreadListener)
    jniThreadListener->onThreadActive();
}

void WorkerQueueImpl::onIdle()
{
  assert(scheduledTaskList.empty());
  if (jniThreadListener)
    jniThreadListener->onThreadInactive();
}

bool WorkerQueueImpl::isRunning(ReturnCode::Type& returnCode)
{
  return Runnable::isRunning(returnCode);
}
