/*---------------------------------------------------------------------------*
 *  Robustness2.java                                                         *
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


package android.speech.recognition.test.robustness1;

import android.speech.recognition.test.contacts.CLRecognizer;
import android.speech.recognition.test.contacts.ICLRecognizer;
import android.speech.recognition.test.contacts.ICLGrammarAction;
import android.speech.recognition.test.contacts.ICLRecognitionAction;

import java.util.Hashtable;

import android.speech.recognition.AbstractGrammarListener;
import android.speech.recognition.AbstractRecognizerListener;
import android.speech.recognition.AudioStream;
import android.speech.recognition.Codec;
import android.speech.recognition.EmbeddedRecognizer;
import android.speech.recognition.Grammar;
import android.speech.recognition.GrammarListener;
import android.speech.recognition.MediaFileReader;
import android.speech.recognition.MediaFileReaderListener;
import android.speech.recognition.RecognizerListener.FailureReason;
import android.speech.recognition.SrecGrammar;
import android.speech.recognition.WordItem;
import android.speech.recognition.RecognizerListener;
import android.speech.recognition.NBestRecognitionResult;

/**
 * Tests the embedded recognizer functionality.
 */

public class Robustness2 implements ICLGrammarAction, ICLRecognitionAction
{
    private static final String ESRSDK = (System.getenv("ESRSDK") != null) ? System.getenv("ESRSDK") : "/system/usr/srec";
    CLRecognizer recognizer = null;
    boolean bRecognitionEnded = false;
    private String[] names;

    private Object waitForTestToFinish;
    private boolean testFinished = false;

    //iteration Number
    int iN = 0;

    public static void main(String[] args) throws Exception
    {
      System.out.println("Robustness2: ESRSDK = " + ESRSDK);

      new Robustness2();
    }


    public void startNextRecognition()
    {
      if (iN==50)
      {
        synchronized (waitForTestToFinish)
        {
            testFinished = true;
            //tell the main thread that it can now exit. we are done running that test.
            waitForTestToFinish.notify();
        }
        return;
      }

      System.out.println("");
      System.out.println("-----------------------------------");
      System.out.println("Iteration: " + iN);
      System.out.println("Recording from microphone...");
      System.out.println("SAY: LOOKUP " + names[iN%3]);
      System.out.println("");
      System.out.println("-----------------------------------");
      iN++;

      try {
        // Recognize using microphone
        recognizer.recognize(this, null);
      } catch (Exception e) {
        System.out.println("Exception from CLRecognizer.recognize(): " + e.toString());
        System.exit(1);
      }
    }

    //ICLGRammarAction interface

    //error (exception)
    public void onGrammarError(ICLRecognizer r, Exception e)
    {
      System.out.println("ICLGrammarAction:onGrammarError: " + e.toString());
      System.exit(1);
    }

    //success
    public void onGrammarSuccess(ICLRecognizer r)
    {
      System.out.println("GrammarAction:onGrammarSuccess");

      startNextRecognition();
    }


    //error (exception)
    public void onRecognitionError(ICLRecognizer r, Exception e)
    {
      System.out.println("Recognition Result: ERROR: " + e.toString());
      System.exit(1);
    }

    //failure - did not recognize anything
    public void onRecognitionFailure(ICLRecognizer r, String reason)
    {
      System.out.println("Recognition Result: FAILURE: " + reason);
    }


    //success - have NBest list
    public void onRecognitionSuccess(ICLRecognizer r, String[] result)
    {
      int numResults = result.length;

      System.out.println("Recognition Result: SUCCESS");
      System.out.println("num results: " + numResults);

      for (int i = 0; i < numResults; i++) {
        System.out.println("result " + (i + 1) + ":" + result[i]);
      }
    }

     public void onFileWritten()
    {
      System.out.println("File was saved");
    }
  
    public void onAudioSourceStopped()
    {
      System.out.println("Audio source stopped");
      try {
        System.out.println("Sleep 1800 ms (workaround for Sooner audio driver bug)");
        Thread.sleep(1800);  // workaround for bug in closing + restarting Android Sooner audio driver.
        System.out.println("Woke up");
      } catch (Exception e) {
        System.out.println("Exception from Thread.sleep(): " + e.toString());
      }
      startNextRecognition();
    }
      
    //cancelled by user
    public void onRecognitionCancelled(ICLRecognizer r)
    {
      System.out.println("Recognition Result: CANCELLED");

      startNextRecognition();
    }


    public Robustness2() throws Exception
    {
      waitForTestToFinish = new Object();
      System.out.println("Initialising...");

      try {
        recognizer = new CLRecognizer();
      } catch (Exception e) {
        System.out.println("Exception from ICLRecognizer(): " + e.toString());
        System.exit(1);
      }

      try {
        String path = ESRSDK + "/config/en.us/baseline11k.par";
        recognizer.allocate(path);
      } catch (Exception e) {
        System.out.println("Exception from ICLRecognizer.allocate(): " + e.toString());
        System.exit(1);
      }

      try {
        String path = ESRSDK + "/config/en.us/grammars/dynamic-test.g2g";

        names = new String[3];
        names[0] = "Andy Wyatt";
        names[1] = "Dennis Velasco";
        names[2] = "Jen Parker";

        recognizer.createGrammar(path, names, this);
      } catch (Exception e) {
        System.out.println("Exception from ICLRecognizer.createGrammar(): " + e.toString());
      }

      //wait for the test to finish before we return.
      //we have to make sure the Java thread stays alive.
      synchronized (waitForTestToFinish)
      {
        waitForTestToFinish.wait();
        if(testFinished)
          System.out.println("----- PASS -----");
        else
          System.out.println("----- FAIL -----");
      }
      //TODO remove this once android fixed the shutdownHook for our automatic System.dispose
      android.speech.recognition.impl.System.getInstance().dispose();
    }
}

