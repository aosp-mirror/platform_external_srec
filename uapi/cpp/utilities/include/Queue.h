/*---------------------------------------------------------------------------*
 *  Queue.h                                                                  *
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

#ifndef __UAPI_QUEUE_
#define __UAPI_QUEUE_

#include "exports.h"
#include "types.h"
#include "ReturnCode.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class Queue;
        class DoublyLinkedList;
        class FwdIterator;
      }
      namespace nmsp
      {
        class SocketListeningThread;
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
         * Singly linked node.
         */
        class SinglyLinkedNode
        {
          public:
            UAPI_EXPORT SinglyLinkedNode(void* data);
            
            SinglyLinkedNode* next;
          private:
            void* data;
            
            friend class DoublyLinkedList;
            friend class FwdIterator;
            friend class Queue;
            friend class android::speech::recognition::nmsp::SocketListeningThread;
        };
        
        /**
         * A forward iterator that can be used on UAPIDLinkedList and Queue.
         * In can be used like this:
         *<code>
         *    Queue m_queue;
         *
         *    FwdIterator it( m_queue.begin(), m_queue.end());
         *    while( it.hasNext() )
         *    {
         *      MyClass * p = (MyClass*) it.next();
         *      p->fct();
         *    }
         *</code>
         */
        class FwdIterator
        {
          public:
            /**
             * Creates a new FwdIterator.
             *
             * @param first The first node
             * @param last The last node
             */
            UAPI_EXPORT FwdIterator(SinglyLinkedNode* first, SinglyLinkedNode* last);
            
            /**
             * destructor.
             */
            UAPI_EXPORT ~FwdIterator();
            
            /**
             * checks if there is a next element in the iterator.
             *
             * @return true if there is another element in the iterator.
             */
            UAPI_EXPORT bool hasNext();
            
            /**
             * returns the next element in the iterator.
             *
             * @return the next element.
             */
            UAPI_EXPORT void* next();
            
            /**
             * Reset the positions of the iterator.
             * @param first where to start from.
             * @param last where we need to stop iterating.
             */
            UAPI_EXPORT void reset(SinglyLinkedNode* first, SinglyLinkedNode* last);
            
            /**
             * Returns the current iterator.
             *
             * @return a pointer to the current SinglyLinkedNode.
             */
            UAPI_EXPORT SinglyLinkedNode* current();
          private:
            /**
             * the current node that the iterator is pointing to.
             */
            SinglyLinkedNode* currentNode;
            /**
             * where to stop iterating.
             */
            SinglyLinkedNode* lastNode;
        };
        
        /**
         * A queue that does not use template (portable). It only supports
         * pointers.
         */
        class Queue
        {
          public:
            /**
             * default constructor
             */
            UAPI_EXPORT Queue();
            
            /**
             * destructor
             */
            UAPI_EXPORT ~Queue();
            
            /**
             * remove all the elements in the queue.
             */
            UAPI_EXPORT void clear();
            
            /**
             * Indicates if the queue contains items or not
             *
             * @return true if the queue is empty, false otherwise.
             */
            UAPI_EXPORT bool empty() const;
            
            /**
             * Return the number of items contained in the queue.
             *
             * @return the number of items in the queue.
             */
            UAPI_EXPORT UINT32 size() const;
            
            /**
             * Get the first element in the queue.
             *
             * @return a pointer to the first element in the queue.
             */
            UAPI_EXPORT void* front() const;
            
            /**
             * Add an element to the end of the queue.
             *
             * @param newData to add to the queue (must be a pointer).
             * @param returnCode SUCCESS unless a fatal error occurs
             */
            UAPI_EXPORT void push(void* newData, ReturnCode::Type& returnCode);
            
            
            /**
             * remove the first element from the queue.
             */
            UAPI_EXPORT void pop();
            
            /**
             * Removes an element from the queue
             *
             * @param data the element to remove
             */
            UAPI_EXPORT void remove(void* data);
            
            /**
             * Returns the first node in the queue.
             *
             * @return the first node in the queue
             */
            UAPI_EXPORT SinglyLinkedNode* begin() const;
            
            /**
             * Returns the last node in the queue.
             *
             * @return the last node in the queue
             */
            UAPI_EXPORT SinglyLinkedNode* end() const;
            
          private:
            /**
             * head of the queue.
             */
            SinglyLinkedNode* head;
            /**
             * tail of the queue.
             */
            SinglyLinkedNode* tail;
            /**
             * number of elements in the queue.
             */
            UINT32 queueSize;
        };
      }
    }
  }
}

#endif /* __UAPI_QUEUE_ */
