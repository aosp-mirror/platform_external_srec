/*---------------------------------------------------------------------------*
 *  main.cpp                                                                 *
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include "crtdbg.h"
#endif
#endif

#include "ReturnCode.h"
#include "AudioStream.h"
#include "MediaFileReader.h"
#include "MediaFileReaderListener.h"
#include "MediaFileWriter.h"
#include "MediaFileWriterListener.h"
#include "DeviceSpeaker.h"
#include "DeviceSpeakerListener.h"
#include "Microphone.h"
#include "MicrophoneListener.h"
#include "Codec.h"
#include "Runnable.h"
#include "WorkerQueue.h"
#include "WorkerQueueFactory.h"
#include "Task.h"
#include "Logger.h"
#include "System.h"

using namespace android::speech::recognition;
// TODO: remove dependency on android::speech::recognition::utilities
using namespace android::speech::recognition::utilities;

/*
const char*       FILENAME_SOURCE = "pcm/yes_08k.pcm";
const char*       FILENAME_SAVED  = "saved_08k.pcm";
const char*       FILENAME_RECORD = "record_08k.pcm";
const Codec::Type CODEC_FREQ      = Codec::PCM_16BIT_8K;
*/
const char*       FILENAME_SOURCE = "pcm/yes_11k.pcm";
const char*       FILENAME_SAVED  = "saved_11k.pcm";
const char*       FILENAME_RECORD = "record_11k.pcm";
const Codec::Type CODEC_FREQ      = Codec::PCM_16BIT_11K;
/*
const char*       FILENAME_SOURCE = "pcm/yes_22k.pcm";
const char*       FILENAME_SAVED  = "saved_22k.pcm";
const char*       FILENAME_RECORD = "record_22k.pcm";
const Codec::Type CODEC_FREQ      = Codec::PCM_16BIT_22K;
*/

#define CHKLOG(rc) \
  if (rc != ReturnCode::SUCCESS) \
  { \
    printf("ERROR: %s in file %s on line %d. Press ENTER...\n", ReturnCode::toString(rc), __FILE__, __LINE__); \
    fflush(NULL); \
    getchar(); \
    exit(-1); \
  }

void printBanner(const char* pMessage)
{
  printf("----------------------------------------------------------------------\n");
  printf(pMessage);
  printf("----------------------------------------------------------------------\n\n");
  fflush(stdout);
}

class Listener
{
  public:
    Listener() : m_bIsDone(false)
    {}
    
    void waitUntilDone()
    {
      while (!m_bIsDone)
        Runnable::sleep(20);
        
      m_bIsDone = false; //prepare for next wait.
    }
    
  protected:
    bool m_bIsDone;
    
};

class MyMediaFileReaderListener : public MediaFileReaderListener, public Listener
{
  public:
    virtual void onStarted()
    {
      fprintf(stderr, "MyMediaFileReaderListener::onStarted\n");
    }
    virtual void onStopped()
    {
      fprintf(stderr, "MyMediaFileReaderListener::onStopped\n");
      m_bIsDone = true;
    }
    virtual void onError(ReturnCode::Type returnCode)
    {
      fprintf(stderr, "MyMediaFileReaderListener::onError -> %s\n", ReturnCode::toString(returnCode));
    }
};
DECLARE_SMARTPROXY(, MyMediaFileReaderListenerProxy, MediaFileReaderListenerProxy, MyMediaFileReaderListener)
DEFINE_SMARTPROXY(, MyMediaFileReaderListenerProxy, MediaFileReaderListenerProxy, MyMediaFileReaderListener)


class MyDeviceSpeakerListener : public DeviceSpeakerListener, public Listener
{
  public:
    MyDeviceSpeakerListener() : m_bGotError(false){}
    virtual void onStarted()
    {
      fprintf(stderr, "MyDeviceSpeakerListener::onStarted\n");
    }
    
    virtual void onStopped()
    {
      fprintf(stderr, "MyDeviceSpeakerListener::onStopped\n");
      m_bIsDone = true;
    }
    
    virtual void onError(ReturnCode::Type returnCode)
    {
      fprintf(stderr, "MyDeviceSpeakerListener::onError -> %s\n", ReturnCode::toString(returnCode));
      m_bGotError = true;
    }
    bool m_bGotError;
};
DECLARE_SMARTPROXY(, MyDeviceSpeakerListenerProxy, DeviceSpeakerListenerProxy, MyDeviceSpeakerListener)
DEFINE_SMARTPROXY(, MyDeviceSpeakerListenerProxy, DeviceSpeakerListenerProxy, MyDeviceSpeakerListener)


