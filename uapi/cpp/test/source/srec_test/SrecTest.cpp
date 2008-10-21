/*---------------------------------------------------------------------------*
 *  SrecTest.cpp                                                             *
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
#if defined(_DEBUG) && defined(_WIN32)
#include "crtdbg.h"
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif
#endif

#include "ReturnCode.h"
#include "EmbeddedRecognizer.h"
#include "Recognizer.h"
#include "SrecGrammar.h"
#include "RecognizerListener.h"
#include "Recognizer.h"
#include "WordItem.h"
#include "AudioStream.h"
#include "MediaFileReader.h"
#include "MediaFileReaderListener.h"
#include "Codec.h"
#include "Runnable.h"
#include "NBestRecognitionResult.h"
#include "EmbeddedGrammarListener.h"
#include "Logger.h"
#include "System.h"

#define CHKLOG(returnCode) \
  if (returnCode != ReturnCode::SUCCESS) \
  { \
    printf("ERROR: %s in file %s on line %d. Press ENTER...\n", ReturnCode::toString(returnCode), \
           __FILE__, __LINE__); \
    fflush(NULL); \
    getchar(); \
    exit(-1); \
  }

using namespace android::speech::recognition;
// TODO: we should not be exporting android::speech::recognition::utilities
using namespace android::speech::recognition::utilities;


// ---- GLOBAL STATE ----
bool recognitionComplete;
// ---- GLOBAL STATE ----


class MyRecognizerListener: public RecognizerListener
{
  public:
    MyRecognizerListener()
    {}
    virtual void onBeginningOfSpeech()
    {
      printf("beginning of speech\n");
    }
    
    virtual void onEndOfSpeech()
    {
      printf("end of speech\n");
    }
    
    virtual void onRecognitionSuccess(RecognitionResultProxy& result)
    {
      NBestRecognitionResultProxy& nbest = (NBestRecognitionResultProxy&) result;
      ReturnCode::Type returnCode;
      for (ARRAY_LIMIT nbestIndex = 0, size = nbest->getSize(); nbestIndex < size; ++nbestIndex)
      {
        printf("NBestList[%u] results:\n", nbestIndex);
        NBestRecognitionResult::EntryProxy entry = nbest->getEntry(nbestIndex, returnCode);
        
        CHKLOG(returnCode);
        printf("----------------------\n");

        const char* semanticMeaning = entry->getSemanticMeaning(returnCode);
        if (returnCode != ReturnCode::SUCCESS)
          return;
        printf("semanticMeaning = %s\n", semanticMeaning);
        const char * const * keys = entry->getKeys();
        for( ARRAY_LIMIT i = 0; i < entry->getKeyCount(); i++ )
        {
          printf("\t%s=%s\n", keys[i], entry->getValue(keys[i], returnCode));
          CHKLOG(returnCode);
        }
        const char* literalMeaning = entry->getLiteralMeaning(returnCode);
        CHKLOG(returnCode);
        printf("literalMeaning = %s\n", literalMeaning);
        printf("confidencScore = %d\n", entry->getConfidenceScore(returnCode));
        CHKLOG(returnCode);


        printf("----------------------\n\n");
        CHKLOG(returnCode);

      }
      printf("recognition result\n");
    }
    
    virtual void onStarted()
    {
      printf("recognizer started\n");
    }
    
    virtual void onRecognitionFailure(RecognizerListener::FailureReason reason)
    {
      printf("Recognition failure: %s\n", toString(reason));
    }
    
    virtual void onError(ReturnCode::Type returnCode)
    {
      printf("Recognizer error: %s\n", ReturnCode::toString(returnCode));
    }
    
    virtual void onStopped()
    {
      printf("recognizer stopped\n");
      recognitionComplete = true;
    }
    
    virtual void onParametersGetError(const char** keys, android::speech::recognition::ARRAY_LIMIT count,
                                      ReturnCode::Type)
    {
      printf("onParametersGetError:\n");
      for (ARRAY_LIMIT i = 0; i < count; ++i)
        printf("%s\n", keys[i]);
    }
    
    virtual void onParametersSetError(const char** keys, const char** values,
                                      ARRAY_LIMIT count,
                                      ReturnCode::Type)
    {
      printf("onParametersSetError:\n");
      for (ARRAY_LIMIT i = 0; i < count; ++i)
        printf("%s=%s\n", keys[i], values[i]);
    }
    
    virtual void onParametersSet(const char** keys, const char** values, ARRAY_LIMIT count)
    {
      printf("onParametersSet:\n");
      for (ARRAY_LIMIT i = 0; i < count; ++i)
        printf("%s=%s\n", keys[i], values[i]);
    }
    
    virtual void onParametersGet(const char** keys, const char** values, ARRAY_LIMIT count)
    {
      printf("onParametersGet:\n");
      for (ARRAY_LIMIT i = 0; i < count; ++i)
        printf("%s=%s\n", keys[i], values[i]);
    }
    
    virtual void onAcousticStateReset()
    {
      printf("onAcousticStateReset\n");
    }
};

class MyGrammarListener: public EmbeddedGrammarListener
{
  public:
    virtual void onError(ReturnCode::Type returnCode)
    {
      printf(">>>>> Grammar operation failed: %s\n", ReturnCode::toString(returnCode));
    }
    
    virtual void onCompileAllSlots()
    {
      printf(">>>>> Grammar.compileAllSlots() complete\n");
    }
    
    virtual void onResetAllSlots()
    {
      printf(">>>>> Grammar.resetAllSlots() complete\n");
    }
    
    virtual void onLoaded()
    {
      printf(">>>>> Grammar loaded\n");
    }
    
    virtual void onUnloaded()
    {
      printf(">>>>> Grammar unloaded\n");
    }
    
    virtual void onSaved(const char* path)
    {
      printf(">>>>> Grammar saved to %s\n", path);
    }
    
    virtual bool isEmbeddedGrammarListener()
    {
      return true;
    }
    virtual bool isSrecGrammarListener()
    {
      return false;
    }
};



// -----------------------------------------------
void embeddedTest()
{
  ReturnCode::Type returnCode = ReturnCode::SUCCESS;
  
  UAPI_FN_SCOPE("embeddedTest");
#ifdef UAPI_LOGGING_ENABLED
  LoggerProxy logger = Logger::getInstance(returnCode);
  CHKLOG(returnCode);

  logger->info("SrecTest.exe", "RUNNING TEST embeddedTest()\n");
#endif

  char path[256];
  const char* ESRSDK = getenv("ESRSDK") ? getenv("ESRSDK") : "/system/usr/srec";
  strcpy(path, ESRSDK);
  strcat(path, "/config/en.us/baseline11k.par");
  
  EmbeddedRecognizerProxy recognizer = EmbeddedRecognizer::getInstance(returnCode);
  CHKLOG(returnCode);
  
  recognizer->configure(path, returnCode);
  CHKLOG(returnCode);

  //BUG 4544
  //Call configure again just to make sure it will not crash.
  recognizer->configure(path, returnCode);
  CHKLOG(returnCode);
  
  RecognizerListenerProxy recognizerListener(new MyRecognizerListener());
  recognizer->setListener(recognizerListener, returnCode);
  CHKLOG(returnCode);
  
  const char* keys[] =
  {
    "SREC.Recognizer.utterance_timeout",
    "CREC.Recognizer.terminal_timeout",
    "CREC.Recognizer.optional_terminal_timeout",
    "CREC.Recognizer.non_terminal_timeout",
    "CREC.Recognizer.eou_threshold",
    "InvalidParameter"
  };
  
  const char* values[] =
  {
    "1000",
    "1250",
    "1500",
    "1250",
    "1000",
    "50"
  };
  
  recognizer->getParameters(keys, 6, returnCode);
  CHKLOG(returnCode);
  
  recognizer->setParameters(keys, values, 6, returnCode);
  CHKLOG(returnCode);
  
  strcpy(path, "file://");
  strcat(path, ESRSDK);
  strcat(path, "/config/en.us/grammars/bothtags5.g2g");
  GrammarListenerProxy grammarListener(new MyGrammarListener());
  SrecGrammarProxy grammar(recognizer->createGrammar(path, grammarListener, returnCode));
  CHKLOG(returnCode);
  grammar->load(returnCode);
  CHKLOG(returnCode);
  
  const char* pronunciation = "jen&p)rkP";
  WordItemProxy word = WordItem::create("Jen_Parker", pronunciation, returnCode);
  int weight = 0;
  grammar->addItem("@Names", word, weight, "V='Jen_Parker'", returnCode);
  CHKLOG(returnCode);
  grammar->compileAllSlots(returnCode);
  CHKLOG(returnCode);
  grammar->save("grammar.g2g", returnCode);
  CHKLOG(returnCode);
  
  
  strcpy(path, ESRSDK);
  strcat(path, "/config/en.us/audio/v139/v139_113.nwv");
  MediaFileReaderListenerProxy listener;
  MediaFileReaderProxy mediaFileReader = MediaFileReader::create(path, listener, returnCode);
  CHKLOG(returnCode);
  AudioStreamProxy audio = mediaFileReader->createAudio(returnCode);
  CHKLOG(returnCode);
  mediaFileReader->start(returnCode);
  CHKLOG(returnCode);
  
  recognitionComplete = false;

  recognizer->recognize(audio, grammar, returnCode);
  CHKLOG(returnCode);
  
  
  // Wait for recognition to finish
  while (!recognitionComplete)
    Runnable::sleep(100);
  recognitionComplete = false;

  
  grammar->unload(returnCode);
  CHKLOG(returnCode);
  


#ifdef UAPI_LOGGING_ENABLED
  logger->info("SrecTest.exe", "RUNNING TEST embeddedTest() DONE\n");
#endif
}

// -----------------------------------------------
int main(int, char*[])
{
#if defined(_DEBUG) && defined(_WIN32)
  _CrtMemState start_state;
  _CrtMemCheckpoint(&start_state);
  
  //_CrtSetBreakAlloc(188);
#endif
  {
    ReturnCode::Type returnCode = ReturnCode::SUCCESS;
    
    UAPI_FN_SCOPE("main");
#ifdef UAPI_LOGGING_ENABLED
    LoggerProxy logger = Logger::getInstance(returnCode);
    CHKLOG(returnCode);

    logger->setLoggingLevel(Logger::LEVEL_WARN, returnCode);
    CHKLOG(returnCode);
#endif

    //delete the singleton(s), optional step
    System* system = System::getInstance(returnCode);
    CHKLOG(returnCode);
    embeddedTest();
    system->dispose(returnCode);
    CHKLOG(returnCode);
    delete system;
    
    system = System::getInstance(returnCode);
    CHKLOG(returnCode);
    embeddedTest();
    
    system->dispose(returnCode);
    CHKLOG(returnCode);
    delete system;
    
    system = System::getInstance(returnCode);
    CHKLOG(returnCode);
    embeddedTest();
    
    system->dispose(returnCode);
    CHKLOG(returnCode);
    delete system;
  }
#if defined(_DEBUG) && defined(_WIN32)
  _CrtMemDumpAllObjectsSince(&start_state);
  
  //_CrtDumpMemoryLeaks();
#endif
  return 0;
}
