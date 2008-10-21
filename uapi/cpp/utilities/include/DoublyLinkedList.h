/*---------------------------------------------------------------------------*
 *  DoublyLinkedList.h                                                       *
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

#ifndef UAPI_DLINKED_LIST_H
#define UAPI_DLINKED_LIST_H

#include "exports.h"
#include "Queue.h"
#include "types.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class DoublyLinkedList;
        class UAPIFwdIterator;
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
      namespace utilities
      {
        /**
         * Double-linked node.
         */
        class DoublyLinkedNode: public SinglyLinkedNode
        {
          public:
            DoublyLinkedNode(void* data);
            
            DoublyLinkedNode* previous;
            
            friend class DoublyLinkedList;
        };
        
        
        /**
         * A double linked list that does not use template (portable). It only supports
         * pointers.
         */
        class UAPI_EXPORT DoublyLinkedList
        {
          public:
            /**
             * constructor
             */
            DoublyLinkedList();
            
            /**
             * destructor
             */
            ~DoublyLinkedList();
            
            /**
             * copy constructor
             *
             * @param toCopy object to copy.
             */
            DoublyLinkedList(const DoublyLinkedList& toCopy);
            
            /**
             * tells if the list contains items or not
             * @return true if the list is empty, false otherwise.
             */
            bool empty() const;
            
            /**
             * return the number of items contained in the list.
             * @return the number of items in the list.
             */
            UINT32 size() const;
            
            /**
             * Returns a pointer to the first element of the list.
             *
             * @return a pointer to the first element in the list
             */
            void* front();
            
            /**
             * Returns a pointer to last element of the list.
             *
             * @return a pointer to the last element in the list
             */
            void* back();
            
            /**
             * Add an element to the start of the list.
             * @param data element to add to the list (must be a pointer)
             */
            void push_front(void* data);
            
            /**
             * Add an element to the end of the list.
             * @param data element to add to the list (must be a pointer)
             */
            void push_back(void* data);
            
            /**
             * remove the first element in the list
             */
            void pop_front();
            
            /**
             * Access any element in the list using the array operator. Ex: m_list[0].
             * @param index the index of the element to return.
             */
            void* operator[](UINT32 index) const;
            
            /**
             * remove an element in the list.
             * @param pElement element to remove.
             */
            void remove(void* element);
            
            /**
             * remove all the elements in the list.
             */
            void clear();
            
            /**
             * Insert an new node in the list.
             *
             * @param index the index at which to insert the node
             * @param data the node data
             */
            void insert(DoublyLinkedNode* index, void* data);
            
            /**
             * Returns the first node in the list.
             *
             * @return the first node in the list
             */
            DoublyLinkedNode* begin() const;
            
            /**
             * Returns the last node in the last
             *
             * @return the last node in the list
             */
            DoublyLinkedNode* end() const;
            
          private:
            /**
             * head of the list
             */
            DoublyLinkedNode* head;
            /**
             * tail of the list
             */
            DoublyLinkedNode* tail;
            /**
             * number of elements in the list
             */
            UINT32 listSize;
        };
      }
    }
  }
}

#endif
