/*---------------------------------------------------------------------------*
 *  jniapi.cpp                                                               *
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

// Jniapi.cpp : Defines the entry point for the DLL application.
//
#include <jni.h>

#ifdef UAPI_WIN32
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>

BOOL APIENTRY DllMain(HANDLE, DWORD, LPVOID)
{
  return TRUE;
}
#endif

jint JNICALL JNI_OnLoad(JavaVM*, void*)
{
  return JNI_VERSION_1_4;
}
