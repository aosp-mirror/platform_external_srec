/*---------------------------------------------------------------------------*
 *  SmartProxy.cpp                                                           *
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

#include "SmartProxy.h"
#include "RefCounter.h"
#include "ReturnCode.h"
#include "Logger.h"
#include "System.h"
#include "Mutex.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

using namespace android::speech::recognition;
using namespace android::speech::recognition::utilities;


SmartProxy::Root::Root(void* object, bool _loggingAllowed, const char* _name, ReturnCode::Type& returnCode):
    RefCounter(object, _loggingAllowed, returnCode),
    registeredWithSystem(false)
{
  UAPI_FN_NAME("SmartProxy::Root::Root");
  
  name = new char[strlen(_name)+1];
  strcpy(name, _name);
  
  mutex = Mutex::create(loggingAllowed, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    const char* message = "Could not create mutex\n";
    if (loggingAllowed)
    {
      UAPI_ERROR(fn,message);
    }
    else
    {
      fprintf(stderr, message);
    }
    return;
  }
}

SmartProxy::Root::~Root()
{
  delete[] name;
  if (mutex)
    delete mutex;
}

#ifdef UAPI_MT

Mutex* SmartProxy::Root::getMutex() const
{
  return mutex;
}

#endif

SmartProxy::SmartProxy(void* object, const char* name)
{
  ReturnCode::Type returnCode = ReturnCode::SUCCESS;
  root = new Root(object, true, name, returnCode);
  if (!root)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return;
  }
  else if (returnCode)
  {
    delete root;
    root = 0;
  }
}

SmartProxy::SmartProxy(void* object, bool loggingAllowed, const char* name)
{
  ReturnCode::Type returnCode = ReturnCode::SUCCESS;
  if (!object)
  {
    root = 0;
    return;
  }
  root = new Root(object, loggingAllowed, name, returnCode);
  if (!root)
    return;
  else if (returnCode)
  {
    delete root;
    root = 0;
  }
}

SmartProxy::SmartProxy():
    root(0)
{}

SmartProxy::SmartProxy(Root* _root):
    root(_root)
{
  if (root)
  {
    // The following assert ensures we aren't increasing the reference count to an object from
    // inside its destructor.
    assert(root->getCount() > 0);
    
    ReturnCode::Type returnCode;
    
    //we want to protect the call to increment
    LockScope ls(root->getMutex(), returnCode);
    if (returnCode)
    {
      fprintf(stderr, "SmartProxy::SmartProxy - failed to create LockScope\n");
      root = 0;
      return;
    }
    
    root->increment(returnCode);
    if (returnCode)
      root = 0;
  }
}

SmartProxy::~SmartProxy()
{
}

SmartProxy::operator SmartProxy::BoolConversion() const
{
  if (root)
    return reinterpret_cast<BoolConversion>(1);
  else
    return 0;
}

SmartProxy& SmartProxy::operator=(const SmartProxy & other)
{
  if (&other == this)
    return *this;
    
  Root* oldRoot = root;
  ReturnCode::Type returnCode;
  if (oldRoot)
  {
  
    LockScope ls(oldRoot->getMutex(), returnCode);
    assert(!returnCode);
    if (returnCode)
    {
      fprintf(stderr, "SmartProxy::operator= - failed to create LockScope oldRoot\n");
      root = 0;
      return *this;
    }
    
    ARRAY_LIMIT oldCount = oldRoot->decrement(returnCode);
    assert(!returnCode);
    if (returnCode)
    {
      root = 0;
      return *this;
    }
    if (oldCount == 0)
    {
      deleteObject(oldRoot->getObject());
      ls.cancel(returnCode); //must unlock before we delete the object that contains the lock.
      assert(!returnCode);
      if (returnCode)
      {
        root = 0;
        return *this;
      }
      delete oldRoot;
    }
  }
  this->root = other.root;
  Root* newRoot = root;
  if (newRoot)
  {
  
    //we want to protect the call to increment
    LockScope ls(newRoot->getMutex(), returnCode);
    assert(!returnCode);
    if (returnCode)
    {
      fprintf(stderr, "SmartProxy::operator= - failed to create LockScope newRoot\n");
      root = 0;
      return *this;
    }
    
    newRoot->increment(returnCode);
    assert(!returnCode);
    if (returnCode)
    {
      root = 0;
      return *this;
    }
  }
  return *this;
}

SmartProxy::SmartProxy(const SmartProxy& other)
{
  root = other.getRoot();
  if (!root)
    return;
  ReturnCode::Type returnCode;
  
  //we want to protect the call to increment
  LockScope ls(root->getMutex(), returnCode);
  if (returnCode)
  {
    fprintf(stderr, "SmartProxy::SmartProxy2 - failed to create LockScope\n");
    root = 0;
    return;
  }
  
  root->increment(returnCode);
  if (returnCode)
    root = 0;
}

bool SmartProxy::operator!() const
{
  return !root;
}

SmartProxy::Root* SmartProxy::getRoot() const
{
  return root;
}

void* SmartProxy::getObject() const
{
  UAPI_FN_NAME("SmartProxy::getObject");

  if ( root == 0 )
    UAPI_ERROR( fn, "NULL Object Crash Iminent");
  return root->getObject();
}

void SmartProxy::onDestruction()
{
  ReturnCode::Type returnCode;

  UAPI_FN_NAME("SmartProxy::onDestruction");

  if (!root)
  {
    return;
  }
  bool loggingAllowed = root->isLoggingAllowed();

  if (loggingAllowed)
  {
    UAPI_TRACE(fn,"Root %s (%p), Count %d\n", root->name, root, root->getCount());
  }
  else
  {
    // Omit trace logging
    // fprintf(stderr, "Root %p, Count %d\n", root, root->getCount());
  }
  
  //protect the call to root->decrement
  LockScope ls(root->getMutex(), returnCode);
  if (returnCode)
  {
    if (loggingAllowed)
    {
      UAPI_ERROR(fn,"SmartProxy::onDestruction(): cannot lock root->getMutex\n");
    }
    else
    {
      fprintf(stderr, "SmartProxy::onDestruction(): cannot lock root->getMutex\n");
    }
    return;
  }
  
  ARRAY_LIMIT count = root->decrement(returnCode);
  if (returnCode)
  {
    return;
  }
  if (count == 0)
  {
    ls.cancel(returnCode);
    if (returnCode)
    {
      if (loggingAllowed)
      {
        UAPI_ERROR(fn,"SmartProxy::onDestruction(): failed to cancel ls\n");
      }
      else
      {
        fprintf(stderr, "SmartProxy::onDestruction(): failed to cancel ls\n");
      }
      return;
    }
    if (root->registeredWithSystem)
    {
      //This Root was added with the System class. Now that we are about the
      //delete the object, it is time to remove it from System.
      if (loggingAllowed)
        UAPI_INFO(fn,"Remove object from System before deleting it\n");
        
      System* system = System::getInstance(returnCode);
      if (returnCode)
      {
        if (loggingAllowed)
        {
          UAPI_ERROR(fn,"SmartProxy::onDestruction(): System::getInstance() "
                     "failed: %s\n", ReturnCode::toString(returnCode));
        }
        else
        {
          fprintf(stderr, "SmartProxy::onDestruction(): System::getInstance() "
                  "failed: %s\n", ReturnCode::toString(returnCode));
        }
        return;
      }
      system->remove(root);
    }
    deleteObject(root->getObject());
    delete root;
  }
  // Prevent superclass destructors from invoking this again
  root = 0;
}

// SmartProxy shouldn't be instantiable but this enables us to define Proxy::Proxy(SuperProxy)
// in the macro
void SmartProxy::deleteObject(void*)
{
  assert(false);
}