class MyMediaFileWriterListener : public MediaFileWriterListener, public Listener
{
  public:
    virtual void onStopped()
    {
      fprintf(stderr, "MyMediaFileWriterListener::onStopped\n");
      m_bIsDone = true;
    }
    virtual void onError(ReturnCode::Type returnCode)
    {
      fprintf(stderr, "MyMediaFileWriterListener::onError -> %s\n", ReturnCode::toString(returnCode));
    }
};
DECLARE_SMARTPROXY(, MyMediaFileWriterListenerProxy, MediaFileWriterListenerProxy, MyMediaFileWriterListener)
DEFINE_SMARTPROXY(, MyMediaFileWriterListenerProxy, MediaFileWriterListenerProxy, MyMediaFileWriterListener)


void simpleMediaTest()
{
  ReturnCode::Type returnCode = ReturnCode::SUCCESS;
  
  UAPI_FN_SCOPE("simpleMediaTest");
  UAPI_INFO(fn,"test.exe : RUNNING TEST simpleMediaTest()\n");
  printBanner("RUNNING TEST simpleMediaTest()\n");
  
  char filename[256];
  const char* QSDK = getenv("QSDK") ? getenv("QSDK") : "/system/usr/srec";
  strcpy(filename, QSDK);
  strcat(filename, "/");
  strcat(filename, FILENAME_SOURCE);
  
  
  MediaFileReaderListenerProxy listener(new MyMediaFileReaderListener());
  MediaFileReaderProxy mediaFileReader = MediaFileReader::create(filename, listener, returnCode);
  CHKLOG(returnCode);
  
  AudioStreamProxy audio1 = mediaFileReader->createAudio(returnCode);
  CHKLOG(returnCode);
  AudioStreamProxy audio2 = mediaFileReader->createAudio(returnCode);
  CHKLOG(returnCode);
  
  //  AudioStream* audioToSave = mediaFileReader->createAudio();
  
  //set properties
  mediaFileReader->setReadingMode(MediaFileReader::REAL_TIME, returnCode);
  //mediaFileReader->SetReadingMode(MediaFileReader::ALL_AT_ONCE, returnCode);
  CHKLOG(returnCode);
  mediaFileReader->start(returnCode);
  CHKLOG(returnCode);
  
  DeviceSpeakerProxy pDevSpeaker = DeviceSpeaker::getInstance(returnCode);
  CHKLOG(returnCode);
  
  pDevSpeaker->setCodec(CODEC_FREQ, returnCode);
  CHKLOG(returnCode);
  
  MyDeviceSpeakerListenerProxy deviceListener(new MyDeviceSpeakerListener());
  if (!deviceListener)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    CHKLOG(returnCode);
  }
  pDevSpeaker->setListener(deviceListener, returnCode);
  CHKLOG(returnCode);
  
  
  pDevSpeaker->start(audio1, returnCode);
  CHKLOG(returnCode);
  
//abort it in the middle
  Runnable::sleep(1250);
  pDevSpeaker->stop(returnCode);
  CHKLOG(returnCode);
  
  //wait for the stop to complete.
  deviceListener->waitUntilDone();
  
  //pDevSpeaker->Start(audio2, returnCode);
  //CHKLOG(returnCode);
  
  //wait for the play to complete
  //devListener.WaitUntilDone();
  
  
  //MyMediaFileWriterListener writerListener;
  //MediaFileWriter* mediaFileWriter = uapi.createMediaFileWriter(&writerListener, returnCode);
  //CHKLOG(returnCode);
  
  //mediaFileWriter->Save(audioToSave, FILENAME_SAVED, returnCode);
  //CHKLOG(returnCode);
  
  //wait for the save to be done.
  //writerListener.WaitUntilDone();
  
  
  //delete audioToSave;
  //delete mediaFileWriter;
  
  UAPI_INFO(fn,"test.exe : RUNNING TEST simpleMediaTest() DONE\n");
  printBanner("RUNNING TEST simpleMediaTest() DONE\n");
}

class MyMediaFileReaderListener2 : public MediaFileReaderListener, public Listener
{
  public:
    virtual void onStarted()
    {
      fprintf(stderr, "MyMediaFileReaderListener2::onStarted\n");
    }
    
