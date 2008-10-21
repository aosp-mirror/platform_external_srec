/*---------------------------------------------------------------------------*
 *  WorkerQueueFactoryImpl.cpp                                               *
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

#include "WorkerQueueFactoryImpl.h"
#include "LoggerImpl.h"
#include "System.h"
#include "WorkerQueueImpl.h"
#include "Mutex.h"

#include <stdio.h>

using namespace android::speech::recognition::utilities;


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      WorkerQueueFactoryImpl* WorkerQueueFactoryImpl::instance = 0;
      Mutex* WorkerQueueFactoryImpl::mutex = 0;
      WorkerQueueFactoryImpl::ComponentInitializer WorkerQueueFactoryImpl::componentInitializer;
      
      WorkerQueueFactoryImpl::ComponentInitializer::ComponentInitializer()
      {
        mutex = Mutex::create(false, returnCode);
        if (returnCode)
        {
          fprintf(stderr, "Could not create WorkerQueueFactoryImpl::mutex\n");
          return;
        }
      }
      
      WorkerQueueFactoryImpl::ComponentInitializer::~ComponentInitializer()
      {
        delete mutex;
        returnCode = ReturnCode::UNKNOWN;
      }
      
      WorkerQueueFactory* WorkerQueueFactory::getInstance(ReturnCode::Type& returnCode)
      {
        UAPI_FN_SCOPE("WorkerQueueFactory::getInstance");
        
        if (WorkerQueueFactoryImpl::componentInitializer.returnCode)
        {
          returnCode = WorkerQueueFactoryImpl::componentInitializer.returnCode;
          return 0;
        }
        
        //we have to protect the construction of "instance". Make sure we don't have
        //multiple threads calling getInstance() while "instance" is equal to 0.
        LockScope lock(WorkerQueueFactoryImpl::mutex, returnCode);
        if (returnCode != ReturnCode::SUCCESS)
        {
          UAPI_ERROR(fn,"Failed to create LockScope of mutex\n");
          return 0;
        }
        
        //it's now safe to check if instance == 0
        if (WorkerQueueFactoryImpl::instance == 0)
        {
          WorkerQueueFactoryImpl* result = new WorkerQueueFactoryImpl();
          if (!result)
            return 0;
          WorkerQueueFactoryImpl::instance = result;
        }
        returnCode = ReturnCode::SUCCESS;
        return WorkerQueueFactoryImpl::instance;
      }
      
      WorkerQueueFactoryImpl::WorkerQueueFactoryImpl():
          workerQueueList(),
          maxNumWorkerQueues(5),
          indexToReturn(0),
          jniThreadListener(0)
      {
        UAPI_FN_NAME("WorkerQueueFactory::WorkerQueueFactoryImpl");
        
        UAPI_TRACE(fn,"this=%p\n", this);
      }
      
      WorkerQueueFactoryImpl::~WorkerQueueFactoryImpl()
      {
        ReturnCode::Type returnCode;
        UAPI_FN_SCOPE("WorkerQueueFactory::~WorkerQueueFactoryImpl");

        UAPI_TRACE(fn,"this=%p\n", this);
        
        if (workerQueueList.empty() == false)
        {
          //cleanup the worker queues
          FwdIterator it(workerQueueList.begin(), workerQueueList.end());
          while (it.hasNext())
          {
            //we have to terminate the thread of this WorkerQueueImpl before we
            //can destroy it.
            WorkerQueueImpl* workerQ = (WorkerQueueImpl*) it.next();
            
            //shut down the workerQueue (thread).
            UAPI_INFO(fn,"Shutting down worker queue\n");
            workerQ->shutdownNow(returnCode);
            if (returnCode)
              UAPI_WARN(fn,"Failed to shutdown worker queue impl\n");
            UAPI_INFO(fn,"Shutting down worker queue SUCCESS\n");
            delete workerQ;
          }
          workerQueueList.clear();
        }
        instance = 0;
      }
      
      WorkerQueue* WorkerQueueFactoryImpl::getWorkerQueue(ReturnCode::Type& returnCode)
      {
        UAPI_FN_SCOPE("WorkerQueueFactoryImpl::getWorkerQueue");

          
        //we have to protect the construction of "instance". Make sure we don't have
        //multiple threads calling getInstance() while "instance" is equal to 0.
        LockScope lock(mutex, returnCode);
        if (returnCode != ReturnCode::SUCCESS)
        {
          UAPI_ERROR(fn,"Failed to create LockScope of static mutex.\n");
          return 0;
        }
        
        while (true)
        {
          if (workerQueueList.size() < maxNumWorkerQueues)
          {
            WorkerQueueImpl* newQ = WorkerQueueImpl::create(jniThreadListener, returnCode);
            if (returnCode != ReturnCode::SUCCESS)
            {
              UAPI_ERROR(fn,"Could not construct a WorkerQueueImpl\n");
              return 0;
            }
            workerQueueList.push_back(newQ);
            
            returnCode = ReturnCode::SUCCESS;
            UAPI_INFO(fn,"Returned WorkerQueueImpl %p\n", newQ);
            return newQ;
          }
          else
          {
            //the maximum allowed number of worker queues was created.
            ++indexToReturn;
            indexToReturn %= workerQueueList.size();
            
            returnCode = ReturnCode::SUCCESS;
            UAPI_INFO(fn,"Returned WorkerQueueImpl %p\n",
                      (WorkerQueue*)workerQueueList[indexToReturn]);
            WorkerQueue* result = (WorkerQueue*) workerQueueList[indexToReturn];
            bool running = result->isRunning(returnCode);
            if (returnCode || !running)
            {
              workerQueueList.remove(result);
              continue;
            }
            return result;
          }
        }
      }
      
      void WorkerQueueFactoryImpl::setPoolSize(UINT8 numWorkerQueue, ReturnCode::Type& returnCode)
      {
        //TODO make this configurable through a parameter.
        maxNumWorkerQueues = numWorkerQueue;
        returnCode = ReturnCode::SUCCESS;
      }
      
      void WorkerQueueFactoryImpl::setJNIThreadListener(JNIThreadListener* _jniThreadListener)
      {
        jniThreadListener = _jniThreadListener;
      }
      
      JNIThreadListener* WorkerQueueFactoryImpl::getJNIThreadListener()
      {
        return jniThreadListener;
      }
    }
  }
}
