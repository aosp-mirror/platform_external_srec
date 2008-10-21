/*---------------------------------------------------------------------------*
 *  AudioStreamImpl.cpp                                                      *
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
#include "AudioStreamImpl.h"
#include "AudioQueue.h"
#include "AudioBuffer.h"
#include "Logger.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

using namespace android::speech::recognition;
using namespace android::speech::recognition::impl;
using namespace android::speech::recognition::utilities;


DEFINE_SMARTPROXY(impl, AudioStreamImplProxy, AudioStreamProxy, AudioStreamImpl)

AudioStreamImplProxy AudioStreamImpl::create(AudioQueue* queue, ReturnCode::Type & returnCode)
{
  UAPI_FN_SCOPE("AudioStreamImpl::create");
    
  AudioStreamImpl* object = new AudioStreamImpl(queue, returnCode);
  if (!object)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    return AudioStreamImplProxy();
  }
  else if (returnCode)
  {
    delete object;
    return AudioStreamImplProxy();
  }
  AudioStreamImplProxy result(object);
  if (!result)
    returnCode = ReturnCode::OUT_OF_MEMORY;
  return result;
}

AudioStreamImpl::AudioStreamImpl(AudioQueue* queue, ReturnCode::Type& returnCode):
    audioQueue(queue),
    readPosition(0),
    isLocked(false)
{
  UAPI_FN_NAME("AudioStreamImpl::AudioStreamImpl");
  UAPI_TRACE(fn,"this=%p\n", this);
  
  //Attach to the audio queue. By doing so, we tell it that it need to save
  //audio buffers for us. It will also initialize the read position.
  audioQueue->attachAudio(readPosition, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to attach AudioStream to AudioQueue\n");
    return;
  }
}

AudioStreamImpl::~AudioStreamImpl()
{
  ReturnCode::Type returnCode;
  UAPI_FN_NAME("AudioStreamImpl::~AudioStreamImpl");
  UAPI_TRACE(fn,"this=%p\n", this);
  
  //detach the audio from the queue.
  audioQueue->detachAudio(readPosition, returnCode);
  if (returnCode)
    UAPI_WARN(fn,"Failed to detatch audio stream from audio queue.\n");
    
}

AudioBuffer* AudioStreamImpl::read(ReturnCode::Type& returnCode)
{
  UAPI_FN_SCOPE("AudioStreamImpl::read");
    
  AudioBuffer* result = audioQueue->read(readPosition, returnCode);
  if( result )
  {
    UAPI_INFO(fn,"AudioStream %p read AudioBuffer %p, next=%p size %d\n", this, result, readPosition, result->size);
  }
  else
  {
    UAPI_INFO(fn,"AudioStream %p read AudioBuffer %p, next=%p\n", this, result, readPosition);
  }
  return result;
}

void AudioStreamImpl::release(AudioBuffer* audioBuffer, ReturnCode::Type & returnCode)
{
  UAPI_FN_SCOPE("AudioStreamImpl::release");
    
  audioQueue->release(audioBuffer, returnCode);
  if (returnCode != ReturnCode::SUCCESS)
  {
    UAPI_ERROR(fn,"Failed to release audio %p buffer from audio queue %p\n", audioBuffer, audioQueue);
    return;
  }
}

Codec::Type AudioStreamImpl::getCodec()
{
  return audioQueue->getCodec();
}

void AudioStreamImpl::lock(ReturnCode::Type& returnCode)
{
  if (isLocked)
  {
    returnCode = ReturnCode::AUDIO_ALREADY_IN_USE;
    return;
  }
  else if (audioQueue->isAtEndOfStream(readPosition))
  {
    returnCode = ReturnCode::END_OF_STREAM;
    return;
  }
  isLocked = true;
  returnCode = ReturnCode::SUCCESS;
}

void AudioStreamImpl::unlock()
{
  // Our code shouldn't unlock twice
  assert(isLocked);
  isLocked = false;
}