    virtual void onStopped()
    {
      fprintf(stderr, "MyMediaFileReaderListener2::onStopped\n");
      m_bIsDone = true;
      
    }
    virtual void onError(ReturnCode::Type returnCode)
    {
      fprintf(stderr, "MyMediaFileReaderListener2::onError -> %s\n", ReturnCode::toString(returnCode));
    }
};
DECLARE_SMARTPROXY(, MyMediaFileReaderListener2Proxy, MediaFileReaderListenerProxy, MyMediaFileReaderListener2)
DEFINE_SMARTPROXY(, MyMediaFileReaderListener2Proxy, MediaFileReaderListenerProxy, MyMediaFileReaderListener2)


void deleteFromCallbackTest()
{
  ReturnCode::Type returnCode = ReturnCode::SUCCESS;
  
  UAPI_FN_SCOPE("deleteFromCallbackTest");
  UAPI_INFO(fn,"test.exe : RUNNING TEST deleteFromCallbackTest()\n");
  printBanner("RUNNING TEST deleteFromCallbackTest()\n");
  
  char filename[256];
  const char* QSDK = getenv("QSDK") ? getenv("QSDK") : "/system/usr/srec";
  strcpy(filename, QSDK);
  strcat(filename, "/");
  strcat(filename, FILENAME_SOURCE);
  
  
  MyMediaFileReaderListener2Proxy listener(new MyMediaFileReaderListener2());
  if (!listener)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    CHKLOG(returnCode);
  }
  MediaFileReaderProxy mfr = MediaFileReader::create(filename, listener, returnCode);
  CHKLOG(returnCode);
  
  AudioStreamProxy audio1 = mfr->createAudio(returnCode);
  
  //set properties
  mfr->setReadingMode(MediaFileReader::REAL_TIME, returnCode);
  //mediaFileReader->setReadingMode(MediaFileReader::ALL_AT_ONCE, returnCode);
  CHKLOG(returnCode);
  mfr->start(returnCode);
  CHKLOG(returnCode);
  
  listener->waitUntilDone();
  
  UAPI_INFO(fn,"test.exe : RUNNING TEST deleteFromCallbackTest() DONE\n");
  printBanner("RUNNING TEST deleteFromCallbackTest() DONE\n");
}

void mediaTest()
{
  ReturnCode::Type returnCode = ReturnCode::SUCCESS;
  
  UAPI_FN_SCOPE("mediaTest");
  UAPI_INFO(fn,"test.exe : RUNNING TEST mediaTest()\n");
  printBanner("RUNNING TEST mediaTest()\n");
  
  char filename[256];
  const char* QSDK = getenv("QSDK") ? getenv("QSDK") : "/system/usr/srec";
  strcpy(filename, QSDK);
  strcat(filename, "/");
  strcat(filename, FILENAME_SOURCE);
  
  
  MediaFileReaderListenerProxy listener(new MyMediaFileReaderListener());
  MediaFileReaderProxy mediaFileReader = MediaFileReader::create(filename, listener, returnCode);
  CHKLOG(returnCode);
  
  AudioStreamProxy audio1 = mediaFileReader->createAudio(returnCode);
  CHKLOG(returnCode);
  AudioStreamProxy audio2 = mediaFileReader->createAudio(returnCode);
  CHKLOG(returnCode);
  AudioStreamProxy audio3 = mediaFileReader->createAudio(returnCode);
  CHKLOG(returnCode);
  AudioStreamProxy audioToSave = mediaFileReader->createAudio(returnCode);
  CHKLOG(returnCode);
  
  //set properties
  mediaFileReader->setReadingMode(MediaFileReader::REAL_TIME, returnCode);
  //mediaFileReader->setReadingMode(MediaFileReader::ALL_AT_ONCE, returnCode);
  CHKLOG(returnCode);
  mediaFileReader->start(returnCode);
  CHKLOG(returnCode);
  
  DeviceSpeakerProxy pDevSpeaker = DeviceSpeaker::getInstance(returnCode);
  CHKLOG(returnCode);
  
  pDevSpeaker->setCodec(CODEC_FREQ, returnCode);
  CHKLOG(returnCode);
  
  MyDeviceSpeakerListenerProxy deviceListener(new MyDeviceSpeakerListener());
  if (!deviceListener)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    CHKLOG(returnCode);
  }
  pDevSpeaker->setListener(deviceListener, returnCode);
  CHKLOG(returnCode);
  
  pDevSpeaker->start(audio1, returnCode);
  CHKLOG(returnCode);
  
  //wait for the play to complete
  deviceListener->waitUntilDone();
  
  //play again, this time it should fail because that audio object was already
  //used.
  pDevSpeaker->start(audio1, returnCode);
  if (returnCode != ReturnCode::END_OF_STREAM)
    exit(-1);
    
    
  //play again, this time do it on a second audio stream.
  pDevSpeaker->start(audio2, returnCode);
  CHKLOG(returnCode);
  
  //wait for the play to complete
  deviceListener->waitUntilDone();
  
  
  //play again, this time do it on a 3rd audio stream.
  pDevSpeaker->start(audio3, returnCode);
  CHKLOG(returnCode);
  
  //abort it in the middle
  Runnable::sleep(1250);
  pDevSpeaker->stop(returnCode);
  CHKLOG(returnCode);
  
  //wait for the stop to complete.
  deviceListener->waitUntilDone();
  
  
  MyMediaFileWriterListenerProxy writerListener(new MyMediaFileWriterListener());
  if (!writerListener)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    CHKLOG(returnCode);
  }
  MediaFileWriterProxy mediaFileWriter = MediaFileWriter::create(writerListener, returnCode);
  CHKLOG(returnCode);
  
  mediaFileWriter->save(audioToSave, FILENAME_SAVED, returnCode);
  CHKLOG(returnCode);
  
  //wait for the save to be done.
  writerListener->waitUntilDone();
  
  UAPI_INFO(fn,"test.exe : RUNNING TEST mediaTest() DONE\n");
  printBanner("RUNNING TEST mediaTest() DONE\n");
}

