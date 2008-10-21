/*---------------------------------------------------------------------------*
 *  System.h                                                                 *
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

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "exports.h"
#include "types.h"
#include "ReturnCode.h"
#include "SmartProxy.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class Singleton;
      namespace impl
      {
        class RedirectToLibrary;
      }
      namespace utilities
      {
        class Queue;
        class Mutex;
        class LockScope;
        class ConditionVariable;
        class LibraryLoader;
        class WorkerQueue;
        class WorkerQueueFactory;
        class WorkerQueueFactoryImpl;
      };
      namespace jni
      {
        class JVMKeepAlive;
      };
    }
  }
}


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      /**
       * Utility class used to destroy the system singleton instance(s) once the
       * program exits. The application can explicitly dispose the memory
       * allocated for the singletons by calling "dispose" or it can let the
       * System class take care of de-allocating the memory once the program
       * exits.
       */
      class System
      {
        public:
          /**
           * returns the singleton instance of this class.
           *
           * @param returnCode THREAD_ERROR on failure
          
           * @return the singleton instance
           */
          UAPI_EXPORT static System* getInstance(ReturnCode::Type& returnCode);
          
          /**
           * Disposes the singleton(s) and the System instance. Shutdown is not complete until
           * the System object is deleted.
           *
           * Invoking this method is optional; it is sufficient to simply delete the System object
           * but the latter mechanism cannot return errors back to the caller.
           *
           * @param returnCode TIMEOUT Waiting for singleton timeout.
           * THREAD_ERROR on failure.
           */
          UAPI_EXPORT void dispose(ReturnCode::Type& returnCode);
          
          UAPI_EXPORT ~System();
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
          
          /**
           * Method used to catch the process signals, e.g. SIGINT.
           */
          static void signalHandler(int sig);
          /**
           * Flag used to indicate if a ^C SIGINT was raised
           */
          static bool signalRaised;
       
          /**
           * structure that contains the things needed to shutdown the singletons
           */
          struct SingletonInfo
          {
            Singleton* singleton;
            utilities::ConditionVariable* condVar;
            bool isWaiting;
          };
          
          
          /**
           * Constructor
           */
          System(utilities::Queue* singletons, utilities::Mutex* singletonListMutex);
          
          /**
           * add a singleton instance to this class. Only friend can add themselves
           * to this class. Singleton add themselves such that they can be cleanup
           * by the system class at exit time.
           *
           * @param singleton a Singleton instance.
           * @param returnCode THREAD_ERROR on failure
           *
           * @see remove
           */
          UAPI_EXPORT void add(Singleton* singleton, ReturnCode::Type& returnCode);
          
          /**
           * Called when the singleton object is deleted.
          *
          * @param root the "real object" is contained in the Root. Used to map
          * back with the singleton that was passed in the add method.
          * @see add
           */
          UAPI_EXPORT void remove(SmartProxy::Root* root);
          
          /**
           * Helper function used to unload the loaded shared libraries.
           * @param returnCode the returnCode.
           */
          void unloadSharedLibraries(ReturnCode::Type& returnCode);
          
          /**
           * Array of pointer to the Singleton.
           */
          utilities::Queue* singletons; //Queue<SingletonInfo*>
          
          /**
           * Synchronizes access to System's state.
           */
          static utilities::Mutex* stateMutex;
          
          /**
           * Synchronizes access to "singletons" variable.
           */
          utilities::Mutex* singletonsMutex;
          
          /**
           * True if dispose() has been invoked on this instance.
           */
          bool disposed;
          
          static ComponentInitializer componentInitializer;
          
          /**
           * The singleton instance.
           */
          static System* instance;
          bool shutdownRequested;
          
          friend class Microphone;
          friend class DeviceSpeaker;
          friend class EmbeddedRecognizer;
          friend class SmartProxy;
          friend class jni::JVMKeepAlive;
      };
    }
  }
}

#endif
