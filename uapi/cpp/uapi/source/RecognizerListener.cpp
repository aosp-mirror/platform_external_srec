/*---------------------------------------------------------------------------*
 *  RecognizerListener.cpp                                                   *
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
#include "RecognizerListener.h"


using namespace android::speech::recognition;
using namespace android::speech::recognition::utilities;


DEFINE_SMARTPROXY(android::speech::recognition, RecognizerListenerProxy, SmartProxy, RecognizerListener)

RecognizerListener::RecognizerListener()
{
}

RecognizerListener::~RecognizerListener()
{
}

const char* RecognizerListener::toString(RecognizerListener::FailureReason reason)
{
  switch (reason)
  {
    case NO_MATCH:
      return "NO_MATCH";
    case SPOKE_TOO_SOON:
      return "SPOKE_TOO_SOON";
    case BEGINNING_OF_SPEECH_TIMEOUT:
      return "BEGINNING_OF_SPEECH_TIMEOUT";
    case RECOGNITION_TIMEOUT:
      return "RECOGNITION_TIMEOUT";
    case TOO_MUCH_SPEECH:
      return "TOO_MUCH_SPEECH";
    default:
      return "UNKNOWN_ERROR_TYPE";
  }
}