class MyMicrophoneListener : public MicrophoneListener, public Listener
{
  public:
	 MyMicrophoneListener() : started(false){}
    virtual void onStarted()
    {
      fprintf(stderr, "MyMicrophoneListener::onStarted\n");
      fprintf(stderr, "\n\n\tSPEAK NOW\n\n");
      started = true;
    }
    
    virtual void onStopped()
    {
      fprintf(stderr, "\n\n\tSTOP SPEAKING NOW\n\n");
      fprintf(stderr, "MyMicrophoneListener::onStopped\n");
      m_bIsDone = true;
    }
    
    virtual void onError(ReturnCode::Type returnCode)
    {
      fprintf(stderr, "MyMicrophoneListener::onError -> %s\n", ReturnCode::toString(returnCode));
    }

    void waitUntilStarted()
    {
      while (!started)
        Runnable::sleep(5);
        
      started = false;
    }

    bool started;
};
DECLARE_SMARTPROXY(, MyMicrophoneListenerProxy, MicrophoneListenerProxy, MyMicrophoneListener)
DEFINE_SMARTPROXY(, MyMicrophoneListenerProxy, MicrophoneListenerProxy, MyMicrophoneListener)


void recorderTest()
{
  ReturnCode::Type returnCode = ReturnCode::SUCCESS;
  
  UAPI_FN_SCOPE("recorderTest");
  UAPI_INFO(fn,"test.exe : RUNNING TEST recorderTest()\n");
  printBanner("RUNNING TEST recorderTest()\n");
  
  
  //create a microphone
  MicrophoneProxy pMicrophone = Microphone::getInstance(returnCode);
  CHKLOG(returnCode);
  
  MyMicrophoneListenerProxy micListener(new MyMicrophoneListener());
  if (!micListener)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    CHKLOG(returnCode);
  }
  pMicrophone->setListener(micListener, returnCode);
  CHKLOG(returnCode);
  pMicrophone->setCodec(CODEC_FREQ, returnCode);
  CHKLOG(returnCode);
  
  AudioStreamProxy audioToPlay = pMicrophone->createAudio(returnCode);
  CHKLOG(returnCode);
  AudioStreamProxy audioToSave = pMicrophone->createAudio(returnCode);
  CHKLOG(returnCode);
  
  //create a device speaker
  DeviceSpeakerProxy pDevSpeaker = DeviceSpeaker::getInstance(returnCode);
  CHKLOG(returnCode);
  
  pDevSpeaker->setCodec(CODEC_FREQ, returnCode);
  CHKLOG(returnCode);
  
  MyDeviceSpeakerListenerProxy deviceListener(new MyDeviceSpeakerListener());
  if (!deviceListener)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    CHKLOG(returnCode);
  }
  pDevSpeaker->setListener(deviceListener, returnCode);
  CHKLOG(returnCode);
  
  //create a file writer
  MyMediaFileWriterListenerProxy writerListener(new MyMediaFileWriterListener());
  if (!writerListener)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    CHKLOG(returnCode);
  }
  MediaFileWriterProxy mediaFileWriter = MediaFileWriter::create(writerListener, returnCode);
  CHKLOG(returnCode);
  
  
  //START RECORDING
  pMicrophone->start(returnCode);
  CHKLOG(returnCode);
  
  //save the recording into a file
  mediaFileWriter->save(audioToSave, FILENAME_RECORD, returnCode);
  CHKLOG(returnCode);

  // wait until started
  micListener->waitUntilStarted();
  
  // record for 4 seconds
  Runnable::sleep(4000);
  
  // STOP RECORDING
  pMicrophone->stop(returnCode);
  CHKLOG(returnCode);
  
  // wait for the stop to be completed
  micListener->waitUntilDone();
  
  // play it back
  pDevSpeaker->start(audioToPlay, returnCode);
  CHKLOG(returnCode);
  
  // wait for the play to complete
  deviceListener->waitUntilDone();
  
  // make sure the save is done
  writerListener->waitUntilDone();
  
