/*---------------------------------------------------------------------------*
 *  EmbeddedRecognizerTest.java                                              *
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

package android.speech.recognition.test;

import android.speech.recognition.RecognitionResult;
import java.util.Hashtable;
import java.util.concurrent.Callable;
import android.speech.recognition.AbstractEmbeddedGrammarListener;
import android.speech.recognition.AbstractRecognizerListener;
import android.speech.recognition.AudioSource;
import android.speech.recognition.AudioStream;
import android.speech.recognition.Codec;
import android.speech.recognition.EmbeddedRecognizer;
import android.speech.recognition.GrammarListener;
import android.speech.recognition.MediaFileReader;
import android.speech.recognition.MediaFileReaderListener;
import android.speech.recognition.MediaFileWriter;
import android.speech.recognition.MediaFileWriterListener;
import android.speech.recognition.NBestRecognitionResult;
import android.speech.recognition.NBestRecognitionResult.Entry;
import android.speech.recognition.SrecGrammar;
import android.speech.recognition.WordItem;
import android.speech.recognition.Grammar;
import java.util.Enumeration;

/**
 * Tests the embedded recognizer functionality.
 */
public class EmbeddedRecognizerTest implements Callable<Void>
{
  private static final String ESRSDK = (System.getenv("ESRSDK") != null) ? System.getenv("ESRSDK") : "/system/usr/srec";
  private static final String QSDK = (System.getenv("QSDK") != null) ? System.getenv("QSDK") : "/system/usr/srec";
  private static final String populatedGrammar = "grammar.g2g";
  private SrecGrammar grammar;
  private final JUnitListener junitListener;

  private Object waitForTestToFinish;
  private boolean testFinished = false;

  public static void main(String[] args)
  {
    EmbeddedRecognizerTest test =
      new EmbeddedRecognizerTest(new EmptyJUnitListener());
    try
    {
      test.call();
    }
    catch (Exception e)
    {
      e.printStackTrace();
    }
  }

  /**
   * Creates a new EmbeddedRecognizerTest with JUnit integration.
   *
   * @param junitListener listens for test events
   */
  public EmbeddedRecognizerTest(JUnitListener junitListener)
  {
    this.junitListener = junitListener;
    waitForTestToFinish = new Object();
  }

  public Void call() throws Exception
  {
    System.out.println("Create an EmbeddedRecognizer");
    EmbeddedRecognizer recognizer = EmbeddedRecognizer.getInstance();
    String recognizerConfiguration = ESRSDK + "/config/en.us/baseline11k.par";
    recognizer.configure(recognizerConfiguration);
    recognizer.setListener(new RecognitionListener1());

    System.out.println("Load a grammar");
    String grammarPath = ESRSDK + "/config/en.us/grammars/bothtags5.g2g";
    grammar =
      (SrecGrammar) recognizer.createGrammar(grammarPath, new GrammarListener1());
    grammar.load();

    System.out.println("Add word to grammar slot");
    String slotName = "@Names";
    WordItem word = WordItem.valueOf("Jen_Parker", "jen&p)rkP");
    int weight = 0;
    String semanticValue = "V='Jen_Parker'";
    grammar.addItem(slotName, word, weight, semanticValue);
    grammar.compileAllSlots();

    System.out.println("Save the grammar");
    grammar.save(populatedGrammar);

    synchronized (waitForTestToFinish)
    {
      waitForTestToFinish.wait(20000);
      if(testFinished)
        System.out.println("----- PASS -----");
      else
        System.out.println("----- FAIL -----");
    }
    //TODO remove this once android fixed the shutdownHook for our automatic System.dispose
    android.speech.recognition.impl.System.getInstance().dispose();
    
    // Next event should be GrammarListener1.onSave()
    return null;
  }

  private class GrammarListener1 extends AbstractEmbeddedGrammarListener
  {
    @Override
    public void onSaved(String path)
    {
      System.out.println("Grammar saved");
      String audioPath = ESRSDK + "/config/en.us/audio/v139/v139_113.nwv";
      int headerLength = 1024;
      MediaFileReader mediaFileReader =
        MediaFileReader.create(audioPath, new ReaderListener());

      System.out.println("Recognizing against the first grammar (dynamic " + "add-word)");
      AudioStream audio = mediaFileReader.createAudio();
      logRecognizerAudio(mediaFileReader, QSDK + "/EmbeddedRecognizerTest1.raw");

      mediaFileReader.start();
      EmbeddedRecognizer.getInstance().recognize(audio, grammar);
      // Next event should be RecognitionListener1.onStopped()
    }

