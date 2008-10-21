/*---------------------------------------------------------------------------*
 *  System.cpp                                                               *
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

#include "System.h"
#include "Singleton.h"
#include "Logger.h"
#include "Queue.h"
#include "Mutex.h"
#include "ConditionVariable.h"
#include "LibraryLoader.h"
#include "JNIThreadListener.h"
#include "WorkerQueueFactory.h"
#include "EmbeddedRecognizerImpl.h"
#include "LoggerImpl.h"
#include "WorkerQueueFactoryImpl.h"


#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

using namespace android::speech::recognition;
using namespace android::speech::recognition::impl;
using namespace android::speech::recognition::utilities;


Mutex* System::stateMutex = 0;
System* System::instance = 0;
System::ComponentInitializer System::componentInitializer;
bool System::signalRaised = false;

/**
 * This code is needed to make sure System::dispose doesn't do anything when ^C
 * is pressed. We were seeing this problem:
 *  1) User would run a java application, e.g. robustness2.java
 *  2) User would press ^C
 *  3) The process would exit. 
 *  4) The Runtime.getRuntime().addShutdownHook in System.java would invoke
 *  System::dispose()
 *  5) System::dispose would hang while trying to join the threads.
 *
 *  This was observed on Windows. It looks like we are not allowed to join a
 *  thread after ^C was pressed. 
 */
void System::signalHandler(int sig)
{
  UAPI_FN_NAME("signalHandler");

  UAPI_WARN(fn, "Signal %d was received\n", sig);


  //set the flag that System::dispose will check to know if a signal was
  //raised. 
  signalRaised = true;

  //propagate the signal.
  signal(SIGINT ,NULL);
  raise(SIGINT);
}


System::ComponentInitializer::ComponentInitializer()
{
  signal(SIGINT ,System::signalHandler);

  stateMutex = Mutex::create(false, returnCode);
  if (returnCode)
  {
    fprintf(stderr, "Could not create System::stateMutex\n");
    return;
  }
}

System::ComponentInitializer::~ComponentInitializer()
{
  delete stateMutex;
  returnCode = ReturnCode::UNKNOWN;
}


System* System::getInstance(ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("System.getInstance");
  if (componentInitializer.returnCode)
  {
    returnCode = componentInitializer.returnCode;
    return 0;
  }
  
  LockScope ls(stateMutex, returnCode);
  if (returnCode)
  {
    UAPI_ERROR(fn,"Could not lock stateMutex\n");
    return 0;
  }
  if (instance == 0)
  {
    Queue* singletons = new Queue();
    if (singletons == 0)
    {
      returnCode = ReturnCode::OUT_OF_MEMORY;
      return 0;
    }
    
    Mutex* singletonsMutex = Mutex::create(returnCode);
    if (returnCode)
    {
      UAPI_ERROR(fn,"Could not create the mutex\n");
      delete singletons;
      returnCode = returnCode;
      return 0;
    }
    
    instance = new System(singletons, singletonsMutex);
    if (instance == 0)
    {
      UAPI_ERROR(fn,"Could not create the System instance\n");
      delete singletons;
      delete singletonsMutex;
      returnCode = ReturnCode::OUT_OF_MEMORY;
      return 0;
    }
  }
  returnCode = ReturnCode::SUCCESS;
  return (System*) instance;
}

System::System(Queue* _singletons, Mutex* _singletonsMutex):
    singletons(_singletons),
    singletonsMutex(_singletonsMutex),
    disposed(false),
    shutdownRequested(false)
{}

System::~System()
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("System::~System");
    
  if (!disposed)
    dispose(returnCode);
    
  delete singletonsMutex;
  instance = 0;
}

void System::unloadSharedLibraries(ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("System::unloadSharedLibraries");
    
  if (EmbeddedRecognizerImpl::srecLoader)
  {
    ReturnCode::Type rc;
    EmbeddedRecognizerImpl::srecLoader->close(rc);
    if (rc != ReturnCode::SUCCESS)
    {
      UAPI_WARN(fn,"Failed to close library for EmbeddedRecognizerImpl\n");
      return;
    }
    delete EmbeddedRecognizerImpl::srecLoader;
    EmbeddedRecognizerImpl::srecLoader = 0;
  }
  returnCode = ReturnCode::SUCCESS;
}

