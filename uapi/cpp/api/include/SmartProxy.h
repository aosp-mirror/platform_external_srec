/*---------------------------------------------------------------------------*
 *  SmartProxy.h                                                             *
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

#ifndef __UAPI__SMART_PROXY
#define __UAPI__SMART_PROXY

#include "exports.h"
#include "types.h"
#include "RefCounter.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
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
      class System;
      
      /**
       * Users should treat a SmartProxy as if it is a pointer to the underlying object it wraps.
       * Smart proxies are modeled after the boost <code>shared_ptr</code> design, discussed one of the the
       * URLs listed below.
       *
       * There are a few guidelines for using proxies:
       *
       * <ol>
       * <li>Retain a copy of (as opposed to a reference to) a proxy to ensure that the underlying
       *    object does not get destroyed until the proxy gets destroyed.</li>
       * <li>Pass a proxy around by reference (as opposed to pass-by-value) for efficiency.</li>
       * <li>There are two kinds of proxy objects: stack-based or heap-based. The most common type of
       *    proxy is a stack-based local variable declared inside a function body. This proxy is ideal because
       *    it is thread-safe (no other thread can access function local variables) and gets deallocated
       *    automatically at the end of the method, along with all other local variables. Heap-based proxies,
       *    on the other hand, must be explicitly deleted.</li>
       * <li>Individual proxy objects are not thread-safe so if you absolutely must access the same proxy from
       *    multiple threads (as opposed to giving each thread its own proxy) then you must synchronize access
       *    to it manually.</li>
       * <li>Factory methods should return proxies by value.</li>
       * <li>Methods should accept proxies by reference.</li>
       * <li>Never construct more than one root proxy per object.
       *    The first SmartProxy ever constructed around an object is called the "root proxy".
       *    Root proxies are constructed using
       *    <code>SmartProxy(void* object, ReturnCode::Type& returnCode)</code>.
       *    A root proxy contains the reference count of the underlying object. This means that
       *    if you construct multiple root proxies per underlying object, each root will see a difference
       *    reference count and one root will destroy the object before the other roots are done using
       *    it.</li>
       * </ol>
       *    If you absolutely must wrap the same pointer multiple times and you do not have access to
       *    existing proxies that wrap it, simply store <code>SmartProxy.getCounter()</code>
       *    alongside the pointer and construct all future proxies using
       *    <code>SmartProxy(RefCounter*)</code> instead.
       *
       * @see http://www.boost.org/libs/smart_ptr/shared_ptr.htm
       * @see http://en.wikipedia.org/wiki/Proxy_pattern
       * @see http://blogs.sun.com/nsolter/entry/reference_counting_smart_pointers_made
       */
      class SmartProxy
      {
        public:
          /**
           * The root proxy is the sole owner of the user object. It ensures that the object is
           * kept alive while normal proxies reference it and that is the object is killed when no
           * references remain.
           */
        class Root: private utilities::RefCounter
          {
              /**
               * Creates a Root for the specified object.
               *
               * @param loggingAllowed true if the object and its dependencies are allowed logging
               * @param object the underlying object to wrap
               * @param returnCode SUCCESS unless a fatal error has occured
               */
              Root(void* object, bool loggingAllowed, const char* name, ReturnCode::Type& returnCode);
              virtual ~Root();
#ifdef UAPI_MT //multi threaded
              /**
               * Returns the mutex used to synchronize access to the underlying object.
               */
              UAPI_EXPORT utilities::Mutex* getMutex() const;
              
              /**
               * protect from being called from different threads.
               */
              utilities::Mutex* mutex;
#endif
              
              /**
               * tells us if this Root was registered with the System class. With
               * this flag, we will know if the Root is a candidate for System
               * cleanup.
               */
              bool registeredWithSystem;
              char* name;
              
              friend class SmartProxy;
              friend class System;
          };
          
          /**
           * Returns the root proxy.
           *
           * @return the root proxy.
           */
          UAPI_EXPORT virtual Root* getRoot() const;
          
          /**
           * Causes the current SmartProxy to point to the same object as another SmartProxy.
           * If the operation fails then <code>!proxy</code> will return true.
           *
           * @param other the SmartProxy whose object to point to
           * @return the resulting SmartProxy
           */
          UAPI_EXPORT virtual SmartProxy& operator=(const SmartProxy& other);
          
          /**
           * Enables boolean conversion.
           *
           * @see http://www.artima.com/cppsource/safebool2.html
           */
          typedef void (*BoolConversion)();
          UAPI_EXPORT operator BoolConversion() const;
          
          /**
           * Returns true if the SmartProxy is null.
           *
           * @return true if the SmartProxy is null
           */
          UAPI_EXPORT virtual bool operator!() const;
        protected:
          /**
           * Creates a root SmartProxy for the specified pointer. If construction
           * fails then <code>!proxy</code> will return true.
           *
           * @param object a pointer to a shared object
           */
          UAPI_EXPORT explicit SmartProxy(void* object, const char* name);
          
          /**
           * Creates a root SmartProxy for the specified pointer. If construction
           * fails then <code>!proxy</code> will return true.
           *
           * @param object a pointer to a shared object
           * @param loggingAllowed true if the object and its dependencies are allowed logging
           */
          UAPI_EXPORT explicit SmartProxy(void* object, bool loggingAllowed, const char* name);
          
          /**
           * Constructs a new SmartProxy from an existing root proxy. If construction
           * fails then <code>!proxy</code> will return true.
           *
           * @param root the root proxy
           */
          UAPI_EXPORT explicit SmartProxy(Root* root);
          
          /**
           * Constructs a copy of an existing SmartProxy. If construction
           * failed then <code>!proxy</code> will return true.
           */
          UAPI_EXPORT SmartProxy(const SmartProxy& other);
          
          /**
           * Enables the construction of arrays of proxies. The proxy is initialized to null.
           */
          UAPI_EXPORT SmartProxy();
          
          /**
           * Destroys the SmartProxy.
           */
          UAPI_EXPORT virtual ~SmartProxy();
          
          /**
           * Deletes the underlying object. Subclasses must override this method as only they know the actual
           * type of the underlying object.
           *
           * @param object the underlying object
           */
          UAPI_EXPORT virtual void deleteObject(void* object);
          
          /**
           * Returns the underlying object.
           *
           * @return the underlying object
           */
          UAPI_EXPORT void* getObject() const;
          
          /**
           * Invoked by the SmartProxy destructor.
           */
          UAPI_EXPORT virtual void onDestruction();
        private:
          /**
           * Prevent dereferencing.
           */
          utilities::RefCounter& operator*();
          
          
          /**
           * The underlying object.
           */
          Root* root;

          friend class System;
      };
    }
  }
}

