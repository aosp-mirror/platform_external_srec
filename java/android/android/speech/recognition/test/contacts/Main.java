/*---------------------------------------------------------------------------*
 *  Main.java                                                                *
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
//import android.util.Log;


/**
 * Tests the embedded recognizer functionality.
 */
public class Main implements ICLGrammarAction, ICLRecognitionAction
{
  private static final String ESRSDK = (System.getenv("ESRSDK") != null) ? System.getenv("ESRSDK") : "/system/usr/srec";
  CLRecognizer recognizer = null;
  boolean bRecognitionEnded = false;

  
  private Object waitForTestToFinish;
  private boolean testFinished = false;
  
  
  public static void main(String[] args) throws Exception
  {
    System.out.println("SpeechTest3Console");

    new Main();
  }
  //ICLGRammarAction interface

  //error (exception)
  public void onGrammarError(ICLRecognizer r, Exception e)
  {
    System.out.println("ICLGrammarAction:onGrammarError: "  + e.toString());
    onRecognitionEnded();
  }

  //success
  public void onGrammarSuccess(ICLRecognizer r)
  {
    System.out.println("GrammarAction:onGrammarSuccess");

    //kick off the recognition
    String audioPath = ESRSDK + "/config/en.us/audio/v139/v139_113.nwv";

    System.out.println("Recognizing against the first grammar (dynamic "  +
      "add-word)");

    try
    {
      r.recognize(this, audioPath);
    }
    catch (Exception e)
    {
      System.out.println("Exception from ICLRecognizer.recognize(): "  +
        e.toString());
      onRecognitionEnded();
    }
  }

  //error (exception)
  public void onRecognitionError(ICLRecognizer r, Exception e)
  {
    System.out.println("Recognition Result: ERROR: "  + e.toString());
    onRecognitionEnded();
  }

  //failure - did not recognize anything
  public void onRecognitionFailure(ICLRecognizer r, String reason)
  {
    System.out.println("Recognition Result: FAILURE: "  + reason);

    onRecognitionEnded();
  }

  //success - have NBest list
  public void onRecognitionSuccess(ICLRecognizer r, String[] result)
  {

    int numResults = result.length;

    System.out.println("Recognition Result: SUCCESS");

    System.out.println("num results: "  + numResults);


    for (int i = 0; i < numResults; i++)
    {
      System.out.println("result "  + (i + 1) + ":" + result[i]);
    }
   
    
    onRecognitionEnded();
  }

  public void onFileWritten()
  {
    testFinished = true;
    synchronized (waitForTestToFinish)
    {
        //tell the main thread that it can now exit. we are done running that test.
        waitForTestToFinish.notify();
    }   
  }
  
    public void onAudioSourceStopped()
    {
      System.out.println("Audio source stopped");
    }
    
  //cancelled by user
  public void onRecognitionCancelled(ICLRecognizer r)
  {
    System.out.println("Recognition Result: CANCELLED");

    onRecognitionEnded();
  }

  public void onRecognitionEnded()
  {

    //synchronized (this) {
    //    System.out.println("onRecognitionEnded()");
    //    bRecognitionEnded = true;
    //    notifyAll();
    //}
  }

  public Main() throws Exception
  {
    waitForTestToFinish = new Object();
    
    System.out.println("Initialising...");

    try
    {
      recognizer = new CLRecognizer();
    }
    catch (Exception e)
    {
      System.out.println("Exception from ICLRecognizer(): "  + e.toString());
      System.exit(1);
    }


    try
    {
      String path = ESRSDK + "/config/en.us/baseline11k.par";
      recognizer.allocate(path);
    }
    catch (Exception e)
    {
      System.out.println("Exception from ICLRecognizer.allocate(): "  +
        e.toString());
      System.exit(1);
    }


    try
    {
      String path = ESRSDK + "/config/en.us/grammars/dynamic-test.g2g";

      String[] names = new String[3];
      names[0] = "Andy Wyatt";
      names[1] = "Dennis Velasco";
      names[2] = "Jen Parker";

      recognizer.createGrammar(path, names, this);
    }
    catch (Exception e)
    {
      System.out.println("Exception from ICLRecognizer.createGrammar(): "  +
        e.toString());
    }

    synchronized (waitForTestToFinish)
    {
        waitForTestToFinish.wait(10000);  
        if(testFinished)
            System.out.println("----- PASS -----");
        else
            System.out.println("----- FAIL -----");
    }
    //TODO remove this once android fixed the shutdownHook for our automatic System.dispose
    android.speech.recognition.impl.System.getInstance().dispose();
  }
}