//make sure the file contains all the samples.
  FILE * fp = fopen(FILENAME_RECORD, "rb");
  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  fclose(fp);
  
  if (size < 80000)
  {
    fprintf(stderr, "We did not record enough data - size %d!!!\n", size);
    UAPI_ERROR(fn,"test.exe : recorderTest() -- we did not record enough data, size of file %d\n", size);
  }
  
  UAPI_INFO(fn,"test.exe : RUNNING TEST recorderTest() DONE\n");
  printBanner("RUNNING TEST recorderTest() DONE\n");
}

void deviceSpeakerDeleteTest()
{
  ReturnCode::Type returnCode = ReturnCode::SUCCESS;
  
  UAPI_FN_SCOPE("deviceSpeakerDeleteTest");
  UAPI_INFO(fn,"test.exe : RUNNING TEST deviceSpeakerDeleteTest()\n");
  printBanner("RUNNING TEST deviceSpeakerDeleteTest()\n");
  
  char filename[256];
  const char* QSDK = getenv("QSDK") ? getenv("QSDK") : "/system/usr/srec";
  strcpy(filename, QSDK);
  strcat(filename, "/");
  strcat(filename, FILENAME_SOURCE);
  
  
  MediaFileReaderListenerProxy listener(new MyMediaFileReaderListener());
  MediaFileReaderProxy mediaFileReader = MediaFileReader::create(filename, listener, returnCode);
  CHKLOG(returnCode);
  
  AudioStreamProxy audio1 = mediaFileReader->createAudio(returnCode);
  
  //set properties
  mediaFileReader->setReadingMode(MediaFileReader::REAL_TIME, returnCode);
  CHKLOG(returnCode);
  mediaFileReader->start(returnCode);
  CHKLOG(returnCode);
  
  DeviceSpeakerProxy pDevSpeaker = DeviceSpeaker::getInstance(returnCode);
  CHKLOG(returnCode);
  
  pDevSpeaker->setCodec(CODEC_FREQ, returnCode);
  CHKLOG(returnCode);
  
  MyDeviceSpeakerListenerProxy deviceListener(new MyDeviceSpeakerListener());
  if (!deviceListener)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    CHKLOG(returnCode);
  }
  pDevSpeaker->setListener(deviceListener, returnCode);
  CHKLOG(returnCode);
  
  
  pDevSpeaker->start(audio1, returnCode);
  CHKLOG(returnCode);
  
  Runnable::sleep(200);
  
  UAPI_INFO(fn,"test.exe : RUNNING TEST deviceSpeakerDeleteTest() DONE\n");
  printBanner("RUNNING TEST deviceSpeakerDeleteTest() DONE\n");
}

void microphoneDeleteTest()
{
  ReturnCode::Type returnCode = ReturnCode::SUCCESS;
  
  UAPI_FN_SCOPE("microphoneDeleteTest");
  UAPI_INFO(fn,"test.exe : RUNNING TEST microphoneDeleteTest()\n");
  printBanner("RUNNING TEST microphoneDeleteTest()\n");
  
  
  //create a microphone
  MicrophoneProxy pMicrophone = Microphone::getInstance(returnCode);
  CHKLOG(returnCode);
  
  MyMicrophoneListenerProxy micListener(new MyMicrophoneListener());
  if (!micListener)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    CHKLOG(returnCode);
  }
  pMicrophone->setListener(micListener, returnCode);
  CHKLOG(returnCode);
  pMicrophone->setCodec(CODEC_FREQ, returnCode);
  CHKLOG(returnCode);
  
  AudioStreamProxy audioToPlay = pMicrophone->createAudio(returnCode);
  CHKLOG(returnCode);
  AudioStreamProxy audioToSave = pMicrophone->createAudio(returnCode);
  CHKLOG(returnCode);
  
  //START RECORDING
  pMicrophone->start(returnCode);
  CHKLOG(returnCode);
  
  // record for 1 seconds
  Runnable::sleep(1000);
  
  // STOP RECORDING
  pMicrophone->stop(returnCode);
  CHKLOG(returnCode);
  
  // wait for the stop to be completed
  micListener->waitUntilDone();
  UAPI_INFO(fn,"test.exe : RUNNING TEST microphoneDeleteTest() DONE\n");
  printBanner("RUNNING TEST microphoneDeleteTest() DONE\n");
}

