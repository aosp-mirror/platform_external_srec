/*---------------------------------------------------------------------------*
 *  JNIHelper.h                                                              *
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

#ifndef __JNI_HELPER
#define __JNI_HELPER

#include "jni.h"
#include "jniapi.h"
#include "exports.h"
#include "Codec.h"
#include "MediaFileReader.h"
#include "CWrapperGrammarListener.h"
#include "Logger.h"
#include "Runnable.h"
#include "ConditionVariable.h"
#include "JNIThreadListener.h"
#include "Grammar.h"
#include "SpeechSynthesizer.h"


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace jni
      {
        class JVMKeepAlive;
      }
      namespace utilities
      {
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
      namespace jni
      {
        /**
         * JNI helper functions.
         */
        class JNI_EXPORT JNIHelper
        {
          public:
            
            /**
             * Returns the native codec associated with the java codec.
             *
             * @param env the JNI environment
             * @param codec java codec
             * @return native codec; undefined if env->ExceptionCheck() returns true
             */
            static Codec::Type toNativeCodec(JNIEnv* env, jobject codec);
            
            /**
             * Returns the native MediaFileReader::Mode associated with the java mode.
             *
             * @param env the JNI environment
             * @param readingMode java MediaFileReader.Mode
             * @return native MediaFileReader::Mode; undefined if env->ExceptionCheck() returns true
             */
            static MediaFileReader::ReadingMode toNativeMediaFileReaderMode(JNIEnv* env,
                jobject readingMode);
            
            /**
             * Returns the native GrammarListener object
             *
             * @param env the JNI environment
             * @param listener java GrammarListener instance
             * @throws RuntimeException if an error occurs
             */
            static GrammarListenerProxy getNativeGrammarListener(JNIEnv* env, jobject listener);
            /**
             * Returns the Java exception class that corresponds to the specified ReturnCode.
             *
             * @param env the JNI environment
             * @param obj the current java object used to retrieve the correct class
             * @param returnCode the return code
             * @return the java exception class or 0 if no match could be found
             */
            static jthrowable getJavaException(JNIEnv* env, jobject obj, ReturnCode::Type returnCode);
            
            /**
             * Throws the Java exception that corresponds to the specified ReturnCode.
             *
             * @param env the JNI environment
             * @param reference the object from which to retrieve the classloader
             * @param returnCode the return code
             */
            static void throwJavaException(JNIEnv* env, jobject reference, ReturnCode::Type returnCode);
            
            /**
             * Throws the Java Run Time Exception
             *
             * @param env the JNI environment
             * @param reference the object from which to retrieve the classloader
             * @param message to report exception
             */
            static void throwRuntimeJavaException(JNIEnv* env, jobject reference, const char *msg);
            
            /**
             * Returns the JNI environment.
             *
             * @param jvm a reference to the Java Virtual Machine
             * @return 0 in case of failure
             */
            static JNIEnv* getEnv(JavaVM* jvm);
            
            /**
             * Returns the JVM associated with a JNI environment.
             *
             * @param env the JNI environment
             * @param reference the object from which to retrieve the classloader
             * @throws RuntimeException if an error occurs
             */
            static JavaVM* getJVM(JNIEnv* env, jobject reference);
            
            /**
             * Attaches a thread to the JVM, preventing it from shutting down until detachJVM() is invoked
             * an equal number of times.
             *
             * @param returnCode the return code
             */
            static void attachJVM(ReturnCode::Type& returnCode);
            
            /**
             * Detaches a thread from the JVM, allowing it to shut down.
             *
             * @param returnCode the return code
             */
            static void detachJVM(ReturnCode::Type& returnCode);
            
            /**
             * Looks up a class using the classloader of an existing object.
             *
             * @param env the JNI environment
             * @param object the java object whose ClassLoader to use
             * @param name the class name to be loaded
             * @return 0 on failure
             */
            static jclass loadClass(JNIEnv* env, jobject object, const char* name);
            
            /**
             * Returns the native LogLevel associated with the java LogLevel.
             *
             * @param env the JNI environment
             * @param loglevel java LogLevel
             * @return native LogLevel; undefined if env->ExceptionCheck() returns true
             */
            static Logger::LogLevel toNativeLogLevel(JNIEnv* env, jobject loglevel);
        };
        
        /**
         * Class used to intercept when the native threads are started, stopped and
         * when all the threads are inactive.
         *
         * Because we are notified, it is possible to do things like
         * "AttachThreadAsDeamon" when the thread starts.
         *
         */
        class NativeThreadWatcher: public utilities::JNIThreadListener
        {
          public:
          
            /**
             * Creates a new NativeThreadListener.
             *
             * @param returnCode the return code
             * @return the new NativeThreadListener
             */
            static NativeThreadWatcher* getInstance(ReturnCode::Type& returnCode);
            
            /**
             * Called when a native thread is started.
             */
            virtual void onThreadStarted();
            
            /**
            * Called when a native thread is stopped.
            */
            virtual void onThreadStopped();
            
            /**
             * Called when a native thread is waken up from its waiting state, i.e.
             * it was told to start executing some work.
             */
            virtual void onThreadActive();
            
            /**
             * Called when a native thread is not processing anything, i.e. it is
             * in a waiting state.
             */
            virtual void onThreadInactive();
            
            /**
             * Sets the reference to the Java Virtual Machine.
             *
             * @param jvm a reference to the Java Virtual Machine
             */
            void setJVM(JavaVM* jvm);
            
          private:
            /**
             * Constructor
             */
            NativeThreadWatcher();
            
            JavaVM* jvm;
            
            static NativeThreadWatcher* instance;
        };
        