#define DECLARE_SMARTPROXY(ExportSymbol, Proxy, SuperProxy, Interface) \
  class Proxy: public SuperProxy \
  { \
    public: \
      /* Creates a root SmartProxy for the specified pointer */ \
      ExportSymbol explicit Proxy(Interface* object); \
      /* Creates a root SmartProxy for the specified pointer */ \
      ExportSymbol explicit Proxy(Interface* object, bool loggingAllowed); \
      /* Constructs a new SmartProxy from an existing root proxy */ \
      ExportSymbol explicit Proxy(Root* root); \
      /* Constructs a copy of an existing SmartProxy */ \
      ExportSymbol Proxy(const Proxy& other); \
      /* Enables the construction of arrays of proxies */ \
      ExportSymbol Proxy(); \
      /* Unsafe cast to a subclass type */ \
      ExportSymbol explicit Proxy(const SmartProxy& other); \
      /* Delegate method invocation to underlying object */ \
      ExportSymbol Interface* operator->() const; \
      /* Destroys the proxy */ \
      ExportSymbol virtual ~Proxy(); \
    protected: \
      /* deletes the underlying object */ \
      virtual void deleteObject(void* object); \
      /* Creates a root SmartProxy for the specified pointer */ \
      ExportSymbol explicit Proxy(Interface* object, const char* name); \
      /* Creates a root SmartProxy for the specified pointer */ \
      ExportSymbol explicit Proxy(Interface* object, bool loggingAllowed, const char* name); \
  };

#define DEFINE_SMARTPROXY(Namespace, Proxy, SuperProxy, Interface) \
  Namespace::Proxy::Proxy(Interface* object): \
      SuperProxy(object, #Proxy) \
  {} \
  \
  Namespace::Proxy::Proxy(Interface* object, const char* name): \
      SuperProxy(object, name) \
  {} \
  \
  Namespace::Proxy::Proxy(Interface* object, bool loggingAllowed): \
      SuperProxy(object, loggingAllowed, #Proxy) \
  {} \
  \
  Namespace::Proxy::Proxy(Interface* object, bool loggingAllowed, const char* name): \
      SuperProxy(object, loggingAllowed, name) \
  {} \
  \
  Namespace::Proxy::Proxy(Root* root): \
      SuperProxy(root) \
  {} \
  \
  Namespace::Proxy::Proxy(const Namespace::Proxy& other): \
      SuperProxy(other) \
  {} \
  \
  Namespace::Proxy::Proxy(const SmartProxy& other): \
      SuperProxy(other) \
  {} \
  \
  Namespace::Proxy::Proxy() \
  {} \
  \
  Interface* Namespace::Proxy::operator->() const \
  { \
    return (Interface*) getObject(); \
  } \
  \
  void Namespace::Proxy::deleteObject(void* object) \
  { \
    delete (Interface*) object; \
  } \
  Namespace::Proxy::~Proxy() \
  { \
    onDestruction(); \
  }

#endif