void microphoneAudioDeleteTest()
{
  ReturnCode::Type returnCode = ReturnCode::SUCCESS;
  
  UAPI_FN_SCOPE("microphoneAudioDeleteTest");
  UAPI_INFO(fn,"test.exe : RUNNING TEST microphoneAudioDeleteTest()\n");
  
  //create a microphone
  MicrophoneProxy pMicrophone = Microphone::getInstance(returnCode);
  CHKLOG(returnCode);
  
  MyMicrophoneListenerProxy micListener(new MyMicrophoneListener());
  if (!micListener)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    CHKLOG(returnCode);
  }
  pMicrophone->setListener(micListener, returnCode);
  CHKLOG(returnCode);
  pMicrophone->setCodec(CODEC_FREQ, returnCode);
  CHKLOG(returnCode);
  
  AudioStreamProxy audioToPlay = pMicrophone->createAudio(returnCode);
  CHKLOG(returnCode);
  AudioStreamProxy audioToSave = pMicrophone->createAudio(returnCode);
  CHKLOG(returnCode);
  
  
  //START RECORDING
  pMicrophone->start(returnCode);
  CHKLOG(returnCode);
  
  // record for 3 seconds
  Runnable::sleep(3000);
  
  //de-reference the audio
  audioToPlay = AudioStreamProxy();
  audioToSave = AudioStreamProxy();
  
  
  //should not change anything. it should keep recording.
  
  UAPI_INFO(fn,"test.exe : RUNNING TEST microphoneAudioDeleteTest() DONE\n");
  printBanner("RUNNING TEST microphoneAudioDeleteTest() DONE\n");
}

void deviceSpeakerAudioDeleteTest()
{
  ReturnCode::Type returnCode = ReturnCode::SUCCESS;
  
  UAPI_FN_SCOPE("deviceSpeakerAudioDeleteTest");
  UAPI_INFO(fn,"test.exe : RUNNING TEST deviceSpeakerAudioDeleteTest()\n");
  printBanner("RUNNING TEST deviceSpeakerAudioDeleteTest()\n");
  
  char filename[256];
  const char* QSDK = getenv("QSDK") ? getenv("QSDK") : "/system/usr/srec";
  strcpy(filename, QSDK);
  strcat(filename, "/");
  strcat(filename, FILENAME_SOURCE);
  
  
  MediaFileReaderListenerProxy listener(new MyMediaFileReaderListener());
  MediaFileReaderProxy mediaFileReader = MediaFileReader::create(filename, listener, returnCode);
  CHKLOG(returnCode);
  
  AudioStreamProxy audio1 = mediaFileReader->createAudio(returnCode);
  
  //set properties
  mediaFileReader->setReadingMode(MediaFileReader::REAL_TIME, returnCode);
  CHKLOG(returnCode);
  mediaFileReader->start(returnCode);
  CHKLOG(returnCode);
  
  DeviceSpeakerProxy pDevSpeaker = DeviceSpeaker::getInstance(returnCode);
  CHKLOG(returnCode);
  
  pDevSpeaker->setCodec(CODEC_FREQ, returnCode);
  CHKLOG(returnCode);
  
  MyDeviceSpeakerListenerProxy deviceListener(new MyDeviceSpeakerListener());
  if (!deviceListener)
  {
    returnCode = ReturnCode::OUT_OF_MEMORY;
    CHKLOG(returnCode);
  }
  pDevSpeaker->setListener(deviceListener, returnCode);
  CHKLOG(returnCode);
  
  
  pDevSpeaker->start(audio1, returnCode);
  CHKLOG(returnCode);
  
  //invalidate audio1
  audio1 = AudioStreamProxy();
  
  // invoke System.dispose() without waiting for the device-speaker to shut down
  
  UAPI_INFO(fn,"test.exe : RUNNING TEST deviceSpeakerAudioDeleteTest() DONE\n");
  printBanner("RUNNING TEST deviceSpeakerAudioDeleteTest() DONE\n");
}

