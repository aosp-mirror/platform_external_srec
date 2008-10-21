/*---------------------------------------------------------------------------*
 *  SrecGrammarImpl.h                                                        *
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

#ifndef __UAPI__SRECGRAMMARIMPL
#define __UAPI__SRECGRAMMARIMPL

#include "exports.h"
#include "SrecGrammar.h"
#include "Task.h"
#include "SlotItem.h"
#include "SrecRecognizerImpl.h"
#include "GrammarListener.h"

typedef struct SR_Grammar_t SR_Grammar;
typedef struct SR_Vocabulary_t SR_Vocabulary;
typedef struct SR_AcousticModels_t SR_AcousticModels;
typedef struct SR_Recognizer_t SR_Recognizer;

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      class GrammarListener;
      namespace utilities
      {
        class WorkerQueue;
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
      namespace srec
      {
        class SrecGrammarImplProxy;
        /**
         * SREC grammar.
         */
        class SrecGrammarImpl: public SrecGrammar
        {
          public:
            /**
             * Creates a new SREC grammar.
             *
             * @param path the path to load the grammar from
             * @param listener the grammar listener
             * @param recognizer an instance of SrecRecognizerImpl::Proxy
             * @param workerQueue the worker queue to uswell
             * @param returnCode returns SUCCESS unless a fatal error occurs
             */
            SREC_EXPORT static SrecGrammarImplProxy create(const char* path,
                android::speech::recognition::GrammarListenerProxy& listener,
                SrecRecognizerImplProxy& recognizer,
                android::speech::recognition::utilities::WorkerQueue* workerQueue,
                ReturnCode::Type& returnCode);
                
            /**
             * Adds an item to a slot.
             *
             * @param slotName the name of the slot
             * @param item the item to add to the slot.
             * @param weight the weight of the item. Smaller values are more likely to get recognized. This should be >= 0.
             * @param semanticMeaning the value that will be returned if this item is recognized. This should
             * be of the form "V='Jen_Parker'"
             * @param returnCode ILLEGAL_ARGUMENT if slotName, item or semanticMeaning are null or if
             * semanticMeaning is not in the form "key=value". ILLEGAL_STATE if the associated
             * recognizer has been deleted. HOMONYM_COLLISION if another item with the same
             * pronunciation already exists in the slot.
             */
            SREC_EXPORT virtual void addItem(const char* slotName, const SlotItemProxy& item,
                                             int weight, const char* semanticMeaning,
                                             ReturnCode::Type& returnCode);
            /**
             * Add a list of item to a slot.
             *
             * @param slotName the name of the slot
             * @param items the array of SlotItems to add to the slot.
             * @param weights the array of weights for each item in the list. Smaller values are more likely to get recognized.  This should be >= 0.
             * @param semanticMeanings the array of strings that will be returned for each item during recognition.
             * @param itemsCount number of items in the list
             * @throws IllegalArgumentException if slotName, items, weights or semanticMeanings are null;if any semanticMeaning of the list is not of the format "V=&#039;Jen_Parker&#039; if the size of list parameters is not equal."
             */
             SREC_EXPORT virtual void addItemList(const char* slotName, 
                                             SlotItemProxy** items,
                                             int* weights,
                                             const char** semanticMeanings,
                                             ARRAY_LIMIT itemsCount,
                                             ReturnCode::Type& returnCode);
            /**
             * Compiles items that were added to any of the grammar slots.
             *
             * @param returnCode ILLEGAL_ARGUMENT if the associated recognizer has been deleted
             */
            SREC_EXPORT virtual void compileAllSlots(ReturnCode::Type& returnCode);
            
            /**
             * Removes all words added to all slots.
             *
             * @param returnCode ILLEGAL_ARGUMENT if the associated recognizer has been deleted
             */
            SREC_EXPORT virtual void resetAllSlots(ReturnCode::Type& returnCode);
            
            /**
             * Indicates that the grammar will be used in the near future.
             *
             * @param returnCode ILLEGAL_ARGUMENT if the associated recognizer has been deleted
             */
            SREC_EXPORT virtual void load(ReturnCode::Type& returnCode);
            
            /**
             * Indicates that the grammar will be used in the near future.
             *
             * @param returnCode ILLEGAL_ARGUMENT if the associated recognizer has been deleted
             */
            SREC_EXPORT virtual void unload(ReturnCode::Type& returnCode);
            
            /**
             * Saves the compiled grammar.
             *
             * @param path the path to save the grammar to
             * @param returnCode ILLEGAL_ARGUMENT if the associated recognizer has been deleted
             */
            SREC_EXPORT virtual void save(const char* path, ReturnCode::Type& returnCode);
            
            
            /**
             * Check if recognizer is idle, if not all grammar actions should be avoided.
             * @return true if recognizer is idle, false otherwise
             */
            SREC_EXPORT static bool isSrecRecognizing();
            friend class SrecRecognizerImpl;
            friend class AddItemTask;
            friend class AddItemListTask;
            friend class CompileAllSlotsTask;
            friend class ResetAllSlotsTask;
            friend class SaveTask;
            friend class LoadTask;
            friend class UnloadTask;
            friend class SrecGrammarImplProxy;
          private:
            /**
             * Creates a new SREC grammar.
             *
             * @param path the path to load the grammar from
             * @param listener the grammar listener
             * @param recognizer an instance of SrecRecognizerImpl::Proxy
             * @param workerQueue the worker queue to uswell
             * @param returnCode returns SUCCESS unless a fatal error occurs
             */
            SrecGrammarImpl(const char* path, android::speech::recognition::GrammarListenerProxy& listener,
                            SrecRecognizerImplProxy& recognizer,
                            android::speech::recognition::utilities::WorkerQueue* workerQueue,
                            ReturnCode::Type& returnCode);
            /**
             * Prevent assignment.
             */
            SrecGrammarImpl& operator=(SrecGrammarImpl&);
            /**
             * Prevent destruction.
             */
            virtual ~SrecGrammarImpl();
            
            /**
             * Indicates if the grammar is loaded.
             *
             * @return true if the grammar is loaded
             */
            SREC_EXPORT virtual bool isLoaded();
            /**
             * Invoked by onAddItem and onAddItemList.
             */
            void onHelperAddItem(char* slotName, const SlotItemProxy& item, int weight, char* semanticMeaning,ReturnCode::Type& returnCode);
            /**
             * Invoked by AddItemTask.
             */
            void onAddItem(char* slotName, const SlotItemProxy& item, int weight, char* semanticMeaning);
            /**
             * Invoked by AddItemListTask.
             */
            void onAddItemList(char* slotName, SlotItemProxy* items, int* weights, char** semanticMeanings, ARRAY_LIMIT itemsCount);
            /**
             * Invoked by CompileAllSlotsTask.
             */
            void onCompileAllSlots();
            /**
             * Invoked by ResetAllSlotsTask.
             */
            void onResetAllSlots();
            /**
             * Invoked by LoadTask.
             */
            void onLoad();
            /**
             * Invoked by UnloadTask.
             */
            void onUnload();
            /**
             * Invoked by SaveTask.
             */
            void onSave(const char* path);
            
            
            /**
             * The path to load the grammar from.
             */
            char* path;
            /**
             * Indicates if the grammar has been loaded.
             */
            bool _loaded;
            /**
             * The underlying grammar.
             */
            SR_Grammar* grammar;
            /**
             * The associated recognizer.
             */
            SrecRecognizerImplProxy recognizer;
            /**
             * The grammar listener.
             */
            GrammarListenerProxy _listener;
            /**
             * The thread and the queue used to process aysnc tasks.
             */
            utilities::WorkerQueue* _workerQueue;
            SmartProxy::Root* rootProxy;
        };
        
        /*
         * @see android::speech::recognition::SmartProxy
         */
        DECLARE_SMARTPROXY(SREC_EXPORT, SrecGrammarImplProxy, SrecGrammarProxy, SrecGrammarImpl)
        
        class AddItemTask: public utilities::Task
        {
          public:
            AddItemTask(SrecGrammarImplProxy& grammar, char* slotName, const SlotItemProxy& item,
                        int weight, char* semanticMeaning);
            virtual void run();
            virtual ~AddItemTask();
            
          private:
            SrecGrammarImplProxy grammar;
            char* _slotName;
            const SlotItemProxy item;
            int _weight;
            char* _semanticMeaning;
        };
        
        class AddItemListTask: public utilities::Task
        {
          public:
            AddItemListTask(SrecGrammarImplProxy& grammar,
                        char* slotName,
                        SlotItemProxy* items,
                        int* weights,
                        char** semanticMeanings,
                        ARRAY_LIMIT itemsCount);
            virtual void run();
            virtual ~AddItemListTask();
            
          private:
            SrecGrammarImplProxy grammar;
            char* _slotName;
            SlotItemProxy* _items;
            int* _weights;
            char** _semanticMeanings;
            ARRAY_LIMIT _itemsCount;
        };
        class CompileAllSlotsTask: public utilities::Task
        {
          public:
            CompileAllSlotsTask(SrecGrammarImplProxy& grammar);
            virtual void run();
          private:
            SrecGrammarImplProxy grammar;
        };
        
        class ResetAllSlotsTask: public utilities::Task
        {
          public:
            ResetAllSlotsTask(SrecGrammarImplProxy& grammar);
            virtual void run();
          private:
            SrecGrammarImplProxy grammar;
        };
        
        class LoadTask: public utilities::Task
        {
          public:
            LoadTask(SrecGrammarImplProxy& grammar);
            virtual void run();
          private:
            SrecGrammarImplProxy grammar;
        };
        
        class UnloadTask: public utilities::Task
        {
          public:
            UnloadTask(SrecGrammarImplProxy& grammar);
            virtual void run();
          private:
            SrecGrammarImplProxy grammar;
        };
        
        class SaveTask: public utilities::Task
        {
          public:
            SaveTask(SrecGrammarImplProxy& grammar, const char* path);
            virtual ~SaveTask();
            virtual void run();
          private:
            const char* path;
            SrecGrammarImplProxy grammar;
        };
      }
    }
  }
}

#endif
