/*---------------------------------------------------------------------------*
 *  AudioQueue.h                                                             *
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

#ifndef __UAPI__AUDIOQUEUE
#define __UAPI__AUDIOQUEUE

#include "exports.h"
#include "ReturnCode.h"
#include "Queue.h"
#include "Codec.h"
#include "RefCounted.h"

namespace android
{
  namespace speech
  {
    namespace recognition
    {
      namespace utilities
      {
        class AudioBuffer;
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
         * A collection of AudioBuffers shared amongst all AudioStreamImpl.
         */
        class AudioQueue: public RefCounted,
              protected Queue
        {
          public:
          
            /**
             * Creates an instance of this class
             *
             * @param codec the audio format of the samples that will be added
             * (AddBuffer) into this queue.
             * @param returnCode SUCCESS unless a fatal error has occured
             * @return a valid AudioQueue pointer if returnCode is SUCCESS, 0 if it failed.
             */
            UAPI_EXPORT static AudioQueue* create(Codec::Type codec, ReturnCode::Type& returnCode);
            
            /**
             * destructor
             */
            virtual ~AudioQueue();
            
            /**
             * Add audio samples to this audio container.
             *
             * @param samples pointer to audio sample memory.
             * @param length number of samples in pSamples.
             * @param isLastSample true no more samples are going to be added.
             * @param returnCode ILLEGAL_ARGUMENT if one of the arguement is invalid,
             *                   SUCCESS unless a fatal error occured.
             */
            UAPI_EXPORT void addBuffer(unsigned char* samples, ARRAY_LIMIT length,
                           bool isLastBuffer, ReturnCode::Type& returnCode);
                           
            /**
             * Call this function to know the audio format of the buffers returned by
             * read().
             */
            Codec::Type getCodec();
            
            
            /**
             * Call this function to attach an AudioStreamImpl to this AudioQueue.
             * @param pInitialReadPosition Will return to the initial read position
             * inside the audio queue.
             * @param SUCCESS unless a fatal error has occured.
             */
            void attachAudio(SinglyLinkedNode*& out_pInitialReadPosition, ReturnCode::Type& returnCode);
            
            /**
             * Call this function when an AudioStreamImpl is no longer using the queue.
             * @param SUCCESS unless a fatal error has occured.
             */
            void detachAudio(SinglyLinkedNode* pLastReadBuffer, ReturnCode::Type& returnCode);
            
            /**
             * Returns the AudioBuffers at the specified position in the queue.
             *
             * @param position Pointer to the node to read from. Will be updated if the
             * read is successful.
             * @param returnCode END_OF_STREAM if no more samples can be read, PENDING_DATA if no data
             * is available now, SUCCESS if a buffer is available.
             * @return the audio buffer at the specified position, or 0 if no data is
             * available. In the latter case, the return code indicates whether more
             * data will be forthcoming.
             */
            AudioBuffer* read(utilities::SinglyLinkedNode*& position,
                              ReturnCode::Type& returnCode);
                              
            using RefCounted::removeRef;
            
            /**
             * Releases the buffer that was acquired by calling read.
             * @see read.
             * @param audioBuffer the audio buffer to release.
             * @param SUCCESS unless a fatal error has occured.
             */
            void release(AudioBuffer* audioBuffer, ReturnCode::Type& returnCode);
            
            /**
             * Check if the attached audio object is pointing at the end of the stream.
             * If it does, this means that this Audio object is no longer usable.
             *
             * @param position Pointer to the node to read from. Will be updated if the
             * read is successful.
             * @return true if position points to the end of the stream.
             */
            bool isAtEndOfStream(utilities::SinglyLinkedNode* position) const;
            
          private:
            /**
             * constructor
             * @param codec the audio format of the samples that will be added
             * (AddBuffer) into this queue.
             * @param returnCode SUCCESS unless a fatal error has occured
             */
            AudioQueue(Codec::Type codec, ReturnCode::Type& returnCode);
            
            /**
             * internal function used to allocate AudioBuffer. It allocates them with
             * the right reference count, which correspond to the number of attached
             * AudioStreamImpl.
             * @param returnCode the returnCode
             * @return a new AudioBuffer or 0 if it failed.
             */
            AudioBuffer* allocateAudioBuffer(ReturnCode::Type& returnCode);
            
            /**
             * inserting a buffer into the queue.
             *
             * @param pBuf the audio buffer to insert.
             * @param returnCode the return code
             */
            void insertBufferInQueue(AudioBuffer* pBuf, ReturnCode::Type& returnCode);
            
            /**
             * Buffer that is currently being filled.
             */
            AudioBuffer* currentBuffer;
            /**
             * audio format of the queued audio buffers.
             */
            Codec::Type codec;
            /**
             * Protect the class from calls from different threads.
             */
            Mutex* mutex;
            
            /**
             * Tells if we have an audio buffer that was released and that is marked as
             * ready for garbage collection.
             */
            AudioBuffer* readyForGarbageCollection;
        };
      }
    }
  }
}

#endif