void System::add(Singleton* singleton, ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("System::add");
    
  {
    LockScope ls(stateMutex, returnCode);
    if (returnCode)
    {
      UAPI_ERROR(fn,"Could not lock stateMutex\n");
      return;
    }
    if (shutdownRequested)
    {
      returnCode = ReturnCode::INVALID_STATE;
      UAPI_ERROR(fn,"Cannot create singletons while System is shutting down\n");
      return;
    }
  }
  
  LockScope ls(singletonsMutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to lock singletonsMutex\n");
    return;
  }
  UAPI_TRACE(fn,"adding singleton %p SmartProxy::Root %p\n", singleton, singleton->getRoot());
  
  SingletonInfo* singletonInfo = new SingletonInfo;
  if (singletonInfo == 0)
  {
    UAPI_ERROR(fn,"Failed to lock mutex\n");
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return;
  }
  
  singletonInfo->isWaiting = false;
  singletonInfo->singleton = singleton;
  
  singletonInfo->condVar = ConditionVariable::create(singletonsMutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to create condition variable\n");
    delete singletonInfo;
    return;
  }
  
  //UAPI_TRACE(fn,"adding SmartProxy::Root=%p (before): begin: %p, end: %p, queueSize: %lu\n", singletonInfo->singleton->getRoot(), _singletons->begin(), _singletons->end(), _singletons->size());
  singletons->push(singletonInfo, returnCode);
  //UAPI_TRACE(fn,"adding SmartProxy::Root=%p (after) : begin: %p, end: %p, queueSize: %lu\n", singletonInfo->singleton->getRoot(), _singletons->begin(), _singletons->end(), _singletons->size());
  if (returnCode)
  {
    UAPI_ERROR(fn,"Failed to add singleton info to the queue.\n");
    delete singletonInfo->condVar;
    delete singletonInfo;
    return;
  }
  
  //tell the Root of this singleton that it is now registered with System.
  singleton->getRoot()->registeredWithSystem = true;
  
  returnCode = ReturnCode::SUCCESS;
  
  UAPI_TRACE(fn,"adding singleton %p SmartProxy::Root %p: success\n", singleton, singleton->getRoot());
}

void System::remove(SmartProxy::Root* root)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("System::remove");
    
  //Lock the System mutex to protect the queue of singleton(s)
  LockScope ls(singletonsMutex, returnCode);
  if (returnCode)
  {
    UAPI_ERROR(fn,"Failed to lock singletonsMutex\n");
    return;
  }
  
  UAPI_TRACE(fn,"removing SmartProxy::Root %p\n", root);
  
  //find which entry in the queue contains the root.
  FwdIterator it(singletons->begin(), singletons->end());
  //UAPI_TRACE(fn,"_singletons: begin=%p, end=%p, queueSize=%lu\n", _singletons->begin(), _singletons->end(), _singletons->size());
  while (it.hasNext())
  {
    SingletonInfo* singletonInfo = (SingletonInfo*) it.next();
    
    //UAPI_TRACE(fn,"checking singleton->getRoot() %p\n", singletonInfo->singleton->getRoot());
    if (singletonInfo->singleton->getRoot() == root)
    {
      //we have a match.
      
      //Acquire the condition variable mutex. We want to check if
      //System::dispose was called and if it is waiting on the condition
      //variable.
      
      //check if condVar->wait() was called.
      if (singletonInfo->isWaiting)
      {
        UAPI_INFO(fn,"signaling SmartProxy::Root %p\n", root);
        
        //System::Dispose is waiting on us. Signal it, System::dispose() will
        //take care of the cleanup.
        singletonInfo->condVar->signal(returnCode);
        if (returnCode != ReturnCode::SUCCESS)
        {
          UAPI_ERROR(fn,"Cannot signal ConditionVariableLockScope\n");
          return;
        }
        return;
      }
      else
      {
        UAPI_INFO(fn,"inactive SmartProxy::Root %p\n", root);
        
        //System::dispose is not waiting. We can simply remove that entry from
        //the queue.
        //UAPI_TRACE(fn,"removing SmartProxy::Root=%p (before): begin=%p, end=%p, queueSize=%lu\n", singletonInfo->singleton->getRoot(), _singletons->begin(), _singletons->end(), _singletons->size());
        singletons->remove(singletonInfo);
        //UAPI_TRACE(fn,"removing SmartProxy::Root=%p (after) : begin=%p, end=%p, queueSize=%lu\n", singletonInfo->singleton->getRoot(), _singletons->begin(), _singletons->end(), _singletons->size());
        delete singletonInfo->condVar;
        delete singletonInfo;
        return;
      }
    }
  }//while
  
  //we should never be here!
  UAPI_ERROR(fn,"Root %p was not found in the queue\n", root);
  assert(false);
}

