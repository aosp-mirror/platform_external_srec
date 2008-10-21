/*---------------------------------------------------------------------------*
 *  Robustness1.java                                                         *
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

public class Robustness1 implements ICLGrammarAction, ICLRecognitionAction
{
    private static final String ESRSDK = (System.getenv("ESRSDK") != null) ? System.getenv("ESRSDK") : "/system/usr/srec";
    CLRecognizer recognizer = null;
    boolean bRecognitionEnded = false;

    //iteration Number
    int iN = 0;

    public static void main(String[] args) throws Exception
    {
      System.out.println("Robustness1: ESRSDK = " + ESRSDK);

      new Robustness1();
    }


    public void startNextRecognition()
    {
      String audioPath = ESRSDK + "/config/en.us/audio/v139/v139_113.nwv";
      iN = iN + 1;

      System.out.println("Iteration: " + iN);
      System.out.println("Recognizing with: " + audioPath);

      try {
        // Recognize from file
        recognizer.recognize(this, audioPath);
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
      startNextRecognition();
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
      startNextRecognition();
    }
  
    public void onAudioSourceStopped()
    {
      System.out.println("Audio source stopped");
    }
      

    //cancelled by user
    public void onRecognitionCancelled(ICLRecognizer r)
    {
      System.out.println("Recognition Result: CANCELLED");
    }


    public Robustness1() throws Exception
    {
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

        String[] names = new String[3];
        names[0] = "Andy Wyatt";
        names[1] = "Dennis Velasco";
        names[2] = "Jen Parker";

        recognizer.createGrammar(path, names, this);
      } catch (Exception e) {
        System.out.println("Exception from ICLRecognizer.createGrammar(): " + e.toString());
      }

      while(true)
      {
        try {
          Thread.sleep(100);
        }
        catch(InterruptedException e)
        {
          System.out.println("Exception from Thread.sleep: " + e.toString());
          throw e;
        }
      }
    }
}

