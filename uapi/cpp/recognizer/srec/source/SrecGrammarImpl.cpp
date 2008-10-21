/*---------------------------------------------------------------------------*
 *  SrecGrammarImpl.cpp                                                      *
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

#include "ReturnCode.h"
#include "SrecGrammarImpl.h"
#include "SrecRecognizerImpl.h"
#include "SrecHelper.h"
#include "SlotItem.h"
#include "WordItemImpl.h"
#include "SrecVoicetagItemImpl.h"
#include "WorkerQueueImpl.h"
#include "EmbeddedGrammarListener.h"
#include "Logger.h"
#include "SrecGrammarListener.h"

#include "plog.h"
#include "SR_Recognizer.h"
#include "SR_Nametag.h"


using namespace android::speech::recognition;
using namespace android::speech::recognition::impl;
using namespace android::speech::recognition::utilities;
using namespace android::speech::recognition::srec;

DEFINE_SMARTPROXY(srec, SrecGrammarImplProxy, SrecGrammarProxy, SrecGrammarImpl)

static   const char * FILE_URL_PREFIX = "file://";

SrecGrammarImplProxy SrecGrammarImpl::create(const char* path, GrammarListenerProxy& listener,
    SrecRecognizerImplProxy& recognizer,
    WorkerQueue* workerQueue,
    ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("SrecGrammarImpl::create");
    
  SrecGrammarImpl* object = new SrecGrammarImpl(path, listener, recognizer, workerQueue, returnCode);
  if (!object)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return SrecGrammarImplProxy();
  }
  else if (returnCode)
  {
    delete object;
    return SrecGrammarImplProxy();
  }
  SrecGrammarImplProxy result(object);
  if (!result)
    returnCode = ReturnCode::OUT_OF_MEMORY;
  object->rootProxy = result.getRoot();
  return result;
}

SrecGrammarImpl::SrecGrammarImpl(const char* _path, GrammarListenerProxy& listener,
                                 SrecRecognizerImplProxy& _recognizer,
                                 WorkerQueue* workerQueue,
                                 ReturnCode::Type& returnCode):
    path(0),
    _loaded(false),
    grammar(0),
    recognizer(_recognizer),
    _listener(listener),
    _workerQueue(workerQueue),
    rootProxy(0)
{
  UAPI_FN_NAME("SrecGrammarImpl::SrecGrammarImpl");
    
  UAPI_TRACE(fn,"this=%p\n", this);

  const char * startPath = _path;
  if( strstr(_path, FILE_URL_PREFIX) != 0)
  {
    startPath += strlen(FILE_URL_PREFIX);
  }

  path = new char[strlen(startPath)+1];
  if (path == 0)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return;
  }
  
  strcpy(path, startPath);
  returnCode = ReturnCode::SUCCESS;
}

SrecGrammarImpl::~SrecGrammarImpl()
{
  UAPI_FN_NAME("SrecGrammarImpl::~SrecGrammarImpl");
    
  UAPI_TRACE(fn, "this=%p\n", this);
  
  if (grammar != 0)
    SR_GrammarDestroy(grammar);
  delete[] path;
}

bool SrecGrammarImpl::isSrecRecognizing()
{
    ReturnCode::Type returnCode;
    srec::SrecRecognizerImplProxy nativeRecognizer = (srec::SrecRecognizerImplProxy) EmbeddedRecognizer::getInstance(returnCode);
    if (returnCode != ReturnCode::SUCCESS)
        return false;
    return nativeRecognizer->isRecognizing();
}

void SrecGrammarImpl::addItem(const char* slotName, const SlotItemProxy& item, int weight,
                              const char* semanticMeaning,
                              ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("SrecGrammarImpl::addItem");
    
  if (slotName == 0 || item == 0 || semanticMeaning == 0)
  {
    UAPI_WARN(fn,"slotName, item or semanticMeaning is null\n");
    returnCode = ReturnCode::ILLEGAL_ARGUMENT;
    return;
  }
  else if (strchr(semanticMeaning, '=') == 0)
  {
    UAPI_WARN(fn,"semanticMeaning must contain a = sign\n");
    // Semantic meaning must be of the form V='Jen_Parker'
    returnCode = ReturnCode::ILLEGAL_ARGUMENT;
    return;
  }
  
  // Declare variables in case of goto CLEANUP
  char* slotNameCopy = 0;
  char* semanticMeaningCopy = 0;
  AddItemTask* task = 0;
  
  slotNameCopy = new char[strlen(slotName)+1];
  if (slotNameCopy == 0)
  {
    UAPI_WARN(fn,"Could not allocate memory for slotNameCopy\n");
    returnCode = ReturnCode::OUT_OF_MEMORY;
    goto CLEANUP;
  }
  strcpy(slotNameCopy, slotName);
  
  semanticMeaningCopy = new char[strlen(semanticMeaning)+1];
  if (semanticMeaningCopy == 0)
  {
    UAPI_WARN(fn,"Could not allocate memory for semanticMeaningCopy\n");
    returnCode = ReturnCode::OUT_OF_MEMORY;
    goto CLEANUP;
  }
  strcpy(semanticMeaningCopy, semanticMeaning);
  
  {
    SrecGrammarImplProxy proxy(rootProxy);
    if (!proxy)
    {
      UAPI_ERROR(fn,"Could not create proxy\n");
      returnCode = ReturnCode::INVALID_STATE;
      goto CLEANUP;
    }
    task = new AddItemTask(proxy, slotNameCopy, item, weight, semanticMeaningCopy);
    if (!task)
    {
      UAPI_ERROR(fn,"Could not create AddItemTask\n");
      returnCode = ReturnCode::OUT_OF_MEMORY;
      goto CLEANUP;
    }
    _workerQueue->enqueue(task, returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"Could not enqueue AddItemTask\n");
      delete task;
      goto CLEANUP;
    }
    
  }
  return;
CLEANUP:
  delete[] slotNameCopy;
  delete[] semanticMeaningCopy;
  return;
}
void SrecGrammarImpl::addItemList(const char* slotName,
                              SlotItemProxy** items,
                              int* weights,
                              const char** semanticMeanings,
                              ARRAY_LIMIT itemCount,
                              ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("SrecGrammarImpl::addItemList");

  if (slotName == 0 || items == 0 || weights == 0 ||semanticMeanings == 0)
  {
    UAPI_WARN(fn,"slotName, item, weights or semanticMeaning is null\n");
    returnCode = ReturnCode::ILLEGAL_ARGUMENT;
    return;
  }
  // Semantic meaning must be of the form V='Jen_Parker'
  ARRAY_LIMIT i=0;
  for(;i<itemCount;i++)
  {
      if (strchr(semanticMeanings[i], '=') == 0)
      {
         UAPI_WARN(fn,"semanticMeaning[%d] must contain a = sign\n", i);
         returnCode = ReturnCode::ILLEGAL_ARGUMENT;
         return;
      }
  }
  
  // Declare variables in case of goto CLEANUP
  char* slotNameCopy = 0;
  SlotItemProxy * itemsCopy = 0;
  char** semanticMeaningsCopy = 0;
  int* weightsCopy = 0;
  AddItemListTask* task = 0;
  
  // Copy arguments before passing into task
  slotNameCopy = new char[strlen(slotName)+1];
  if (slotNameCopy == 0)
  {
    UAPI_WARN(fn,"Could not allocate memory for slotNameCopy\n");
    returnCode = ReturnCode::OUT_OF_MEMORY;
    goto CLEANUP;
  }
  strcpy(slotNameCopy, slotName);

  itemsCopy = new SlotItemProxy[itemCount];
  semanticMeaningsCopy = new char*[itemCount];
  weightsCopy = new int[itemCount];

  for (i=0;i<itemCount;++i)
  {
    // copy slots
    itemsCopy[i] = *items[i];

    // copy semantic meanings
    semanticMeaningsCopy[i] = new char[strlen(semanticMeanings[i]) + 1];
    if (semanticMeaningsCopy[i] == 0)
    {
        UAPI_WARN(fn,"Could not allocate memory for slotNameCopy\n");
        returnCode = ReturnCode::OUT_OF_MEMORY;
        goto CLEANUP;
    }
    strcpy(semanticMeaningsCopy[i], semanticMeanings[i]);
    // copy weights
    weightsCopy[i] = weights[i];
  }
  
  {
    SrecGrammarImplProxy proxy(rootProxy);
    if (!proxy)
    {
      UAPI_ERROR(fn,"Could not create proxy\n");
      returnCode = ReturnCode::INVALID_STATE;
      goto CLEANUP;
    }
    task = new AddItemListTask(proxy, slotNameCopy, itemsCopy, weightsCopy, semanticMeaningsCopy,itemCount);
    if (!task)
    {
      UAPI_ERROR(fn,"Could not create AddItemListTask\n");
      returnCode = ReturnCode::OUT_OF_MEMORY;
      goto CLEANUP;
    }
    _workerQueue->enqueue(task, returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"Could not enqueue AddItemListTask\n");
      delete task;
      goto CLEANUP;
    }
    
  }
  return;
CLEANUP:
  delete[] slotNameCopy;
  for (i =0;i<itemCount;++i)
    delete[] semanticMeaningsCopy[i];
  delete[] itemsCopy;
  delete[] semanticMeaningsCopy;
  delete[] weightsCopy;
  return;
}

void SrecGrammarImpl::compileAllSlots(ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("SrecGrammarImpl::compileAllSlots");
    
  SrecGrammarImplProxy proxy(rootProxy);
  if (!proxy)
  {
    UAPI_ERROR(fn,"Could not create proxy\n");
    returnCode = ReturnCode::INVALID_STATE;
    return;
  }
  CompileAllSlotsTask* task = new CompileAllSlotsTask(proxy);
  if (!task)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    UAPI_ERROR(fn,"Could not create CompileAllSlotsTask\n");
    return;
  }
  _workerQueue->enqueue(task, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Could not enqueue CompileAllSlotsTask\n");
    delete task;
    return;
  }
}

void SrecGrammarImpl::resetAllSlots(ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("SrecGrammarImpl::resetAllSlots");
    
  SrecGrammarImplProxy proxy(rootProxy);
  if (!proxy)
  {
    UAPI_ERROR(fn,"Could not create proxy\n");
    returnCode = ReturnCode::INVALID_STATE;
    return;
  }
  ResetAllSlotsTask* task = new ResetAllSlotsTask(proxy);
  if (task == 0)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    UAPI_ERROR(fn,"Could not create ResetAllSlotsTask\n");
    return;
  }
  _workerQueue->enqueue(task, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Could not enqueue ResetAllSlotsTask\n");
    delete task;
    return;
  }
}

void SrecGrammarImpl::load(ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("SrecGrammarImpl::load");
    
  SrecGrammarImplProxy proxy(rootProxy);
  if (!proxy)
  {
    UAPI_ERROR(fn,"Could not create proxy\n");
    return;
  }
  LoadTask* task = new LoadTask(proxy);
  if (task == 0)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    UAPI_ERROR(fn,"Could not create LoadTask\n");
    return;
  }
  _workerQueue->enqueue(task, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Could not enqueue LoadTask\n");
    delete task;
    return;
  }
}

void SrecGrammarImpl::unload(ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("SrecGrammarImpl::unload");
    
  SrecGrammarImplProxy proxy(rootProxy);
  if (!proxy)
  {
    UAPI_ERROR(fn,"Could not create proxy\n");
    returnCode = ReturnCode::INVALID_STATE;
    return;
  }
  UnloadTask* task = new UnloadTask(proxy);
  if (task == 0)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    UAPI_ERROR(fn,"Could not create UnloadTask\n");
    return;
  }
  _workerQueue->enqueue(task, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Could not enqueue UnloadTask\n");
    delete task;
    return;
  }
}

void SrecGrammarImpl::save(const char* path, ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("SrecGrammarImpl::save");
    
  char* pathCopy = new char[strlen(path)+1];
  if (pathCopy == 0)
  {
    UAPI_ERROR(fn,"Failed to allocate memory for pathCopy\n");
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return;
  }
  strcpy(pathCopy, path);
  
  SrecGrammarImplProxy proxy(rootProxy);
  if (!proxy)
  {
    UAPI_ERROR(fn,"Could not create proxy\n");
    returnCode = ReturnCode::INVALID_STATE;
    delete[] pathCopy;
    return;
  }
  SaveTask* task = new SaveTask(proxy, pathCopy);
  if (task == 0)
  {
    UAPI_ERROR(fn,"Could not create SaveTask\n");
    returnCode = ReturnCode::OUT_OF_MEMORY;
    delete[] pathCopy;
    return;
  }
  _workerQueue->enqueue(task, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Could not enqueue UnloadTask\n");
    delete task; //will delete pathCopy
    return;
  }
}

bool SrecGrammarImpl::isLoaded()
{
  return _loaded;
}

void SrecGrammarImpl::onHelperAddItem(char* slotName, const SlotItemProxy& item, 
                                      int weight, char* semanticMeaning,
                                      ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("SrecGrammarImpl::onHelperAddItem");
  ESR_ReturnCode rc;  
  if (item->isWord())
  {
    WordItemImplProxy& word = (WordItemImplProxy&) item;
    const char* pronunciation = word->getPronunciation();
   
    rc = SR_GrammarAddWordToSlot(grammar, slotName, word->getWord(),
                                 pronunciation, weight, semanticMeaning);
    if (rc == ESR_NOT_SUPPORTED)
    {
      UAPI_WARN(fn,"SR_GrammarAddWordToSlot returned ESR_NOT_SUPPORTED\n");
      returnCode = ReturnCode::HOMONYM_COLLISION;
      return;
    }
    else if (rc == ESR_OUT_OF_MEMORY)
    {
      UAPI_WARN(fn,"SR_GrammarAddWordToSlot returned ESR_OUT_OF_MEMORY\n");
      returnCode = ReturnCode::GRAMMAR_SLOT_FULL;
      return;
    }
    else if (rc != ESR_SUCCESS)
    {
      UAPI_WARN(fn,"SR_GrammarAddWordToSlot returned %d\n", rc);
      returnCode = SrecHelper::toUAPI(rc);
      return;
    }
  }
  else
  {
    SrecVoicetagItemImplProxy& voicetag = (SrecVoicetagItemImplProxy&) item;
    SR_Nametag* nametag = voicetag->getNametag();
    rc = SR_GrammarAddNametagToSlot(grammar, slotName, nametag,
                                    weight, semanticMeaning);
    // Don't destroy nametag here
    // the Voicetag implementation will take care of it.
    // SR_NametagDestroy(nametag);
    nametag = NULL;

    if (rc == ESR_NOT_SUPPORTED)
    {
      UAPI_WARN(fn,"SR_GrammarAddNametagToSlot returned ESR_NOT_SUPPORTED\n");
      returnCode = ReturnCode::HOMONYM_COLLISION;
      return;
    }
    else if (rc != ESR_SUCCESS)
    {
      UAPI_WARN(fn,"SR_GrammarAddNametagToSlot returned %d\n", rc);
      returnCode = SrecHelper::toUAPI(rc);
      return;
    }
  }
  returnCode = ReturnCode::SUCCESS;
  return;
}
void SrecGrammarImpl::onAddItem(char* slotName, const SlotItemProxy& item, int weight,
                                char* semanticMeaning)
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("SrecGrammarImpl::onAddItem");
    
  if (recognizer->isRecognizing())
  {
    UAPI_WARN(fn, "Recognizer is recognizing\n");
    // Recognizer is working in a recognition; no grammar operations allowed
    returnCode = ReturnCode::INVALID_STATE;
    goto CLEANUP;
  }
  ESR_BOOL wasActive;
  recognizer->RecognizerIsActiveRule(grammar, &wasActive, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to call RecognizerIsActiveRule\n");
    goto CLEANUP;
  }
  
  if (!wasActive)
  {
    SrecGrammarImplProxy proxy(rootProxy);
    if (!proxy)
    {
      UAPI_ERROR(fn,"Could not create proxy\n");
      goto CLEANUP;
    }
    recognizer->bind(proxy, returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_WARN(fn, "Recognizer bind failed %d\n", returnCode);
      goto CLEANUP;
    }
  }

  onHelperAddItem(slotName,item, weight, semanticMeaning,returnCode);

  if (returnCode != ReturnCode::SUCCESS)
  {
      UAPI_WARN(fn, "onHelperAddItem failed %d\n", returnCode);
      goto CLEANUP;
  }

  if (!wasActive)
  {
    SrecGrammarImplProxy proxy(rootProxy);
    if (!proxy)
    {
      UAPI_ERROR(fn,"Could not create proxy\n");
      goto CLEANUP;
    }
    recognizer->unbind(proxy, returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_WARN(fn, "Recognizer unbind failed %d\n", returnCode);
      goto CLEANUP;
    }
  }
  else
    returnCode = ReturnCode::SUCCESS;
  return;
CLEANUP:
  if (_listener)
    _listener->onError(returnCode);
}

void SrecGrammarImpl::onAddItemList(char* slotName, SlotItemProxy* items, int* weights, char** semanticMeanings, ARRAY_LIMIT itemsCount)
{
  ReturnCode::Type returnCode;
  ARRAY_LIMIT index=0;
  UAPI_FN_SCOPE("SrecGrammarImpl::onAddItemList");
  
  ESR_BOOL wasActive;
  
  if (recognizer->isRecognizing())
  {
    UAPI_WARN(fn, "Recognizer is recognizing\n");
    // Recognizer is working in a recognition; no grammar operations allowed
    returnCode = ReturnCode::INVALID_STATE;
    goto CLEANUP;
  }
  recognizer->RecognizerIsActiveRule(grammar, &wasActive, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to call RecognizerIsActiveRule\n");
    goto CLEANUP;
  }
  
  if (!wasActive)
  {
    SrecGrammarImplProxy proxy(rootProxy);
    if (!proxy)
    {
      UAPI_ERROR(fn,"Could not create proxy\n");
      goto CLEANUP;
    }
    recognizer->bind(proxy, returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_WARN(fn, "Recognizer bind failed %d\n", returnCode);
      goto CLEANUP;
    }
  }

  for(index=0;index<itemsCount;index++)
  {
      onHelperAddItem(slotName,items[index], weights[index], semanticMeanings[index],returnCode);
      if(0) ;
      // skip this check, so as to (1) notify the caller through the callback (2) continue looping
      //if (returnCode == ReturnCode::HOMONYM_COLLISION)
      //  goto CLEANUP;
      else if (returnCode == ReturnCode::GRAMMAR_SLOT_FULL)
        goto CLEANUP;
      else if (returnCode != ReturnCode::SUCCESS)
      {
         if (_listener)
         {
            if (_listener->isSrecGrammarListener())
			        ((SrecGrammarListenerProxy&) _listener)->onAddItemListFailure(index,returnCode);
			      else if(_listener->isEmbeddedGrammarListener())
			        ((EmbeddedGrammarListenerProxy&) _listener)->onError(returnCode);
			      else goto CLEANUP;
		      }
          else goto CLEANUP;
      }
  }

  if (!wasActive)
  {
    SrecGrammarImplProxy proxy(rootProxy);
    if (!proxy)
    {
      UAPI_ERROR(fn,"Could not create proxy\n");
      goto CLEANUP;
    }
    recognizer->unbind(proxy, returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_WARN(fn, "Recognizer unbind failed %d\n", returnCode);
      goto CLEANUP;
    }
  }
   
  returnCode = ReturnCode::SUCCESS;
  if (_listener)
  {
     if (_listener->isSrecGrammarListener())
        ((SrecGrammarListenerProxy&) _listener)->onAddItemList();
  }
  return;
CLEANUP:
  if (_listener)
  {
    _listener->onError(returnCode);
    if (_listener->isSrecGrammarListener())
                ((SrecGrammarListenerProxy&) _listener)->onAddItemListFailure(index, returnCode);
    else if(_listener->isEmbeddedGrammarListener())
			    ((EmbeddedGrammarListenerProxy&) _listener)->onError(returnCode);
  }
}
void SrecGrammarImpl::onCompileAllSlots()
{
  UAPI_FN_SCOPE("SrecGrammarImpl::onCompileAllSlots");
    
  ESR_ReturnCode rc;
  if (recognizer->isRecognizing()) // Recognizer is working in a recognition; no grammar operations allowed
  {
      UAPI_WARN(fn, "Recognizer is recognizing\n");
      rc = ESR_INVALID_STATE;
  }
  else
  {
    rc = SR_GrammarCompile(grammar);
    if (rc != ESR_SUCCESS)
    {
      UAPI_WARN(fn,"SR_GrammarCompile returned %d\n", rc);
    }
  }
  if (_listener)
  {
    if (rc == ESR_FATAL_ERROR || rc == ESR_INVALID_STATE)
      _listener->onError(ReturnCode::INVALID_STATE);
    else if (_listener->isEmbeddedGrammarListener())
      ((EmbeddedGrammarListenerProxy&) _listener)->onCompileAllSlots();
  }
}

void SrecGrammarImpl::onResetAllSlots()
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("SrecGrammarImpl::onResetAllSlots");
    
  ESR_ReturnCode rc;
  
  if (recognizer->isRecognizing()) // Recognizer is working in a recognition; no grammar operations allowed
  {
      UAPI_WARN(fn, "Recognizer is recognizing\n");
      rc = ESR_INVALID_STATE;
      goto CLEANUP;
  }
  CHKLOG(rc, SR_GrammarResetAllSlots(grammar));
  if (_listener && _listener->isEmbeddedGrammarListener())
    ((EmbeddedGrammarListenerProxy&) _listener)->onResetAllSlots();
  return;
CLEANUP:
  UAPI_WARN(fn,"SR_GrammarResetAllSlots failed %d\n", rc);
  returnCode = SrecHelper::toUAPI(rc);
  if (_listener != 0)
    _listener->onError(returnCode);
}

void SrecGrammarImpl::onLoad()
{
  UAPI_FN_SCOPE("SrecGrammarImpl::onLoad");
    
  ESR_ReturnCode rc;
  
  if (recognizer->isRecognizing()) // Recognizer is working in a recognition; no grammar operations allowed
  {
      UAPI_WARN(fn, "Recognizer is recognizing\n");
      rc = ESR_INVALID_STATE;
  }
  else
  {
    rc = SR_GrammarLoad(path, &grammar);
    if (rc != ESR_SUCCESS)
    {
      UAPI_WARN(fn,"SR_GrammarLoad returned %d\n", rc);
    }
    _loaded = true;
  }
  if (_listener)
  {
    if (rc == ESR_READ_ERROR)
      _listener->onError(ReturnCode::READ_ERROR);
    else if (rc == ESR_INVALID_STATE)
        _listener->onError(ReturnCode::INVALID_STATE);
    else
      _listener->onLoaded();
  }
}

void SrecGrammarImpl::onUnload()
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("SrecGrammarImpl::onUnload");
    
  ESR_ReturnCode rc;
  
  if (recognizer->isRecognizing()) // Recognizer is working in a recognition; no grammar operations allowed
  {
      UAPI_WARN(fn, "Recognizer is recognizing\n");
      rc = ESR_INVALID_STATE;
      goto CLEANUP;
  }
  CHKLOG(rc, SR_GrammarDestroy(grammar));
  grammar = 0;
  _loaded = false;
  if (_listener)
    _listener->onUnloaded();
  return;
CLEANUP:
  UAPI_WARN(fn,"SR_GrammarDestroy failed %d\n", rc);
  returnCode = SrecHelper::toUAPI(rc);
  if (_listener != 0)
    _listener->onError(returnCode);
}

void SrecGrammarImpl::onSave(const char* path)
{
  UAPI_FN_SCOPE("SrecGrammarImpl::onSave");
    
  ESR_ReturnCode rc;
  if (recognizer->isRecognizing()) // Recognizer is working in a recognition; no grammar operations allowed
  {
    UAPI_WARN(fn, "Recognizer is recognizing\n");
    rc = ESR_INVALID_STATE;
  }
  else
  {
    rc = SR_GrammarSave(grammar, path);
    if (rc != ESR_SUCCESS)
    {
      UAPI_WARN(fn,"SR_GrammarSave returned %d\n", rc);
    }
  }

  if (_listener)
  {
    if (rc == ESR_WRITE_ERROR)
      _listener->onError(ReturnCode::WRITE_ERROR);
    else if (rc == ESR_INVALID_STATE)
      _listener->onError(ReturnCode::INVALID_STATE);
    else if (_listener->isEmbeddedGrammarListener())
      ((EmbeddedGrammarListenerProxy&) _listener)->onSaved(path);
  }
}

AddItemTask::AddItemTask(SrecGrammarImplProxy& _grammar, char* slotName,
                         const SlotItemProxy& _item, int weight, char* semanticMeaning):
    Task("AddItemTask"),
    grammar(_grammar),
    _slotName(slotName),
    item(_item),
    _weight(weight),
    _semanticMeaning(semanticMeaning)
{}

void AddItemTask::run()
{
  grammar->onAddItem(_slotName, item, _weight, _semanticMeaning);
}

AddItemTask::~AddItemTask()
{
  delete[] _slotName;
  delete[] _semanticMeaning;
}

AddItemListTask::AddItemListTask(SrecGrammarImplProxy& _grammar, char* slotName,
                         SlotItemProxy* items, int* weights, char** semanticMeanings, ARRAY_LIMIT itemsCount):
    Task("AddItemListTask"),
    grammar(_grammar),
    _slotName(slotName),
    _items(items),
    _weights(weights),
    _semanticMeanings(semanticMeanings),
    _itemsCount(itemsCount)
{}

void AddItemListTask::run()
{
  grammar->onAddItemList(_slotName, _items, _weights, _semanticMeanings,_itemsCount);
}

AddItemListTask::~AddItemListTask()
{
  delete[] _slotName;
  for (ARRAY_LIMIT i=0;i<_itemsCount;++i)
    delete[] _semanticMeanings[i];
  delete[] _semanticMeanings;
  delete[] _items;
  delete[] _weights;
}

CompileAllSlotsTask::CompileAllSlotsTask(SrecGrammarImplProxy& _grammar):
    Task("CompileAllSlotsTask"),
    grammar(_grammar)
{}

void CompileAllSlotsTask::run()
{
  grammar->onCompileAllSlots();
}


ResetAllSlotsTask::ResetAllSlotsTask(SrecGrammarImplProxy& _grammar):
    Task("ResetAllSlotsTask"),
    grammar(_grammar)
{}

void ResetAllSlotsTask::run()
{
  grammar->onResetAllSlots();
}

UnloadTask::UnloadTask(SrecGrammarImplProxy& _grammar):
    Task("UnloadTask"),
    grammar(_grammar)
{}

void UnloadTask::run()
{
  grammar->onUnload();
}

SaveTask::~SaveTask()
{
  delete[] path;
}

SaveTask::SaveTask(SrecGrammarImplProxy& _grammar, const char* _path):
    Task("SaveTask"),
    path(_path),
    grammar(_grammar)
{}

void SaveTask::run()
{
  grammar->onSave(path);
}

LoadTask::LoadTask(SrecGrammarImplProxy& _grammar):
    Task("LoadTask"),
    grammar(_grammar)
{}

void LoadTask::run()
{
  grammar->onLoad();
}