void System::dispose(ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("System::dispose");
 
  //check if ^C was pressed.
  if( signalRaised )
  {
    disposed = true;
    UAPI_INFO(fn,"A signal was raised, not joining the threads\n");
    return;
  }

  {
    LockScope ls(stateMutex, returnCode);
    if (returnCode)
    {
      UAPI_ERROR(fn,"Could not lock stateMutex\n");
      return;
    }
    if (shutdownRequested)
    {
      UAPI_ERROR(fn,"System is already being disposed\n");
      return;
    }
    shutdownRequested = true;
  }
  
  LockScope ls(singletonsMutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to lock mutex\n");
    return;
  }
  
  while (!singletons->empty())
  {
    SingletonInfo* singletonInfo = (SingletonInfo*) singletons->front();
    
    //lock the condition variable lock scope.
    UAPI_INFO(fn,"before shutting down SmartProxy::Root %p\n",
              singletonInfo->singleton->getRoot());
              
    // Signal the singleton to shutdown and wait for remove() to get invoked
    SmartProxy::Root* root = singletonInfo->singleton->getRoot();
    
    {
      // protect the call to getCount()
      LockScope rootls(root->getMutex(), returnCode);
      if (returnCode)
      {
        UAPI_ERROR(fn,"Failed to increment singleton root\n");
        return;
      }
      
      if (root->getCount() == 0)
      {
        // We're too late, the proxy is already being destroyed by another thread.
        // SmartProxy::onDestruction() will block on system->remove() until we release singletonsMutex.
        // in such a case, we don't want to call shutdown.
      }
      else
      {
        singletonInfo->singleton->shutdown(returnCode);
        if (returnCode != ReturnCode::SUCCESS)
        {
          UAPI_ERROR(fn,"Failed to Shutdown singleton\n");
          return;
        }
      }
      
      singletonInfo->isWaiting = true;
    }
    
#ifdef _DEBUG
    UINT32 delta = UINT32_MAX;
#else
    UINT32 delta = 5 * 1000;
#endif
    
    UAPI_INFO(fn,"System::dispose waiting for SmartProxy::Root %p to be removed\n",
              singletonInfo->singleton->getRoot());
    singletonInfo->condVar->wait(delta, returnCode);
    if (returnCode == ReturnCode::TIMEOUT)
    {
      UAPI_WARN(fn,"Wait for singleton timed out after %u milliseconds\n", delta);
      //continue and hope for the best.
      singletonInfo->isWaiting = false;
    }
    else if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"Failed waiting for singleton to get deleted\n");
      return;
    }
    
    //Because the system mutex was unlock, the content of the queue could have
    //changed. Here we want to remove from the queue singletonInfo. We have to
    //find it back because it may no longer be the first item in the queue.
    
    singletons->remove(singletonInfo);
    delete singletonInfo->condVar;
    delete singletonInfo;
    
  }
  delete singletons;

  // shut down the WorkerQueues
  WorkerQueueFactory* workerQueueFactory = WorkerQueueFactory::getInstance(returnCode);
  delete workerQueueFactory;
  
  //To make sure everything is clean, we unload the SRec library.
  
  
  /**
   * NOTE: This cannot be called or it could make the program crash. The
   * java garbage collector could do the cleanup of the objects after we have
   * unloaded the Srec shared library. If this happens, we will try to delete
   * unreferenced memory and we will crash. For now, we have a memory leak when
   * the program exits.
   * TODO: fix the exit time memory leak caused by not calling unloadSharedLibraries.
   */
  //unloadSharedLibraries(returnCode);
  //if (returnCode != ReturnCode::SUCCESS)
  //{
  //  UAPI_ERROR(fn,"Failed to unloadSharedLibraries\n");
  //  return;
  //}
  
  disposed = true;
}
