/*---------------------------------------------------------------------------*
 *  EmbeddedRecognizerImpl.cpp                                               *
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

#include <string.h>

//Memory leak detection
#if defined(_DEBUG) && defined(_WIN32)
#include "crtdbg.h"
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif

#include "EmbeddedRecognizerImpl.h"
#include "Logger.h"
#include "Mutex.h"
#include <stdio.h>
#include "LibraryLoader.h"
#include "Grammar.h"
#include "System.h"

using namespace android::speech::recognition::utilities;

#ifndef RECOGNIZER_DYNAMICALLY_LOADED
// shouldn't re-declare here, but is defined in a different compile domain, intended to be dynamically linked
extern "C" android::speech::recognition::EmbeddedRecognizerProxy* ConfigureEmbeddedRecognizer(
    const char* config, android::speech::recognition::ReturnCode::Type& returnCode);
#endif
    
namespace android
{
  namespace speech
  {
    namespace recognition
    {
      EmbeddedRecognizerProxy EmbeddedRecognizer::getInstance(ReturnCode::Type& returnCode)
      {
        UAPI_FN_SCOPE("EmbeddedRecognizer::getInstance");
        if (impl::EmbeddedRecognizerImpl::componentInitializer.returnCode)
        {
          returnCode = impl::EmbeddedRecognizerImpl::componentInitializer.returnCode;
          return EmbeddedRecognizerProxy();
        }
        
        //we have to protect the construction of "instance". Make sure we don't have
        //multiple threads calling getInstance() while "instance" is equal to 0.
        LockScope lock(impl::EmbeddedRecognizerImpl::mutex, returnCode);
        if (returnCode)
        {
          UAPI_ERROR(fn,"Failed to create LockScope of EmbeddedRecognizerImpl::mutex\n");
          return EmbeddedRecognizerProxy();
        }
        
        //it's now safe to check if instance == 0
        if (impl::EmbeddedRecognizerImpl::instance == 0)
        {
          impl::EmbeddedRecognizerImpl* temp = new impl::EmbeddedRecognizerImpl();
          if (temp == 0)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return EmbeddedRecognizerProxy();
          }
          else if (returnCode)
          {
            delete temp;
            return EmbeddedRecognizerProxy();
          }
          impl::EmbeddedRecognizerImpl::instance = temp;
          
          EmbeddedRecognizerProxy result(impl::EmbeddedRecognizerImpl::instance);
          if (!result)
            returnCode = ReturnCode::OUT_OF_MEMORY;
          impl::EmbeddedRecognizerImpl::instance->rootProxy = result.getRoot();
          
          System* system = System::getInstance(returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            UAPI_ERROR(fn,"Could not get the System instance.\n");
            return EmbeddedRecognizerProxy();
          }
          
          // has to be called after rootProxy is assigned.
          system->add(impl::EmbeddedRecognizerImpl::instance, returnCode);
          if (returnCode != ReturnCode::SUCCESS)
          {
            delete impl::EmbeddedRecognizerImpl::instance;
            impl::EmbeddedRecognizerImpl::instance = 0;
            return EmbeddedRecognizerProxy();
          }
          return result;
        }
        else
          returnCode = ReturnCode::SUCCESS;
        EmbeddedRecognizerProxy result(impl::EmbeddedRecognizerImpl::instance->rootProxy);
        if (!result)
          returnCode = ReturnCode::OUT_OF_MEMORY;
        return result;
      }
      
      namespace impl
      {
        EmbeddedRecognizerImpl* EmbeddedRecognizerImpl::instance = 0;
        LibraryLoader* EmbeddedRecognizerImpl::srecLoader = 0;
        Mutex* EmbeddedRecognizerImpl::mutex = 0;
        EmbeddedRecognizerImpl::ComponentInitializer EmbeddedRecognizerImpl::componentInitializer;
        
        EmbeddedRecognizerImpl::ComponentInitializer::ComponentInitializer()
        {
          mutex = Mutex::create(false, returnCode);
          if (returnCode)
          {
            fprintf(stderr, "Could not create EmbeddedRecognizerImpl::mutex\n");
            return;
          }
        }
        
        EmbeddedRecognizerImpl::ComponentInitializer::~ComponentInitializer()
        {
          delete mutex;
          returnCode = ReturnCode::UNKNOWN;
        }
        
        
        EmbeddedRecognizerImpl::EmbeddedRecognizerImpl():
            rootProxy(0)
        {
          UAPI_FN_NAME("EmbeddedRecognizerImpl::EmbeddedRecognizerImpl");
          UAPI_TRACE(fn,"this=%p\n", this);
        }
        
#ifdef RECOGNIZER_DYNAMICALLY_LOADED
        
        void EmbeddedRecognizerImpl::configure(const char* config, ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("EmbeddedRecognizerImpl::configure");
            
          const char* libname = "UAPI_srec";
          
          if (strcmp(libname, "UAPI_srec") == 0)
          {
            ConfigureEmbeddedRecognizer configureFunction;
            {
              LockScope ls(mutex, returnCode);
              if (returnCode)
              {
                UAPI_ERROR(fn,"Could not lock mutex: %s\n", ReturnCode::toString(returnCode));
                return;
              }
              if (srecLoader == 0)
              {
                srecLoader = LibraryLoader::create(returnCode);
                if (returnCode != ReturnCode::SUCCESS)
                {
                  UAPI_ERROR(fn,"Failed to allocate a LibraryLoader: %s\n", ReturnCode::toString(returnCode));
                  return;
                }
                
                // try to load the library.
                srecLoader->load(libname, returnCode);
                if (returnCode != ReturnCode::SUCCESS)
                {
                  UAPI_ERROR(fn,"Unable to load %s EmbeddedRecognizer implementation: %s\n",
                             libname, ReturnCode::toString(returnCode));
                  return;
                }
              }
              // get the address of the symbol
              static const char* const symbol = "ConfigureEmbeddedRecognizer";
              configureFunction =
                (ConfigureEmbeddedRecognizer) srecLoader->symbolToAddress(symbol, returnCode);
              if (returnCode != ReturnCode::SUCCESS)
              {
                UAPI_ERROR(fn,"Could not find symbol %s of library %s\n", symbol, libname);
                return;
              }
            }
            
					  //BUG 4544
						//We must make sure the object wrapped by "delegate" gets deleted
						//before we call configureFunction again. By doing this,
						//SrecRecognizerImpl::~SrecRecognizerImpl() will get called before
						//configureFunction invokes
						//SrecRecognizerImpl::SrecRecognizerImpl() 
            delegate = EmbeddedRecognizerProxy();

            // create a new instance of an embedded recognizer.
            EmbeddedRecognizerProxy* object = configureFunction(config, returnCode);
            if (returnCode)
              return;
              
            EmbeddedRecognizerProxy result(*object);
            delete object;
            delegate = result;
            if (!result)
            {
              returnCode = ReturnCode::OUT_OF_MEMORY;
              return;
            }
            return;
          }
          else
          {
            UAPI_ERROR(fn,"Unsupported embedded recognizer of name %s\n", libname);
            returnCode = ReturnCode::NOT_SUPPORTED;
            return;
          }
        }
        
#else
        
        void EmbeddedRecognizerImpl::configure(const char* config, ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("EmbeddedRecognizerImpl::configure");

          //BUG 4544
          //We must make sure the object wrapped by "delegate" gets deleted
          //before we call configureFunction again. By doing this,
          //SrecRecognizerImpl::~SrecRecognizerImpl() will get called before
          //configureFunction invokes
          //SrecRecognizerImpl::SrecRecognizerImpl() 
          delegate = EmbeddedRecognizerProxy();
        
          // just call the method directly, instead of loading the .so and doing a symbol lookup
          EmbeddedRecognizerProxy* object = ::ConfigureEmbeddedRecognizer(config, returnCode);
          if (returnCode)
            return;
        
          EmbeddedRecognizerProxy result(*object);
          delete object;
          delegate = result;
          if (!result)
          {
            returnCode = ReturnCode::OUT_OF_MEMORY;
            return;
          }
          return;
        }
        
#endif
        
        EmbeddedRecognizerImpl::~EmbeddedRecognizerImpl()
        {
          UAPI_FN_NAME("EmbeddedRecognizerImpl::~EmbeddedRecognizerImpl");
          UAPI_TRACE(fn,"this=%p\n", this);
          
          // Must destroy the delegate before unloading the library we got it from, otherwise
          // the destructor call will crash
          delegate = EmbeddedRecognizerProxy();
          instance = 0;
        }
        
        void EmbeddedRecognizerImpl::setListener(RecognizerListenerProxy& listener, ReturnCode::Type& returnCode)
        {
          if (delegate == 0)
          {
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          delegate->setListener(listener, returnCode);
        }
        
        GrammarProxy EmbeddedRecognizerImpl::createGrammar(const char* value, GrammarListenerProxy& listener, ReturnCode::Type& returnCode)
        {
          if (delegate == 0)
          {
            returnCode = ReturnCode::INVALID_STATE;
            return GrammarProxy();
          }
          return delegate->createGrammar(value, listener, returnCode);
        }
        
        void EmbeddedRecognizerImpl::recognize(AudioStreamProxy& audio, GrammarProxy* grammars,
                                               ARRAY_LIMIT grammarCount, ReturnCode::Type& returnCode)
        {
          if (delegate == 0)
          {
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          delegate->recognize(audio, grammars, grammarCount, returnCode);
        }
        
        void EmbeddedRecognizerImpl::recognize(AudioStreamProxy& audio, GrammarProxy& grammar,
                                               ReturnCode::Type& returnCode)
        {
          if (delegate == 0)
          {
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          delegate->recognize(audio, grammar, returnCode);
        }
        
        void EmbeddedRecognizerImpl::stop(ReturnCode::Type& returnCode)
        {
          if (delegate == 0)
          {
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          delegate->stop(returnCode);
        }
        
        void EmbeddedRecognizerImpl::setParameters(const char** keys, const char** values,
            ARRAY_LIMIT count, ReturnCode::Type& returnCode)
        {
          if (delegate == 0)
          {
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          delegate->setParameters(keys, values, count, returnCode);
        }
        
        void EmbeddedRecognizerImpl::getParameters(const char** keys, ARRAY_LIMIT count,
            ReturnCode::Type& returnCode)
        {
          if (delegate == 0)
          {
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          delegate->getParameters(keys, count, returnCode);
        }
        
        void EmbeddedRecognizerImpl::resetAcousticState(ReturnCode::Type& returnCode)
        {
          if (delegate == 0)
          {
            returnCode = ReturnCode::INVALID_STATE;
            return;
          }
          delegate->resetAcousticState(returnCode);
        }
        
        SmartProxy::Root* EmbeddedRecognizerImpl::getRoot()
        {
          return rootProxy;
        }
        
        void EmbeddedRecognizerImpl::shutdown(ReturnCode::Type& returnCode)
        {
          UAPI_FN_SCOPE("EmbeddedRecognizerImpl::shutdown");
            
          returnCode = ReturnCode::SUCCESS;
        }
      }
    }
  }
}