#ifdef ENABLE_JNI_KEEP_ALIVE
        /**
         * Class used to make sure the JVM will not shutdown while we are running
         * native code.
         */
        class JVMKeepAlive
        {
          public:
          class Worker: public utilities::Runnable
            {
              public:
                /**
                * Method that is called when the thread is started.
                *
                * @return SUCCESS if the thread terminated normally
                */
                virtual ReturnCode::Type runThread();
                
                Worker(JVMKeepAlive* parent, utilities::Mutex* mutex);
              private:
                JVMKeepAlive* parent;
            };
            
            /**
             * Creates a new JVMKeepAlive thread.
             *
             * @param returnCode the return code
             * @return the new JVMKeepAlive
             */
            static JVMKeepAlive* getInstance(ReturnCode::Type& returnCode);
            
            /**
             * Sets the reference to the Java Virtual Machine.
             *
             * @param jvm a reference to the Java Virtual Machine
             */
            void setJVM(JavaVM* jvm);
            
            /**
             * Attaches a thread to the JVM, preventing it from shutting down until
             * detachJVM() is invoked an equal number of times.
             *
             * @param returnCode the return code
             */
            void attach(ReturnCode::Type& returnCode);
            
            /**
             * Detaches a thread from the JVM, allowing it to shut down.
             *
             * @param returnCode the return code
             */
            void detach(ReturnCode::Type& returnCode);
            
            /**
             * Called when the thread starts running
             */
            ReturnCode::Type runThread();
            
            /**
             * Creates a new JVMKeepAlive thread.
             *
             * @param threadStarted signaled when the thread starts running
             * @param shutdownRequested indicates the thread should shut down
             * @param onShutdown signaled when the thread shuts down
             */
            JVMKeepAlive(utilities::ConditionVariable* threadStarted,
                         utilities::ConditionVariable* shutdownRequested,
                         utilities::ConditionVariable* onShutdown);
                         
            /**
             * Prevent destruction.
             */
            ~JVMKeepAlive();
            
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
             * Synchronizes access to thread state.
             */
            static utilities::Mutex* mutex;
            
            /**
             * Signaled when the thread starts running
             */
            utilities::ConditionVariable* threadStarted;
            
            /**
             * Indicates the thread should shut down.
             */
            utilities::ConditionVariable* shutdownRequested;
            
            /**
             * Indicates when the worker thread shuts down. This is necessary because
             * the behavior of multiple threads joining on the same thread is undefined.
             */
            utilities::ConditionVariable* onShutdown;
            
            /**
             * The thread that is attached to the JVM.
             */
            utilities::Runnable* worker;
            
            /**
             * pointer to the JVM
             */
            JavaVM* jvm;
            
            /**
             * Count used to know if we have to start or stop the thread.
             */
            UINT8 jvmReferences;
            
            /**
             * Singleton instance.
             */
            static JVMKeepAlive* instance;
            
            friend class NativeThreadWatcher;
        };
#endif //ENABLE_JNI_KEEP_ALIVE
      }
    }
  }
}

#endif
