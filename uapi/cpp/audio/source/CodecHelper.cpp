/*---------------------------------------------------------------------------*
 *  CodecHelper.cpp                                                          *
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
#include <assert.h>
#include "CodecHelper.h"
#include "assert.h"
#include "Codec.h"

using namespace android::speech::recognition;
using namespace android::speech::recognition::utilities;


/**
 * 120 ms of PCM_16_8K is
 *
 * 8000 samples/sec * 16 bits/samples * 0.120 sec * 1/8 bytes/bits = 1920
 * bytes.
 */
#ifdef ANDROID
  static const UINT16 REC_BUFFER_SIZE_PCM_16_8K = 1536;
#else
  static const UINT16 REC_BUFFER_SIZE_PCM_16_8K = 1920;
#endif

/**
 * 120 ms of ULAW_8_8K is
 *
 * 8000 samples/sec * 8 bits/samples * 0.120 sec * 1/8 bytes/bits = 960
 * bytes.
 */
static const UINT16 REC_BUFFER_SIZE_ULAW_8_8K = 960;

/**
 * 120 ms of PCM_16_11K is
 *
 * 11025 samples/sec * 16 bits/samples * 0.120 sec * 1/8 bytes/bits = 2646
 * bytes.
 */
#ifdef ANDROID
  static const UINT16 REC_BUFFER_SIZE_PCM_16_11K = 2048;
#else
  static const UINT16 REC_BUFFER_SIZE_PCM_16_11K = 2646;
#endif

/**
 * 120 ms of PCM_16_22K is
 *
 * 22050 samples/sec * 16 bits/samples * 0.120 sec * 1/8 bytes/bits = 5292
 * bytes.
 */
static const UINT16 REC_BUFFER_SIZE_PCM_16_22K = 5292;
/**
 * 120 ms of ULAW is
 *
 * 8000 samples/sec * 8 bits/samples * 0.120 sec * 1/8 bytes/bits = 960
 * bytes.
 */
static const UINT16 REC_BUFFER_SIZE_ULAW = 960;

/** 
 * Those correspond to 120 ms of audio.
 */
static const UINT16 FILE_READER_BUFFER_SIZE_PCM_16_8K = 1920;
static const UINT16 FILE_READER_BUFFER_SIZE_ULAW_8_8K = 960;
static const UINT16 FILE_READER_BUFFER_SIZE_PCM_16_11K = 2646;
static const UINT16 FILE_READER_BUFFER_SIZE_PCM_16_22K = 5292;
static const UINT16 FILE_READER_BUFFER_SIZE_ULAW       = 960;












CodecHelper::CodecHelper()
{}

UINT16 CodecHelper::GetPreferredBufferSize(Codec::Type codec)
{
// For 8K and 11K I have changed the buffer size to 256 samples ( 512 bytes ) for 11K
// and 192 samples ( 384 bytes ) for 8K to match
// what is currently delivered by the Google audio driver. This code is tied to the
// changed audio polling time in the RecognizerImpl code that is meant to handle
// the smaller sample size and improve latency.  SteveR
// Because of performance issues and because Google starts collecting audio during the
// startup, I am making the audio buffers larger. They will now be for 11K 1024 samples
// ( 2048 bytes ) and for 8K 768 samples ( 1536 bytes ) Polling is no longer an issue
// since Google collects audio long before recognition starts. SteveR

  switch (codec)
  {
    case Codec::PCM_16BIT_8K:
      return REC_BUFFER_SIZE_PCM_16_8K;
    case Codec::PCM_16BIT_11K:
      return REC_BUFFER_SIZE_PCM_16_11K;
    case Codec::PCM_16BIT_22K:
      return REC_BUFFER_SIZE_PCM_16_22K;
    case Codec::ULAW_8BIT_8K:
      return REC_BUFFER_SIZE_ULAW_8_8K;
    default:
      assert(false);
      return 1;
  }
}

UINT16 CodecHelper::GetPreferredFileReaderBufferSize(Codec::Type codec)
{
  switch (codec)
  {
    case Codec::PCM_16BIT_8K:
      return FILE_READER_BUFFER_SIZE_PCM_16_8K;
    case Codec::PCM_16BIT_11K:
      return FILE_READER_BUFFER_SIZE_PCM_16_11K;
    case Codec::PCM_16BIT_22K:
      return FILE_READER_BUFFER_SIZE_PCM_16_22K;
    case Codec::ULAW_8BIT_8K:
      return FILE_READER_BUFFER_SIZE_ULAW_8_8K;
    default:
      assert(false);
      return 1;
  }
}

UINT32 CodecHelper::GetPreferredRealTimeDelay(Codec::Type codec)
{
  switch (codec)
  {
    case Codec::PCM_16BIT_8K:
      //120 ms if REC_BUFFER_SIZE_PCM_16_8K is 1920
      return (REC_BUFFER_SIZE_PCM_16_8K / (2*8));
    case Codec::PCM_16BIT_11K:
      //120 ms if REC_BUFFER_SIZE_PCM_16_11K is 2646
      return ((REC_BUFFER_SIZE_PCM_16_11K * 1000) / (2*11025));
    case Codec::PCM_16BIT_22K:
      //120 ms if REC_BUFFER_SIZE_PCM_16_22K is 5292
      return ((REC_BUFFER_SIZE_PCM_16_22K * 1000) / (2*22050));
    case Codec::ULAW_8BIT_8K:
      //120 ms if REC_BUFFER_SIZE_ULAW_8_8K is 960
      return (REC_BUFFER_SIZE_ULAW_8_8K / (8));
    default:
      assert(false);
      return 1;
  }
}

UINT32 CodecHelper::GetNumMsecForNumBytes(Codec::Type codec, UINT32 fileLength)
{
  switch (codec)
  {
    case Codec::PCM_16BIT_8K:
      return (fileLength/ (2*8));
    case Codec::PCM_16BIT_11K:
      return ((fileLength * 1000)/ (2*11025));
    case Codec::PCM_16BIT_22K:
      return ( (fileLength * 1000) / (2*22050));
    case Codec::ULAW_8BIT_8K:
      return (fileLength / (8));
    default:
      assert(false);
      return 1;
  }
}

Codec::Type CodecHelper::GetCodecFromRateNumBytesPerSample(INT32 sampleRate, INT16 numBytesPerSample, ReturnCode::Type & returnCode)
{
  returnCode = ReturnCode::SUCCESS;
  if( sampleRate == 11025 && numBytesPerSample == 2 )
  {
    return Codec::PCM_16BIT_11K;
  }
  else if( sampleRate == 8000 && numBytesPerSample == 2 )
  {
    return Codec::PCM_16BIT_8K;
  }
  else if( sampleRate == 8000 && numBytesPerSample == 1 )
  {
    return Codec::ULAW_8BIT_8K;
  }
  else if( sampleRate == 22050 && numBytesPerSample == 2 )
  {
    return Codec::PCM_16BIT_22K;
  }
  else
  {
    returnCode = ReturnCode::NOT_SUPPORTED;
    return Codec::PCM_16BIT_11K;
  }
}