#if 0
void selectablePipeTest()
{
  ReturnCode::Type returnCode;
  SelectablePipe* pPipe = SelectablePipe::create(returnCode);
  CHKLOG(returnCode);
  
  
  
  fd_set fds[3];
  FD_ZERO(&fds[0]);
  FD_ZERO(&fds[1]);
  FD_ZERO(&fds[2]);
  
  FD_SET(pPipe->GetReadFD(), &fds[0]);
  
  int time_ms = 2000;
  
  struct timeval timeout;
  timeout.tv_sec = time_ms / 1000;
  timeout.tv_usec = (time_ms - timeout.tv_sec * 1000) * 1000;
  
  int num_fd = 1;
  int retval = select(num_fd, &fds[0], &fds[1], &fds[2], &timeout);
  if (retval != 0)
  {
    fprintf(stderr, "select did not timeout\n");
    exit(-1);
  }
  
  // write something on the pipe.
  char message = 'S';
  pPipe->WriteTo(&message, sizeof(message));
  
  retval = select(num_fd, &fds[0], &fds[1], &fds[2], &timeout);
  if (retval == 0)
  {
    fprintf(stderr, "select should not timeout\n");
    exit(-1);
  }
  
  char toRead;
  pPipe->ReadFrom(&toRead, sizeof(toRead));
  
  if (toRead != 'S')
  {
    fprintf(stderr, "could not read \'S\' on the pipe\n");
    exit(-1);
  }
  
  delete pPipe;
}
#endif

class TaskOne : public Task
{
  public:
    TaskOne(): Task("TaskOne")
    {}
    virtual void run()
    {
      printf("TaskOne\n");
    };
};

class TaskTwo : public Task
{
  public:
    TaskTwo(): Task("TaskTwo")
    {}
    virtual void run()
    {
      printf("TaskTwo\n");
    };
};

class TaskThree : public Task
{
  public:
    TaskThree(): Task("TaskThree")
    {}
    virtual void run()
    {
      printf("TaskThree\n");
    };
};

class TaskFour : public Task
{
  public:
    TaskFour(): Task("TaskFour")
    {}
    virtual void run()
    {
      printf("TaskFour\n");
    };
};

class ST30Ms_A : public ScheduledTask
{
  public:
    ST30Ms_A(): ScheduledTask(30, "ST30Ms_A")
    {}
    virtual void run()
    {
      printf("ST30Ms_A\n");
    };
};

class ST30Ms_B : public ScheduledTask
{
  public:
    ST30Ms_B(): ScheduledTask(30, "ST30Ms_B")
    {}
    virtual void run()
    {
      printf("ST30Ms_B\n");
    };
};

class ST10Ms_A : public ScheduledTask
{
  public:
    ST10Ms_A(): ScheduledTask(10, "ST10Ms_A")
    {}
    virtual void run()
    {
      printf("ST10Ms_A\n");
    };
};

class ST10Ms_B : public ScheduledTask
{
  public:
    ST10Ms_B(): ScheduledTask(10, "ST10Ms_B")
    {}
    virtual void run()
    {
      printf("ST10Ms_B\n");
    };
};

class ST1000Ms : public ScheduledTask
{
  public:
    ST1000Ms(): ScheduledTask(1000, "ST1000Ms")
    {}
    virtual void run()
    {
      printf("ST1000Ms\n");
    };
};



