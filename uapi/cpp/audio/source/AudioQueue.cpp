/*---------------------------------------------------------------------------*
 *  AudioQueue.cpp                                                           *
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
#include "AudioQueue.h"
#include "AudioBuffer.h"
#include "CodecHelper.h"
#include "LoggerImpl.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

using namespace android::speech::recognition;
using namespace android::speech::recognition::utilities;


AudioQueue* AudioQueue::create(Codec::Type codec, ReturnCode::Type& returnCode)
{
  AudioQueue* result = new AudioQueue(codec, returnCode);
  if (result == 0)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return 0;
  }
  if (returnCode != ReturnCode::SUCCESS)
  {
    delete result;
    return 0;
  }
  
  returnCode = ReturnCode::SUCCESS;
  return result;
}

AudioQueue::AudioQueue(Codec::Type _codec, ReturnCode::Type& returnCode):
    RefCounted(returnCode),
    currentBuffer(0),
    codec(_codec),
    readyForGarbageCollection(0)
{
  UAPI_FN_SCOPE("AudioQueue::AudioQueue");
    
  if (returnCode)
  {
    UAPI_ERROR(fn,"Failed to construct RefCount for AudioQueue\n");
    return;
  }
  
  mutex = Mutex::create(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Could not create mutex\n");
    return;
  }
}

AudioQueue::~AudioQueue()
{
  ReturnCode::Type returnCode;
  UAPI_FN_SCOPE("AudioQueue::~AudioQueue");
    
  {
    LockScope ls(mutex, returnCode);
    if (returnCode)
      UAPI_WARN(fn,"Failed to create LockScope\n");
      
    if (currentBuffer != 0)
    {
      currentBuffer->removeRef(returnCode);
      if (returnCode != ReturnCode::SUCCESS)
      {
        UAPI_WARN(fn,"Failed to release ref count on AudioBuffer\n");
      }
    }
      
    // this will cleanup the queue of audio buffers.
    clear();
  }
  delete mutex;
}

void AudioQueue::addBuffer(unsigned char* samples, ARRAY_LIMIT length,
                           bool isLastBuffer, ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("AudioQueue::addBuffer");
    
  if (isLastBuffer && samples == 0 && length == 0)
  {
    LockScope ls(mutex, returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"Failed to create LockScope\n");
      return;
    }
    // nothing to do, nobody is waiting for audio. The AudioQueue is shared by
    // the AudioStreamImpl objects. It is created by the Microphone or the
    // MediaFileReader. To avoid the object being deleted while used by other
    // modules, we use Ref Counting (AudioQueue : public RefCounted). If the
    // RefCount is 1, it means that no AudioStreamImpl are using this audio yet. In
    // those cases, we do not have to save the audio. Simply discard it.
    if (RefCounted::getCount() <= 1)
    {
      returnCode = ReturnCode::SUCCESS;
      return;
    }
    
    //special case.
    //On some platform, it's not possible to know when the last buffer is
    //recorded. We only know after the fact that record is now stopped and at
    //this point we have no more data to submit. The code that uses the
    //AudioQueue::read() expect to receive a buffer that contains isLastBuffer
    //flag set to true. To solve this issue, we enqueue an empty buffer with
    //the flag set to true.
    AudioBuffer* lastBuffer;
    if (currentBuffer != 0)
    {
      //currentBuffer was not pushed into the queue. Set its flag to true.
      lastBuffer = currentBuffer;
      currentBuffer = 0;
    }
    else
    {
      lastBuffer = allocateAudioBuffer(returnCode);
      if (returnCode != ReturnCode::SUCCESS)
      {
        UAPI_ERROR(fn,"Could not allocate an AudioBuffer\n");
        return;
      }
    }
    lastBuffer->isLastBuffer = true;
    returnCode = ReturnCode::SUCCESS;
    
    insertBufferInQueue(lastBuffer, returnCode);
    return;
  }
  
  if (!samples || length == 0)
  {
    //nothing to add
    returnCode = ReturnCode::SUCCESS;
    return;
  }
  
  UINT16 bufSize = CodecHelper::GetPreferredBufferSize(codec);
  if (bufSize <= 0)
  {
    returnCode = ReturnCode::ILLEGAL_ARGUMENT;
    return;
  }
  
  
  LockScope ls(mutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to create LockScope\n");
    return;
  }
  // nothing to do, nobody is waiting for audio. The AudioQueue is shared by
  // the AudioStreamImpl objects. It is created by the Microphone or the
  // MediaFileReader. To avoid the object being deleted while used by other
  // modules, we use Ref Counting (AudioQueue : public RefCounted). If the
  // RefCount is 1, it means that no AudioStreamImpl are using this audio yet. In
  // those cases, we do not have to save the audio. Simply discard it.
  if (RefCounted::getCount() <= 1)
  {
    returnCode = ReturnCode::SUCCESS;
    return;
  }
  
  if (currentBuffer == 0)
  {
    //allocate memory forcurrentBuffer
    currentBuffer = allocateAudioBuffer(returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"Could not allocate an AudioBuffer\n");
      return;
    }
  }
  
  unsigned char* pBufToAdd = samples;
  ARRAY_LIMIT nSampleSaved = 0;
  //
  // check if new data fits in current buffer
  //
  
  // if it does not fit
  if (currentBuffer->size + length > bufSize)
  {
    //fill up the current buffer
    ARRAY_LIMIT nSpaceLeft = (bufSize - currentBuffer->size);
    
    // copy from buffer the data that fit
    memcpy(currentBuffer->buffer + currentBuffer->size, samples,
           nSpaceLeft);
    currentBuffer->size += nSpaceLeft;
    nSampleSaved = nSpaceLeft;
    
    // save new buffer to the list.
    currentBuffer->isLastBuffer = isLastBuffer;
    insertBufferInQueue(currentBuffer, returnCode);
    currentBuffer = 0;
    if (returnCode != ReturnCode::SUCCESS)
      return;
    
    AudioBuffer *pNewBuff;
    // compute how many new buffers we will need
    UINT16 bufSize = CodecHelper::GetPreferredBufferSize(codec);
    if (bufSize <= 0)
    {
      returnCode = ReturnCode::ILLEGAL_ARGUMENT;
      return;
    }
    ARRAY_LIMIT numOfBuffersToAdd = (length - nSpaceLeft) / bufSize;
    
    // fill each one of the buffer except for the last one
    for (ARRAY_LIMIT i = 0; i < numOfBuffersToAdd; ++i)
    {
      pNewBuff = allocateAudioBuffer(returnCode);
      if (returnCode != ReturnCode::SUCCESS)
      {
        UAPI_ERROR(fn,"Could not allocate an AudioBuffer\n");
        return;
      }
      
      // copy next chunk
      memcpy(pNewBuff->buffer, samples + nSampleSaved, bufSize);
      pNewBuff->size  = bufSize;
      nSampleSaved += bufSize;
      
      // save new buffer to the list.
      pNewBuff->isLastBuffer = isLastBuffer;
      insertBufferInQueue(pNewBuff, returnCode);
      if (returnCode != ReturnCode::SUCCESS)
        return;
    }
    
    length -= nSampleSaved;
    if (length > 0)
    {
      // remaining data to be added, prepare it
      pBufToAdd       = (samples + nSampleSaved);
      
      //allocate memory forcurrentBuffer
      currentBuffer = allocateAudioBuffer(returnCode);
      if (returnCode != ReturnCode::SUCCESS)
      {
        UAPI_ERROR(fn,"Could not allocate an AudioBuffer\n");
        return;
      }
      //data will get copied when we go inside BLOCK A
    }
  }
  
  //
  //  BLOCK A
  //
  if (length > 0)
  {
    memcpy(currentBuffer->buffer + currentBuffer->size,
           pBufToAdd, length);
    currentBuffer->size += length;
    
    if (currentBuffer->size == bufSize) //buffer full?
    {
      currentBuffer->isLastBuffer = isLastBuffer;
      insertBufferInQueue(currentBuffer, returnCode);
      currentBuffer = 0;
      if (returnCode != ReturnCode::SUCCESS)
        return;
    }
  }
  
  //make sure we add current buffer to the list even if it's not full. Only
  //do this when the isLastBuffer flag is set.
  if (isLastBuffer && currentBuffer)
  {
    //last buffer and current buffer was not copied.
    currentBuffer->isLastBuffer = true;
    insertBufferInQueue(currentBuffer, returnCode);
    currentBuffer = 0;
    if (returnCode != ReturnCode::SUCCESS)
      return;
  }
  
  returnCode = ReturnCode::SUCCESS;
}

AudioBuffer* AudioQueue::allocateAudioBuffer(ReturnCode::Type& returnCode)
{
  // the AudioBuffer are RefCounted. If we have 2 attached AudioStreamImpl
  // (getCount is 3), the count for each buffer has to be initialized to 2.
  // One for each attached AudioStreamImpl When the buffer are passed to the
  // consumers through the AudioStreamImpl::read function, the caller of the read
  // function will call Release to decrease the count. If the buffer are not
  // read, the count will be decremented when DetachAudio is called.
  UINT8 nInitialCount = RefCounted::getCount() - 1;
  return AudioBuffer::create(nInitialCount, CodecHelper::GetPreferredBufferSize(codec), returnCode);
}

void AudioQueue::attachAudio(SinglyLinkedNode*& out_pInitialReadPosition,
                             ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("AudioQueue::attachAudio");
    
  LockScope ls(mutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to create LockScope\n");
    return;
  }
  
  //someone new is using us. Increment the count.
  RefCounted::addRef(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to increase the ref count\n");
    return;
  }
  
  //It will start reading at the oldest buffer we have in the queue. To avoid
  //issues, we must increment the count of all the buffers in the queue.
  out_pInitialReadPosition = begin();
  FwdIterator it(begin(), end());
  while (it.hasNext())
  {
    AudioBuffer* audioBuffer = (AudioBuffer*) it.next();
    audioBuffer->addRef(returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"Failed to increase ref count on AudioBuffer\n");
      return;
    }
  }
  
  if (currentBuffer != 0)
  {
    //a buffer is currently being filled. We have to increase the count on
    //this one as well.
    currentBuffer->addRef(returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"Failed to increase ref count on AudioBuffer\n");
      return;
    }
  }
  
  returnCode = ReturnCode::SUCCESS;
}

void AudioQueue::detachAudio(SinglyLinkedNode* lastReadBuffer, ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("AudioQueue::detachAudio");
    
  LockScope ls(mutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to create LockScope\n");
    return;
  }
  
  //Decrement the count on each buffer, we have to do this because those
  //buffer will not get read and their count was intialized to the number of
  //Audio attached. We start at the next entry after the last read buffer.
  FwdIterator it(lastReadBuffer, end());
  while (it.hasNext())
  {
    AudioBuffer* audioBuffer = (AudioBuffer*) it.next();
    audioBuffer->removeRef(returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"Failed to release ref count on AudioBuffer\n");
      return;
    }
  }
  
  if (currentBuffer != 0)
  {
    //a buffer is currently being filled. We have to decrease the count on
    //this one as well.
    UINT8 count = currentBuffer->removeRef(returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"Failed to release ref count on AudioBuffer\n");
      return;
    }
    if(count == 0 )
      currentBuffer = 0;
  }
  
  //we now need to decrement the count.
  if (RefCounted::getCount() == 1)
  {
    //about to kill ourselves, make sure we unlock the mutex. The next line
    //RefCounted::Release() will call "delete this;"
    ls.cancel(returnCode);
    if (returnCode != ReturnCode::SUCCESS)
    {
      UAPI_ERROR(fn,"Failed to cancel LockScope\n");
      return;
    }
  }
  RefCounted::removeRef(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to release ref count on AudioQueue\n");
    return;
  }
  
  returnCode = ReturnCode::SUCCESS;
}

AudioBuffer* AudioQueue::read(SinglyLinkedNode*& lastReadBuffer, ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("AudioQueue::read");
    
  LockScope ls(mutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to create LockScope\n");
    return 0;
  }
  
  //check if a new buffer is ready.
  FwdIterator it(lastReadBuffer, end());
  if (it.hasNext())
  {
    //a new buffer was added since read was called the last time.
    AudioBuffer* audioBuffer = (AudioBuffer*) it.next();
    lastReadBuffer = lastReadBuffer->next;
    
    if (audioBuffer->isLastBuffer)
    {
      //to know an AudioStreamImpl has reached the END_OF_STREAM, we simply
      //point lastReadBuffer to the last element.
      lastReadBuffer = end();
    }
    
    UAPI_INFO(fn,"AudioQueue reading buffer %p\n", audioBuffer);
    returnCode = ReturnCode::SUCCESS;
    return audioBuffer;
  }
  else
  {
    if (lastReadBuffer == end())
      returnCode = ReturnCode::END_OF_STREAM;
    else
      returnCode = ReturnCode::PENDING_DATA;
    return 0;
  }
}

void AudioQueue::release(AudioBuffer* audioBuffer, ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("AudioQueue::release");
    
  LockScope ls(mutex, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to create LockScope\n");
    return;
  }
  
  if (readyForGarbageCollection != 0 && audioBuffer->getCount() == 1)
  {
    //we have an audio buffer that must be remove from the queue, i.e. we have
    //to remove it's SinglyLinkedNode.
    
    UAPI_INFO(fn,"Removing buffer %p\n", readyForGarbageCollection);
    
    //we remove the item from the queue. readyForGarbageCollection should
    //always be the first item on the queue (it's not always the case, but it
    //is 99.9% of the time). This means that a call to remove will be very
    //fast.
    remove(readyForGarbageCollection);
    
    readyForGarbageCollection = 0;
  }
  
  if (audioBuffer->getCount() == 1)
  {
    //we cannot get rid of the SinglyLinkedNode that holds this AudioBuffer now. We have
    //to wait because lastReadBuffer is pointing to this node. We mark this
    //buffer as ready for garbage collection. It will be cleaned up the next
    //time release is called if the buffer that is deleted at that time also
    //has a count of one.
    readyForGarbageCollection = audioBuffer;
  }
  UAPI_INFO(fn,"Releasing buffer %p\n", audioBuffer);
  audioBuffer->removeRef(returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to release ref count on AudioBuffer\n");
    return;
  }
  
  returnCode = ReturnCode::SUCCESS;
}

bool AudioQueue::isAtEndOfStream(SinglyLinkedNode* position) const
{
  return position == end();
}

Codec::Type AudioQueue::getCodec()
{
  return codec;
}

void AudioQueue::insertBufferInQueue(AudioBuffer* pBuf, ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("AudioQueue::insertBufferInQueue");
  UAPI_INFO(fn,"Adding buffer %p size %d is last %d\n", pBuf, pBuf->size, pBuf->isLastBuffer);
  
  push(pBuf, returnCode);
}
