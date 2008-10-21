/*---------------------------------------------------------------------------*
 *  RecursiveMutexTest.cpp                                                   *
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

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "Runnable.h"
#include "ReturnCode.h"
#include "Mutex.h"


using namespace android::speech::recognition;
using namespace android::speech::recognition::utilities;


int  Q_RecursiveMutexTest()
{
  pthread_mutex_t     my_mutex;
  pthread_mutexattr_t my_mutex_attr;
  int                 ret;
  
  printf("\n--------------- Q_RecursiveMutexTest Running ---------------\n\n");
  
  printf("Initializing mutex attribute to default value.\n");
  ret = pthread_mutexattr_init(&my_mutex_attr);
  if (ret)
  {
    fprintf(stderr, "ERROR: Failure in pthread_mutexattr_init(&my_mutex_attr), returns %d\n", ret);
    return 1;
  }
  
  printf("Setting mutex attribute to type PTHREAD_MUTEX_RECURSIVE.\n");
  ret = pthread_mutexattr_settype(&my_mutex_attr, PTHREAD_MUTEX_RECURSIVE);
  if (ret)
  {
    fprintf(stderr, "ERROR: Failure in pthread_mutexattr_settype(&my_mutex_attr, PTHREAD_MUTEX_RECURSIVE), returns %d\n", ret);
    return 1;
  }
  
  printf("Initializing mutex with the above attributes.\n");
  ret = pthread_mutex_init(&my_mutex, &my_mutex_attr);
  if (ret)
  {
    fprintf(stderr, "ERROR: Failure in pthread_mutex_init(&my_mutex, &my_mutex_attr), returns %d\n", ret);
    return 1;
  }
  
  printf("Locking mutex the first time from application thread\n");
  ret = pthread_mutex_lock(&my_mutex);
  if (ret)
  {
    fprintf(stderr, "ERROR: Failure in pthread_mutex_lock(&my_mutex), returns %d\n", ret);
    return 1;
  }
  printf("\n***** MUTEX LOCKED *****\n\n");
  
  printf("Locking the already locked mutex from the same thread.\n");
  printf("This should not block since the mutex is recursive.\n");
  ret = pthread_mutex_lock(&my_mutex);
  if (ret)
  {
    fprintf(stderr, "ERROR: Failure in pthread_mutex_lock(&my_mutex), returns %d\n", ret);
    return 1;
  }
  
  printf("\n***** Recursive mutex did not block (as expected) *****\n\n");
  
  printf("Unlocking mutex\n");
  ret = pthread_mutex_unlock(&my_mutex);
  if (ret)
  {
    fprintf(stderr, "ERROR: Failure in pthread_mutex_unlock(&my_mutex), returns %d\n", ret);
    return 1;
  }
  printf("\n***** MUTEX UNLOCKED *****\n");
  
  printf("\n--------------- Q_RecursiveMutexTest Finished ---------------\n\n");
  
  return 0;
  
}

#ifdef ENABLE_CUSTOM_MUTEX_TEST

class MyThread : public Runnable
{
  public:
    MyThread(Mutex* mutex) : _mutex(mutex), count(0) {}
    
    virtual ReturnCode::Type runThread()
    {
      ReturnCode::Type rc;
      //lock twice
      _mutex->lock(rc);
      if (rc != ReturnCode::SUCCESS)
        return ReturnCode::THREAD_ERROR;
        
      _mutex->lock(rc);
      if (rc != ReturnCode::SUCCESS)
        return ReturnCode::THREAD_ERROR;
        
      count = 1;
      
      Runnable::sleep(5000);
      
      count = 2;
      
      _mutex->unlock(rc);
      if (rc != ReturnCode::SUCCESS)
        return ReturnCode::THREAD_ERROR;
        
      _mutex->unlock(rc);
      if (rc != ReturnCode::SUCCESS)
        return ReturnCode::THREAD_ERROR;
        
        
      return ReturnCode::SUCCESS;
    }
    
    Mutex * _mutex;
    int count;
};

int CustomMutexTest()
{
  printf("\n--------------- CustomMutexTest Running ---------------\n\n");
  
  ReturnCode::Type rc;
  
  Mutex * mutex = Mutex::create(rc);
  if (rc != ReturnCode::SUCCESS)
    return 1;
    
    
  MyThread thread(mutex);
  thread.start(rc);
  if (rc != ReturnCode::SUCCESS)
    return 1;
    
  //make sure thread starts
  Runnable::sleep(2000);
  
  
  //lock twice
  mutex->lock(rc);
  if (rc != ReturnCode::SUCCESS)
    return 1;
    
  mutex->lock(rc);
  if (rc != ReturnCode::SUCCESS)
    return 1;
    
  if (thread.count != 2)
  {
    printf("Thread count is not ok\n");
    return 1;
  }
  
  
  mutex->unlock(rc);
  if (rc != ReturnCode::SUCCESS)
    return 1;
    
  mutex->unlock(rc);
  if (rc != ReturnCode::SUCCESS)
    return 1;
    
  thread.join(rc);
  if (rc != ReturnCode::SUCCESS)
    return 1;
    
  delete mutex;
  
  printf("\n--------------- CustomMutexTest Finished ---------------\n\n");
  return 0;
}

#endif /* ENABLE_CUSTOM_MUTEX_TEST */


int main(int argc, char *argv[])
{

  /* disable stdout buffering to make sure printf() output always appear even on crash */
  int ret = setvbuf(stdout, NULL, _IONBF, 0);
  if (ret)
  {
    printf("Unable to disable buffering of stdout.\n");
  }
  
  if (Q_RecursiveMutexTest() != 0)
  {
    printf("Q_RecursiveMutexTest failed\n");
    return 1;
  }
  
#ifdef ENABLE_CUSTOM_MUTEX_TEST
  if (CustomMutexTest() != 0)
  {
    printf("Custom mutex failed\n");
    return 1;
  }
#endif
  
  printf("SUCCESS\n");
  return 0;
}
