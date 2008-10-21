/*---------------------------------------------------------------------------*
 *  DoublyLinkedList.cpp                                                     *
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
#include <stdlib.h>

#include "DoublyLinkedList.h"

using namespace android::speech::recognition::utilities;


namespace android
{
  namespace speech
  {
    namespace recognition
    {
      DoublyLinkedNode::DoublyLinkedNode(void* _data):
          SinglyLinkedNode(_data),
          previous(0)
      {}
      
      DoublyLinkedList::DoublyLinkedList():
          listSize(0)
      {
        head = new DoublyLinkedNode(0);
	
        if ( head != NULL )
        {	
          tail = new DoublyLinkedNode(0);

          if ( tail != NULL )
          {
            head->next = tail;
            head->previous = tail;
        
            tail->next = head;
            tail->previous = head;
          }
          else
          {
            delete head;
            head = NULL;
          }
        }
        else
        {
          tail = NULL;
        }
      }


      DoublyLinkedList::~DoublyLinkedList()
      {
        clear();
        delete tail;
        delete head;
      }


      DoublyLinkedList::DoublyLinkedList(const DoublyLinkedList & toCopy)
      {
        head = new DoublyLinkedNode(0);
	
        if ( head != NULL )
        {	
          tail = new DoublyLinkedNode(0);
        
          if ( tail != NULL )
          {
            head->next = tail;
            head->previous = tail;
        
            tail->next = head;
            tail->previous = head;
        
            FwdIterator it(toCopy.begin(), toCopy.end());
        
            while (it.hasNext())
            {
              void* item = it.next();
              this->push_back(item);
            }
          }
          else
          {
            delete head;
            head = NULL;
          }
        }
        else
        {
          tail = NULL;
        }
      }
 

      void DoublyLinkedList::push_front(void* data)
      {
        insert((DoublyLinkedNode*)(head->next), data);
      }
      
      void DoublyLinkedList::push_back(void* data)
      {
        insert(tail, data);
      }
      
      void DoublyLinkedList::pop_front()
      {
        remove(head->next->data);
      }
      
      void* DoublyLinkedList::operator[](unsigned n) const
      {
        if (n >= size())
          return head->data; //NULL
          
        DoublyLinkedNode* pTemp = (DoublyLinkedNode*)(head->next);
        for (unsigned i = 0; i < n ; ++i)
          pTemp = (DoublyLinkedNode*)(pTemp->next);
        return pTemp->data;
      }
      
      void DoublyLinkedList::remove(void* pElement)
      {
        DoublyLinkedNode* pTemp = (DoublyLinkedNode*)(head->next);
        //search from head
        while (pTemp != tail)
        {
          if (pTemp->data == pElement)
          {
            //found it
            DoublyLinkedNode* prevNode = pTemp->previous;
            DoublyLinkedNode* nextNode = (DoublyLinkedNode*)(pTemp->next);
            
            prevNode->next = nextNode;
            nextNode->previous = prevNode;
            delete pTemp;
            --listSize;
            break;
          }
          pTemp = (DoublyLinkedNode*)(pTemp->next);
        }
      }
      
      void DoublyLinkedList::clear()
      {
        DoublyLinkedNode* pTemp = (DoublyLinkedNode*)(head->next);
        //search from head
        while (pTemp != tail)
        {
          DoublyLinkedNode* pToDelete = pTemp;
          pTemp = (DoublyLinkedNode*)(pTemp->next);
          delete pToDelete;
        }
        head->next = tail;
        head->previous = tail;
        
        tail->next = head;
        tail->previous = head;
        listSize = 0;
      }
      
      void DoublyLinkedList::insert(DoublyLinkedNode* index, void* data)
      {
        DoublyLinkedNode* newNode = new DoublyLinkedNode(data);
        DoublyLinkedNode* previous = index->previous;
        
        newNode->next = index;
        newNode->previous = previous;
        previous->next = newNode;
        index->previous = newNode;
        ++listSize;
      }
      
      bool DoublyLinkedList::empty() const
      {
        return head->next == tail;
      }
      
      UINT32 DoublyLinkedList::size() const
      {
        return listSize;
      }
      
      void* DoublyLinkedList::front()
      {
        return head->next->data;
      }
      
      void* DoublyLinkedList::back()
      {
        return tail->previous->data;
      }
      
      DoublyLinkedNode* DoublyLinkedList::begin() const
      {
        return head;
      }
      
      DoublyLinkedNode* DoublyLinkedList::end() const
      {
        return tail;
      }
    }
  }
}
