/*---------------------------------------------------------------------------*
 *  ICLRecognizer.java                                                       *
 *                                                                           *
 *  Copyright 2007, 2008 Nuance Communciations, Inc.                         *
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

package android.speech.recognition.test.contacts;

//package com.android.speechtest;
public interface ICLRecognizer
{
  //
  public void allocate(java.lang.String config) throws Exception;

  //asynchronous call
  public void createGrammar(java.lang.String grammarURI,
    java.lang.String[] dynamicItems, ICLGrammarAction grammarAction) throws Exception;

  //asynchronous call
  //if audioFile provided - use it, otherwise use Microphone
  public void recognize(ICLRecognitionAction action, java.lang.String audioFile)
    throws Exception;

  //
  public void cancel() throws Exception;

  //
  public void deallocate() throws Exception;
}
