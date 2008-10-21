/*---------------------------------------------------------------------------*
 *  AudioBuffer.cpp                                                          *
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
#include "WavePCMHelper.h"
#include "File.h"
#include "Logger.h"
#include "CodecHelper.h"

#include <stdlib.h>
#include <string.h>

using namespace android::speech::recognition;
using namespace android::speech::recognition::utilities;



static void fwriteId(const char* id, File* file, ReturnCode::Type & returnCode) 
{
	UINT32 numItems = 1;
	file->write((void*)id, sizeof(long), numItems, returnCode);
}

static void fwriteLong(long data, File* file, ReturnCode::Type & returnCode)
{
	UINT32 numItems = 1;
	file->write(&data, sizeof(data), numItems, returnCode);
}

static void fwriteShort(INT16 data, File* file, ReturnCode::Type & returnCode)
{
	UINT32 numItems = 1;
	file->write(&data, sizeof(data), numItems, returnCode);
}

/* definition from http://ccrma.stanford.edu/courses/422/projects/WaveFormat */
void WavePCMHelper::writeWavHeader(File * file,
		INT32 sampleRate, 
		INT16 numChannels, 
		INT16 bitsPerSample, 
		INT32 numBytes, 
		ReturnCode::Type & returnCode)
{
	UAPI_FN_SCOPE("WavePCMHelper::writeWavHeader");
	/* RIFF header */
	fwriteId("RIFF", file, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
		return;
	fwriteLong(36 + numBytes, file, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
		return;
	fwriteId("WAVE", file, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
		return;

	/* fmt chunk */
	fwriteId("fmt ", file, returnCode); 
	if( returnCode != ReturnCode::SUCCESS )
		return;
	fwriteLong(16, file, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
		return;
	fwriteShort(1, file, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
		return;
	fwriteShort(numChannels, file, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
		return;
	fwriteLong(sampleRate, file, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
		return;
	fwriteLong(numChannels * sampleRate * bitsPerSample / 8, file, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
		return;
	fwriteShort(numChannels * bitsPerSample / 8, file, returnCode); 
	if( returnCode != ReturnCode::SUCCESS )
		return;
	fwriteShort(bitsPerSample, file, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
		return;

	/* data chunk */
	fwriteId("data", file, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
		return;
	fwriteLong(numBytes, file, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
		return;
}


void WavePCMHelper::updateFileLength(File * file, INT32 numBytes, ReturnCode::Type & returnCode)
{
	UAPI_FN_SCOPE("WavePCMHelper::updateFileLength");

  file->seek(4, File::UAPI_SEEK_SET, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
	{
		UAPI_WARN(fn,"Could not seek to position 4\n");
		return;
	}

	fwriteLong(36 + numBytes, file, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
	{
		UAPI_WARN(fn,"Failed to update the ChunkSize\n");
		return;
	}
  
  file->seek(40, File::UAPI_SEEK_SET, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
	{
		UAPI_WARN(fn,"Could not seek to position 40\n");
		return;
	}

	fwriteLong(numBytes, file, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
	{
		UAPI_WARN(fn,"Failed to update the subChunk2Size\n");
		return;
	}
}

void WavePCMHelper::getWavFileInfo( File * file, 
                                    Codec::Type & out_codec, 
                                    UINT16 & out_headerOffset,
                                    ReturnCode::Type & returnCode )
{
  UAPI_FN_SCOPE("WavePCMHelper::getWavFileInfo");

  //we only support two formats
  //1) Sphere NIST
  //2) RIFF wave


  //go at the beginning of the file. 
  file->seek(0, File::UAPI_SEEK_SET, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
	{
		UAPI_WARN(fn,"Could not seek to position 0\n");
		return;
	}

  UINT32 numToRead = 4;
  char szHeader[5];
  file->read( szHeader, sizeof(char), numToRead, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
	{
		UAPI_WARN(fn,"Could not read file header\n");
		return;
	}
  szHeader[4] = '\0';

  if( strcmp(szHeader, "RIFF") == 0 )
  {
    out_headerOffset = 44;
    getRIFFfileInfo(file, out_codec, returnCode);
    return;
  }
  else if( strcmp(szHeader, "NIST") == 0 )
  {
    out_headerOffset = 1024;
    getNISTfileInfo(file, out_codec, returnCode);
    return;
  }
  else
  {
    UAPI_ERROR(fn,"Unsupported wave file format. Supported formats are NIST"
        " Sphere and Wave RIFF.\n");
    returnCode = ReturnCode::NOT_SUPPORTED;
    return;
  }
}

void WavePCMHelper::getRIFFfileInfo( File * file, 
                                    Codec::Type & out_codec, 
                                    ReturnCode::Type & returnCode )
{
  UAPI_FN_SCOPE("WavePCMHelper::getRIFFfileInfo");

  //the sample rate is located at position 24
  file->seek(24, File::UAPI_SEEK_SET, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
	{
		UAPI_WARN(fn,"Could not seek to position 0\n");
		return;
	}

  //read the SampleRate @ index 24
  long sample_rate;
  UINT32 numToRead = 1;
  file->read( &sample_rate, sizeof(long), numToRead, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
	{
		UAPI_WARN(fn,"Could not read sample_rate\n");
		return;
	}


	//read BitsPerSample @index 34
  file->seek(34, File::UAPI_SEEK_SET, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
	{
		UAPI_WARN(fn,"Could not seek to position 0\n");
		return;
	}
	
	INT16 sample_num_bits;
  numToRead = 1;
  file->read( &sample_num_bits, sizeof(INT16), numToRead, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
	{
		UAPI_WARN(fn,"Could not read sample_num_bytes\n");
		return;
	}
  INT16 sample_num_bytes = sample_num_bits / 8;

	out_codec = CodecHelper::GetCodecFromRateNumBytesPerSample(sample_rate, sample_num_bytes, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
	{
		UAPI_WARN(fn,"Could not find a supported codec that matches sample rate %ld, "
				"num samples per byte %d\n", sample_rate, sample_num_bytes);
		return;
	}


}

void WavePCMHelper::getNISTfileInfo( File * file, 
                                    Codec::Type & out_codec, 
                                    ReturnCode::Type & returnCode )
{
  UAPI_FN_SCOPE("WavePCMHelper::getNISTfileInfo");

  //TODO this is a very basic implementation and it is only used for our
  //internal testing. If we want to fully support this format, more code should
  //be added.

  /*
     NIST_1A
     1024
     sample_count -i 24707
     sample_n_bytes -i 2
     channel_count -i 1
     sample_byte_format -s2 01
     sample_rate -i 11025
     sample_coding -s3 pcm
     end_head
  */

  //go at the beginning of the file. 
  file->seek(0, File::UAPI_SEEK_SET, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
	{
		UAPI_WARN(fn,"Could not seek to position 0\n");
		return;
	}

  //read the full header
  UINT32 numToRead = 1024;
  char szHeader[1025];
  file->read( szHeader, sizeof(char), numToRead, returnCode);
	if( returnCode != ReturnCode::SUCCESS )
	{
		UAPI_WARN(fn,"Could not read file header\n");
		return;
	}
  szHeader[1024] = '\0';

  //find the "sample_rate -i "
  static const char * const NIST_SAMPLE_RATE = "sample_rate -i ";
  char * sample_rate_start = strstr(szHeader, NIST_SAMPLE_RATE );
  if( sample_rate_start == 0 )
  {
    UAPI_WARN(fn,"NIST files does not contain %s\n", NIST_SAMPLE_RATE);
    returnCode = ReturnCode::NOT_SUPPORTED;
    return;
  }
  //find the end
  char * end = strstr(sample_rate_start + strlen(NIST_SAMPLE_RATE), "\n");
  if( end == 0 )
  {
    UAPI_WARN(fn,"NIST files does not contain end of %s\n", NIST_SAMPLE_RATE);
    returnCode = ReturnCode::NOT_SUPPORTED;
    return;
  }
  *end = '\0';

  INT32 sample_rate = atoi(sample_rate_start + strlen(NIST_SAMPLE_RATE));

  //find the "sample_n_bytes -i" 
  static const char * const NIST_SAMPLE_NUM_BYES = "sample_n_bytes -i ";
  char * sample_num_bytes_start = strstr(szHeader, NIST_SAMPLE_NUM_BYES );
  if( sample_num_bytes_start == 0 )
  {
    UAPI_WARN(fn,"NIST files does not contain %s\n", NIST_SAMPLE_NUM_BYES);
    returnCode = ReturnCode::NOT_SUPPORTED;
    return;
  }
  //find the end
  end = strstr(sample_num_bytes_start + strlen(NIST_SAMPLE_NUM_BYES), "\n");
  if( end == 0 )
  {
    UAPI_WARN(fn,"NIST files does not contain end of %s\n", NIST_SAMPLE_NUM_BYES);
    returnCode = ReturnCode::NOT_SUPPORTED;
    return;
  }
  *end = '\0';

  INT16 sample_num_bytes = (INT16)atoi(sample_num_bytes_start + strlen(NIST_SAMPLE_NUM_BYES));

  out_codec = CodecHelper::GetCodecFromRateNumBytesPerSample(sample_rate, sample_num_bytes, returnCode);
  if( returnCode != ReturnCode::SUCCESS )
	{
		UAPI_WARN(fn,"Could not find a supported codec that matches sample rate %d, "
      "num samples per byte %d\n", sample_rate, sample_num_bytes);
		return;
	}
}