    @Override
    public void onError(Exception e)
    {
      junitListener.onException(e);
    }
  }

  private class RecognitionListener1 extends AbstractRecognizerListener
  {
    @Override
    public void onRecognitionFailure(FailureReason reason)
    {
      junitListener.onRecognitionFailure(reason);
    }

    @Override
    public void onError(Exception e)
    {
      junitListener.onException(e);
    }

    @Override
    public void onRecognitionSuccess(RecognitionResult result)
    {
      if( result instanceof NBestRecognitionResult )
      {
          Entry entry = ((NBestRecognitionResult)result).getEntry(0);
          if (entry != null)
            System.out.println("Meaning1=" + entry.getSemanticMeaning());
          else
            System.out.println("Meaning1=null");
      }
    }

    @Override
    public void onStopped()
    {
      System.out.println("RecognizerListener1.onStopped()");
      grammar.dispose();

      EmbeddedRecognizer recognizer = EmbeddedRecognizer.getInstance();
      recognizer.setListener(new RecognizerListener2());
      grammar =
        (SrecGrammar) recognizer.createGrammar(populatedGrammar, new GrammarListener2());
      System.out.println("Load the second grammar (pre-populated slot)");
      grammar.load();
      // Next event should be GrammarListener2.onLoaded()
    }
  }

/**
   * Now try recognizing against a grammar with pre-populated slots.
   */
  private class GrammarListener2 extends AbstractEmbeddedGrammarListener
  {
    @Override
    public void onLoaded()
    {
      System.out.println("Grammar loaded");
      String audioPath = ESRSDK + "/config/en.us/audio/v139/v139_113.nwv";
      int headerLength = 1024;
      MediaFileReader mediaFileReader =
        MediaFileReader.create(audioPath, new ReaderListener());

      System.out.println("Recognizing against the pre-populated grammar");
      AudioStream audio = mediaFileReader.createAudio();
      logRecognizerAudio(mediaFileReader, QSDK + "/EmbeddedRecognizerTest2.raw");
      mediaFileReader.start();
      EmbeddedRecognizer.getInstance().recognize(audio, grammar);
      // Next event should be RecognitionListener2.onStopped()
    }

    @Override
    public void onError(Exception e)
    {
      junitListener.onException(e);
    }
  }

  private class RecognizerListener2 extends AbstractRecognizerListener
  {
    @Override
    public void onRecognitionFailure(FailureReason reason)
    {
      junitListener.onRecognitionFailure(reason);
    }

    @Override
    public void onError(Exception e)
    {
      junitListener.onException(e);
    }

    @Override
    public void onRecognitionSuccess(RecognitionResult result)
    {
      if( result instanceof NBestRecognitionResult ) {
          Entry entry = ((NBestRecognitionResult)result).getEntry(0);
          if (entry != null)
          {
            System.out.println("Meaning=" + entry.getSemanticMeaning());
            System.out.println("Literal=" + entry.getLiteralMeaning());
            System.out.println("Score=" + entry.getConfidenceScore());
            Enumeration keys = entry.keys();
            while( keys.hasMoreElements())
            {
                String key = (String)keys.nextElement();
                String value = entry.get(key);
                System.out.println("\tkey=" + key + " value=" + value);
            }
          }
          else
            System.out.println("Meaning2=null");
      }
      testFinished = true;
    }

    @Override
    public void onStopped()
    {
      grammar.dispose();
      junitListener.afterTest();

      synchronized (waitForTestToFinish)
      {
        //tell the main thread that it can now exit. we are done running that test.
        waitForTestToFinish.notify();
      }
    }
  }

  private void logRecognizerAudio(AudioSource audioSource, String path)
  {
    AudioStream recognizerAudio = audioSource.createAudio();
    MediaFileWriter mediaFileWriter =
      MediaFileWriter.create(new WriterListener());
    mediaFileWriter.save(recognizerAudio, path);
  }

  private class ReaderListener implements MediaFileReaderListener
  {
    public void onError(Exception e)
    {
      junitListener.onException(e);
    }

    public void onStarted()
    {
    }

    public void onStopped()
    {
    }
  }

  private class WriterListener implements MediaFileWriterListener
  {
    public void onError(Exception e)
    {
      junitListener.onException(e);
    }

    public void onStopped()
    {
    }
  }
}