void WorkerQueueTest()
{
  ReturnCode::Type returnCode = ReturnCode::SUCCESS;
  
  UAPI_FN_SCOPE("WorkerQueueTest");
  UAPI_INFO(fn,"test.exe : RUNNING TEST WorkerQueueTest()\n");
  printBanner("RUNNING TEST WorkerQueueTest()\n");
  
  WorkerQueueFactory* factory = WorkerQueueFactory::getInstance(returnCode);
  CHKLOG(returnCode);
  
  WorkerQueue* workerQueue = factory->getWorkerQueue(returnCode);
  CHKLOG(returnCode);
  
  
  if (!workerQueue)
  {
    printf("could not create worker queue\n");
    exit(-1);
  }
  
  workerQueue->enqueue(new TaskOne(), returnCode);
  CHKLOG(returnCode);
  
  workerQueue->enqueue(new TaskOne(), returnCode);
  CHKLOG(returnCode);
  
  workerQueue->enqueue(new ST1000Ms(), returnCode);
  CHKLOG(returnCode);
  
  workerQueue->enqueue(new TaskTwo(), returnCode);
  CHKLOG(returnCode);
  
  workerQueue->enqueue(new ST30Ms_A(), returnCode);
  CHKLOG(returnCode);
  
  workerQueue->enqueue(new TaskThree(), returnCode);
  CHKLOG(returnCode);
  
  workerQueue->enqueue(new ST10Ms_A(), returnCode);
  CHKLOG(returnCode);
  
  workerQueue->enqueue(new ST30Ms_B(), returnCode);
  CHKLOG(returnCode);
  
  workerQueue->enqueue(new ST10Ms_B(), returnCode);
  CHKLOG(returnCode);
  
  workerQueue->enqueue(new TaskFour(), returnCode);
  CHKLOG(returnCode);
  
  Runnable::sleep(3000);
  UAPI_INFO(fn,"test.exe : RUNNING TEST WorkerQueueTest() DONE\n");
  printBanner("RUNNING TEST WorkerQueueTest() DONE\n");
}

/****************************************/
int main(int, char*[])
{
#if defined(_WIN32) && defined(_DEBUG)
  _CrtMemState start_state;
  _CrtMemCheckpoint(&start_state);
  
  //_CrtSetBreakAlloc(102);
#endif

  {
    ReturnCode::Type returnCode;
    System* system = System::getInstance(returnCode);
    CHKLOG(returnCode);
  

    UAPI_FN_SCOPE("main");
#ifdef UAPI_LOGGING_ENABLED
    LoggerProxy log = Logger::getInstance(returnCode);
    CHKLOG(returnCode);
    log->setLoggingLevel(Logger::LEVEL_WARN, returnCode);
    CHKLOG(returnCode);
#endif
    UAPI_INFO(fn,"test.exe : ********** START OF MAIN() **********\n");
    
    UAPI_INFO(fn,"main", "This is a test\n");
    UAPI_INFO(fn,"main", "This is a test %d\n", 40);
    UAPI_INFO(fn,"main", "This is a test %d %s\n", 40, "hi");
    
    printBanner("********** START OF MAIN() **********\n");
    
    WorkerQueueTest();
    system->dispose(returnCode);
    CHKLOG(returnCode);
    delete system;
    
    system = System::getInstance(returnCode);
    CHKLOG(returnCode);
    WorkerQueueTest();
    system->dispose(returnCode);
    CHKLOG(returnCode);
    delete system;
    
    system = System::getInstance(returnCode);
    CHKLOG(returnCode);
    mediaTest();
    system->dispose(returnCode);
    CHKLOG(returnCode);
    delete system;
    
    system = System::getInstance(returnCode);
    CHKLOG(returnCode);
    mediaTest();
    system->dispose(returnCode);
    CHKLOG(returnCode);
    delete system;
    
    system = System::getInstance(returnCode);
    CHKLOG(returnCode);
    simpleMediaTest();
    system->dispose(returnCode);
    CHKLOG(returnCode);
    delete system;
    
    system = System::getInstance(returnCode);
    CHKLOG(returnCode);
    recorderTest();
    system->dispose(returnCode);
    CHKLOG(returnCode);
    delete system;
    
    system = System::getInstance(returnCode);
    CHKLOG(returnCode);
    deleteFromCallbackTest();
    system->dispose(returnCode);
    CHKLOG(returnCode);
    delete system;
    
    system = System::getInstance(returnCode);
    CHKLOG(returnCode);
    deviceSpeakerDeleteTest();
    system->dispose(returnCode);
    CHKLOG(returnCode);
    delete system;
    
    system = System::getInstance(returnCode);
    CHKLOG(returnCode);
    microphoneDeleteTest();
    system->dispose(returnCode);
    CHKLOG(returnCode);
    delete system;
    
    system = System::getInstance(returnCode);
    CHKLOG(returnCode);
    microphoneAudioDeleteTest();
    system->dispose(returnCode);
    CHKLOG(returnCode);
    delete system;
    
    system = System::getInstance(returnCode);
    CHKLOG(returnCode);
    deviceSpeakerAudioDeleteTest();
    system->dispose(returnCode);
    CHKLOG(returnCode);
    delete system;
    
    UAPI_INFO(fn,"test.exe : ********** END OF MAIN() **********\n");
    printBanner("********** END OF MAIN() **********\n");
  }
#if defined(_WIN32) && defined(_DEBUG)
  _CrtMemDumpAllObjectsSince(&start_state);
#endif
  
  return 0;
}

