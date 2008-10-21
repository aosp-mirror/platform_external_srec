/*---------------------------------------------------------------------------*
 *  CLRecognizer.java                                                        *
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

import android.speech.recognition.AbstractEmbeddedGrammarListener;
import android.speech.recognition.AbstractRecognizerListener;
import android.speech.recognition.AudioSource;
import android.speech.recognition.AudioStream;
import android.speech.recognition.Codec;
import android.speech.recognition.EmbeddedRecognizer;
import android.speech.recognition.Grammar;
import android.speech.recognition.GrammarListener;
import android.speech.recognition.MediaFileReader;
import android.speech.recognition.MediaFileReaderListener;
import android.speech.recognition.MediaFileWriter;
import android.speech.recognition.MediaFileWriterListener;
import android.speech.recognition.Microphone;
import android.speech.recognition.MicrophoneListener;
import android.speech.recognition.NBestRecognitionResult;
import android.speech.recognition.RecognitionResult;
import android.speech.recognition.RecognizerListener;
import android.speech.recognition.SrecGrammar;
import android.speech.recognition.WordItem;
import java.util.Hashtable;

//import android.util.Log;
//CL stands for Contact List
public class CLRecognizer extends AbstractRecognizerListener implements ICLRecognizer
{
  private static final String QSDK = (System.getenv("QSDK") != null) ? System.getenv("QSDK") : "/system/usr/srec";
  private EmbeddedRecognizer recognizer;
  //a single SrecGrammar
  private SrecGrammar grammar;
  private java.lang.String[] dynamicItems;
  private ICLGrammarAction grammarAction;
  private ICLRecognitionAction recognitionAction;
  //recognition results
  private String[] recognitionResults;
  private String recognitionFailureReason;
  private Exception recognitionException;
  private boolean recognitionCancelled = false;
  private AudioSourceListener audioSourceListener;
  private AudioStream audioStream;
  private AudioSource audioSource;
  private MediaFileWriter mediaFileWriter;
  private AudioStream audioToSave;
  private boolean isMicrophone;
  
  public CLRecognizer()
  {
    //interesting stuff is done in allocate()
    recognizer = null;
    isMicrophone = false;
  }

  //synchronous
  public void allocate(java.lang.String config) throws Exception
  {
    logDebug("before EmbeddedRecognizer.getInstance()");

    recognizer = EmbeddedRecognizer.getInstance();

    logDebug("after EmbeddedRecognizer.getInstance(): "  +
      recognizer.getClass().getName() + " "  + recognizer.toString());


    logDebug("before EmbeddedRecognizer.configure("  + config + ")");
    try
    {
      recognizer.configure(config);
    }
    catch (Exception e)
    {
      logDebug("exception from EmbeddedRecognizer.configure()");
      throw e;
    }
    logDebug("after EmbeddedRecognizer.configure()");
  }

  //asynchronous call
  public void createGrammar(java.lang.String grammarURI,
    java.lang.String[] dynamicItems, ICLGrammarAction grammarAction) throws Exception
  {

    logDebug("before recognizer.createGrammar("  + grammarURI + ")");

    this.dynamicItems = dynamicItems;
    this.grammarAction = grammarAction;
    grammar =
      (SrecGrammar) recognizer.createGrammar(grammarURI, 
					   new CLGrammarListener());
    logDebug("after recognizer.createGrammar("  + grammarURI + ")");


    logDebug("before grammar.load()");
    grammar.load();
    logDebug("after grammar.load()");
  }

  //asynchronous call
  public void recognize(ICLRecognitionAction action, java.lang.String audioFile)
    throws Exception
  {
    logDebug("+recognize()");

    //XXX if action is null throw our own exception
    //save for future use
    recognitionAction = action;

    //clear results from previous recognition
    recognitionResults = null;
    recognitionFailureReason = null;
    recognitionException = null;

    //cleanup other objects from previous recognition
    //just in case they remebered some state and we would need to get out of here
    //on exception
    audioSourceListener = null;
    audioSource = null;
    audioStream = null;

    audioSourceListener = new AudioSourceListener();

    if (audioFile != null)
    {
      audioSource =
        MediaFileReader.create(audioFile, audioSourceListener);

      //make MediaFileReader file look more like microphone
      //((MediaFileReader) audioSource).setMode(MediaFileReader.Mode.REAL_TIME);
      isMicrophone = false;
    }
    else
    {
      //use Microphone
      audioSource = Microphone.getInstance();
      ((Microphone) audioSource).setListener(audioSourceListener);
      ((Microphone) audioSource).setCodec(Codec.PCM_16BIT_11K);
      isMicrophone = true;
    }


    //create AudioStream to recognize from
    audioStream = audioSource.createAudio();
    //create AudioStream to save the audio
    audioToSave = audioSource.createAudio();

    //start capturing audio
    audioSource.start();
    
    recognizer.setListener(this);

    recognizer.recognize(audioStream, grammar);

    logDebug("-recognize()");
  }

  //
  public void cancel() throws Exception
  {
  }

  //
  public void deallocate() throws Exception
  {
  }

  private class writerListener implements MediaFileWriterListener
  {
    public void onError(Exception e)
    {
       CLRecognizer.logDebug("writerListener: onError(): "  + e.toString());
    }

    public void onStopped()
    {
      System.out.println("writerListener: onStopped()");
      recognitionAction.onFileWritten();
    }
  }
  
  private class AudioSourceListener implements MediaFileReaderListener,
    MicrophoneListener
  {
    public AudioSourceListener()
    {
    }
    public void onError(Exception e)
    {
      CLRecognizer.logDebug("AudioSource: onError(): "  + e.toString());
      //XXX do something smart here
    }

    public void onStarted()
    {
      CLRecognizer.logDebug("AudioSource: onStarted()");
    }

    public void onStopped()
    {
      CLRecognizer.logDebug("AudioSource: onStopped()");
      recognitionAction.onAudioSourceStopped();
    }
  }

  private class CLGrammarListener extends AbstractEmbeddedGrammarListener
    implements GrammarListener
  {
    @Override
    public void onCompileAllSlots()
    {
      logDebug("onCompileAllSlots");
      CLRecognizer.this.grammarAction.onGrammarSuccess(CLRecognizer.this);
    }

    @Override
    public void onError(Exception e)
    {
      logDebug("GrammarListener:onError: "  + e.toString());
      CLRecognizer.this.grammarAction.onGrammarError(CLRecognizer.this, e);
    }

    @Override
    public void onLoaded()
    {
      try
      {
        logDebug("onLoaded");
        //add words here
        logDebug("Adding words to grammar slot");
        if (CLRecognizer.this.dynamicItems != null)
        {
          for (int i = 0; i < CLRecognizer.this.dynamicItems.length; i++)
          {
            logDebug("item "  + i + ": "  + CLRecognizer.this.dynamicItems[i]);
            String slotName = "@Names";
            WordItem word =
              WordItem.valueOf(CLRecognizer.this.dynamicItems[i], (String) null);
            int weight = 1;
            String semanticValue =
              "V='"  + CLRecognizer.this.dynamicItems[i] + "'";
            grammar.addItem(slotName, word, weight, semanticValue);
          }
        }

        grammar.compileAllSlots();
      }
      catch (Exception e)
      {
        //this error is asynchronous to the user
        CLRecognizer.this.grammarAction.onGrammarError(CLRecognizer.this, e);
      }
    }

    @Override
    public void onResetAllSlots()
    {
      logDebug("onResetAllSlots");
    }

    @Override
    public void onSaved(String path)
    {
      logDebug("onSaved");
    }

    @Override
    public void onUnloaded()
    {
      logDebug("onUnloaded");
    }
  }

  //RecognizerListener interface
  //
  @Override
  public void onBeginningOfSpeech()
  {
    logDebug("onBeginningOfSpeech");
  }

  @Override
  public void onEndOfSpeech()
  {
    logDebug("onEndOfSpeech");
  }

  @Override
  public void onRecognitionFailure(RecognizerListener.FailureReason reason)
  {
    logDebug("onExpectedError");
    //save the result for future use
    recognitionFailureReason = new String(reason.toString());
  }

  @Override
  public void onRecognitionSuccess(RecognitionResult result)
  {
        if( result instanceof NBestRecognitionResult )
        {
            int numResults = ((NBestRecognitionResult)result).getSize();
            logDebug("onRecognitionResult: "  + numResults);

            recognitionResults = new String[numResults];
            for (int i = 0; i < numResults; i++)
            {
              NBestRecognitionResult.Entry entry = ((NBestRecognitionResult)result).getEntry(i);
              if (entry!=null)
              {
                    String txt = "result "  + (i + 1);
                    if (entry.getSemanticMeaning()!=null)
                    {
                        String semanticMeaning = entry.getSemanticMeaning();
                        recognitionResults[i] = new String(semanticMeaning);
                        txt = txt + " semantic: "+ entry.getSemanticMeaning();
                    }
                    if (entry.getLiteralMeaning()!=null)
                        txt = txt + " literal:"+entry.getLiteralMeaning();
                    txt = txt + " score:"+entry.getConfidenceScore();
                    logDebug(txt);
                    txt = null;
              }
              else 
                  recognitionResults[i] = "";
            }
        }
  }

  @Override
  public void onStarted()
  {
    logDebug("onStarted");
  }

  @Override
  public void onStartOfSpeechTimeout()
  {
    logDebug("onStartOfSpeechTimeout");
  }

  @Override
  public void onStopped()
  {
    logDebug("+onStopped()");
   
    //cleanup AudioSource/AudioStream
    if (audioSource != null) {
        audioSource.stop();
    }

     // Display results
     // call only one of ICLRecognitionAction methods to mark the end of recognition
        if (recognitionException != null)
        {
          recognitionAction.onRecognitionError(this, recognitionException);
        }
        else if (recognitionCancelled == true)
        {
          recognitionAction.onRecognitionCancelled(this);
        }
        else if (recognitionFailureReason != null)
        {
          recognitionAction.onRecognitionFailure(this, recognitionFailureReason);
          return;
        }
        else if (recognitionResults != null)
        {
          recognitionAction.onRecognitionSuccess(this, recognitionResults);
        }
        else
        {
          //we should not be able to get here
          //
          recognitionException =
            new Exception("recognition stopped for unknown reason");
          recognitionAction.onRecognitionError(this, recognitionException);
        }
    
     // Save sound file
    String savePath = QSDK+ "/contactsTestUsedSound.raw";
    mediaFileWriter = MediaFileWriter.create(new writerListener());
    mediaFileWriter.save(audioToSave, savePath);
    
    logDebug("-onStopped()");
  }

  @Override
  public void onError(java.lang.Exception e)
  {
    logDebug("+onError");
    recognitionException = e;
    //cleanup AudioSource/AudioStream
    if (audioSource != null) {
        audioSource.stop();
    }
    recognitionAction.onRecognitionError(this, recognitionException);

    logDebug("-onUnexpectedError");
  }

  static void logDebug(String msg)
  {
    System.out.println(CLRecognizer.class.getName() + ":"  + msg);
    //Log.d(CLRecognizer.class.getName(), msg);
  }
}
