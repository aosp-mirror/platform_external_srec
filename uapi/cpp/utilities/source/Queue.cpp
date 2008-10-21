/*---------------------------------------------------------------------------*
 *  Queue.cpp                                                                *
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
#include "Queue.h"
#include <stdio.h>

using namespace android::speech::recognition;
using namespace android::speech::recognition::utilities;


SinglyLinkedNode::SinglyLinkedNode(void* _data):
    next(0), data(_data)
{}

bool Queue::empty() const
{
  return head->next == 0;
}

UINT32 Queue::size() const
{
  return queueSize;
}

SinglyLinkedNode* Queue::begin() const
{
  return head;
}

SinglyLinkedNode* Queue::end() const
{
  return 0;
}

FwdIterator::FwdIterator(SinglyLinkedNode* _first, SinglyLinkedNode* _last):
    currentNode(_first), lastNode(_last)
{}

SinglyLinkedNode* FwdIterator::current()
{
  return currentNode;
}


FwdIterator::~FwdIterator()
{}

bool FwdIterator::hasNext()
{
  return currentNode == 0 ? false : (currentNode->next != lastNode);
}

void* FwdIterator::next()
{
  void* result = currentNode->next->data;
  currentNode = currentNode->next;
  return result;
}

void FwdIterator::reset(SinglyLinkedNode* first, SinglyLinkedNode* last)
{
  currentNode = first;
  lastNode = last;
}

/////////////////////////////////////////////////////
Queue::Queue():
    queueSize(0)
{
  head = new SinglyLinkedNode(0);
  tail = 0;
  head->next = tail;
}

Queue::~Queue()
{
  clear();
  delete head;
}

void Queue::clear()
{
  while (!empty())
    pop();
}

void* Queue::front() const
{
  if (empty())
    return 0;
  return head->next->data;
}

void Queue::push(void* data, ReturnCode::Type& returnCode)
{
  SinglyLinkedNode* newNode = new SinglyLinkedNode(data);
  if (newNode == 0)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return;
  }
  if (empty())
  {
    head->next = newNode;
    tail = head->next;
  }
  else
  {
    tail->next = newNode;
    tail = tail->next;
  }
  ++queueSize;
  
  returnCode = ReturnCode::SUCCESS;
}

void Queue::pop()
{
  SinglyLinkedNode* pOld = head->next;
  head->next = head->next->next;
  delete pOld;
  --queueSize;
}


void Queue::remove(void* dataToRemove)
{
  SinglyLinkedNode* current = begin();
  
  while (current->next != end())
  {
    void* item = current->next->data;
    if (item == dataToRemove)
    {
      // found it
      SinglyLinkedNode* nodeToRemove = current->next;
      current->next = current->next->next;
      if (nodeToRemove == tail)
        tail = current;
      delete nodeToRemove;
      --queueSize;
      break;
    }
    current = current->next;
  }
}
