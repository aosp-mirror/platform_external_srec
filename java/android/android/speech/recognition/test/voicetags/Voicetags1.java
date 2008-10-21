/*---------------------------------------------------------------------------*
 *  Voicetags1.java                                                          *
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


package android.speech.recognition.test.voicetags;

import android.speech.recognition.AbstractSrecGrammarListener;
import android.speech.recognition.AbstractRecognizerListener;
import android.speech.recognition.AudioStream;
import android.speech.recognition.Codec;
import android.speech.recognition.EmbeddedRecognizer;
import android.speech.recognition.Grammar;
import android.speech.recognition.RecognitionResult;
import android.speech.recognition.SrecGrammarListener;
import android.speech.recognition.GrammarListener;
import android.speech.recognition.MediaFileReader;
import android.speech.recognition.MediaFileReaderListener;
import android.speech.recognition.MediaFileWriter;
import android.speech.recognition.MediaFileWriterListener;
import android.speech.recognition.Microphone;
import android.speech.recognition.MicrophoneListener;
import android.speech.recognition.NBestRecognitionResult;
import android.speech.recognition.RecognizerListener;
import android.speech.recognition.SrecGrammar;
import android.speech.recognition.VoicetagItem;
import android.speech.recognition.VoicetagItemListener;
import android.speech.recognition.Logger;
import java.util.Hashtable;
import java.util.Vector;
import java.util.Enumeration;
import java.io.File;
/**
 */
public class Voicetags1 extends AbstractRecognizerListener {

    private static final String ESRSDK = (System.getenv("ESRSDK") != null) ? System.getenv("ESRSDK") : "/system/usr/srec";
    private static final String QSDK = (System.getenv("QSDK") != null) ? System.getenv("QSDK") : "/system/usr/srec";
    private static final int STATE_IDLE                           = 0;
    private static final int STATE_CREATE_RECOGNIZER              = 1;
    private static final int STATE_SET_PARAM_ON                   = 2;
    private static final int STATE_SETTING_PARAM_ON               = 3;
    private static final int STATE_CREATE_GRAMMAR1                = 4;
    private static final int STATE_LOAD_GRAMMAR1                  = 5;
    private static final int STATE_LOADING_GRAMMAR1               = 6;
    private static final int STATE_COMPILING_GRAMMAR1             = 7;
    private static final int STATE_MICROPHONE                     = 8;
    private static final int STATE_MICROPHONE_START               = 9;
    private static final int STATE_RECORD_RECOGNIZING             = 10;
    private static final int STATE_SET_PARAM_OFF                  = 11;
    private static final int STATE_SETTING_PARAM_OFF              = 12;
    private static final int STATE_UNLOAD_GRAMMAR1                = 13;
    private static final int STATE_UNLOADING_GRAMMAR1             = 14;
    private static final int STATE_CREATE_GRAMMAR2                = 15;
    private static final int STATE_LOAD_GRAMMAR2                  = 16;
    private static final int STATE_LOADING_GRAMMAR2               = 17;
    private static final int STATE_SHOW_ITERATION                 = 18;
    private static final int STATE_RESET_ALL_SLOTS_GRAMMAR2       = 19;
    private static final int STATE_RESETTING_SLOTS_GRAMMAR2       = 20;
    private static final int STATE_LOAD_VOICETAG_IDX              = 21;
    private static final int STATE_LOADING_VOICETAG_IDX           = 22;
    private static final int STATE_ADD_VOICETAGITEM_GRAMMAR2      = 23;
    private static final int STATE_ADDING_VOICETAGITEM            = 24;
    private static final int STATE_COMPILE_GRAMMAR2               = 25;
    private static final int STATE_COMPILING_GRAMMAR2             = 26;
    private static final int STATE_RECOGNIZE_WITH_GRAMMAR2        = 27;
    private static final int STATE_RECOGNIZING_WITH_GRAMMAR2      = 28;
    private static final int STATE_RESET                          = 29;
    private static final int STATE_QUIT                           = 30;
    private static final int STATE_ERROR                          = 31;
    private static final int STATE_RESET_ENROLLMENT               = 32;
    private static final int STATE_WAIT_SAVED_VOICETAG            = 33;
    private static final int STATE_SAVE_GRAMMAR2                  = 34;
    private static final int STATE_SAVING_GRAMMAR2                = 35;
    private static final int STATE_SAVE_AUDIOSOURCE               = 36;
    private static final int STATE_SAVING_AUDIOSOURCE             = 37;
 
    private String[] stateNames = { 
    "STATE_IDLE",
    "STATE_CREATE_RECOGNIZER",
    "STATE_SET_PARAM_ON",
    "STATE_SETTING_PARAM_ON",
    "STATE_CREATE_GRAMMAR1",
    "STATE_LOAD_GRAMMAR1",
    "STATE_LOADING_GRAMMAR1",
    "STATE_COMPILING_GRAMMAR1",
    "STATE_MICROPHONE",
    "STATE_MICROPHONE_START",
    "STATE_RECORD_RECOGNIZING",
    "STATE_SET_PARAM_OFF",
    "STATE_SETTING_PARAM_OFF",
    "STATE_UNLOAD_GRAMMAR1",
    "STATE_UNLOADING_GRAMMAR1",
    "STATE_CREATE_GRAMMAR2",
    "STATE_LOAD_GRAMMAR2",
    "STATE_LOADING_GRAMMAR2",
    "STATE_SHOW_ITERATION",
    "STATE_RESET_ALL_SLOTS_GRAMMAR2",
    "STATE_RESETTING_SLOTS_GRAMMAR2",
    "STATE_LOAD_VOICETAG_IDX",
    "STATE_LOADING_VOICETAG_IDX",
    "STATE_ADD_VOICETAGITEM_GRAMMAR2",
    "STATE_ADDING_VOICETAGITEM",
    "STATE_COMPILE_GRAMMAR2",
    "STATE_COMPILING_GRAMMAR2",
    "STATE_RECOGNIZE_WITH_GRAMMAR2",
    "STATE_RECOGNIZING_WITH_GRAMMAR2",
    "STATE_RESET",
    "STATE_QUIT",
    "STATE_ERROR",
    "STATE_RESET_ENROLLMENT",
    "STATE_WAIT_SAVED_VOICETAG",
    "STATE_SAVE_GRAMMAR2",
    "STATE_SAVING_GRAMMAR2",
    "STATE_SAVE_AUDIOSOURCE",
    "STATE_SAVING_AUDIOSOURCE"
    };
  
    private int iN = 0;
    private int itemIdx = 0;
    private int m_State = STATE_IDLE;
    private Object mutex;
    private Object micMutex;
    private boolean isRecording;
    private EmbeddedRecognizer recognizer;  
    private SrecGrammar grammar1;
    private SrecGrammar grammar2;
    private MFRListener audioSourceListener;
    private MFWListener mfwListener;
    private VTGListener voicetagListener;
    private MICListener micListener;
    private AudioStream audioStream;
    private AudioStream audioToSave;
    private MediaFileReader mfr;
    private MediaFileWriter mfw;
    private Microphone mic;
    private String[]    pathG;
    private String[]    names;
    private String[]    semanticValues;
    private VoicetagItem currentVoicetag;
    private VoicetagItem[] voicetags;
  
    public static void main(String[] args) throws Exception
    {
        System.out.println("Voicetags1: ESRSDK = " + ESRSDK);
        Logger.getInstance().info("Voicetags1");
        new Voicetags1();
    }
    
    /*-------------------------------------------------------
    * Voicetags1 Diagram Flow
    * -------------------------------------------------------
    * 1) Create recognizer
    * 2) Set recognizer parameter enableGetWaveform to true '1'
    * 3) Create Enrollment Grammar1 - Load - CompileAllSlots
    * 4) Create Microphone 
    * 5) Start Mic 
    * 6) Show name[itemIdx] to be enrolled, ask user to speak
    * 7) Record until, recognition (success or fail)
    * 8) Stop Mic, Create Voicetag
    * 9) Save Voicetag to file
    * 10) Goto (5) until all "itemIdx" enrollments are done.
    * 11) Set recognizer parameter enableGetWaveform to false '0' 
    * 12) Unload Grammar1
    * 13) Create Grammar2 - load grammar2
    * 14) Show iteration number
    * 15) Grammar2.ResetAllSlots
    * 16) Load Voicetag[itemIdx] - add VoicetagItem[itemIdx] - repeat until "itemIdx" are done.
    * 17) Compile Grammar2
    * 18) MFR:Load previously recorded Voicetag[itemIdx] sound.
    * 19) Recognize with Grammar2
    * 20) Goto  (18) until "itemIdx" are done.
    * 21) Infinite Loop, Goto (15)
    * -------------------------------------------------------*/
    public Voicetags1() throws Exception
    {
       Logger.getInstance().setLoggingLevel(Logger.LogLevel.LEVEL_WARN);
        // Store two grammar paths
        pathG = new String[2];
        // Enrollment Grammar
        pathG[0] = ESRSDK + "/config/en.us/grammars/enroll.g2g";
        // Grammar to be use to recognize
        pathG[1] = ESRSDK + "/config/en.us/grammars/bothtags5.g2g";
        
        names = new String[3];
        semanticValues = new String[3];
        names[0] = "Jen Parker";           semanticValues[0]="(514)555-8666";
        names[1] = "Pete Gonzalez";        semanticValues[1]="(613)555-5464";
        names[2] = "Andrew Evans";         semanticValues[2]="(800)555-1234";
        
        // Create object that will be used to synch the states
        mutex = new Object();
        // Create mic mutex
        micMutex = new Object();
        isRecording = false;
        // Change the state to create the recognizer
        setState(STATE_CREATE_RECOGNIZER);
        
        // Main thread loop
        while(true)
        {
            synchronized(mutex)
            {
                if ( m_State == STATE_QUIT) break; // Exit application
                
                switch (m_State)
                {
                    case STATE_ERROR:
                        //Report Error
                        logDebug("ERROR: Application stopped!");
                        Logger.getInstance().error("ERROR: Fatal error has been reached\nApplication will stop");
                        setStateLocked(STATE_QUIT);
                        break;
                    //For these state, we do nothing since we are waiting for
                    //the event to trigger.
                    case STATE_IDLE: 
                    case STATE_LOADING_GRAMMAR1: 
                    case STATE_COMPILING_GRAMMAR1: 
                    case STATE_RECORD_RECOGNIZING:
                    case STATE_RESETTING_SLOTS_GRAMMAR2:    
                    case STATE_UNLOADING_GRAMMAR1:
                    case STATE_LOADING_GRAMMAR2:
                    case STATE_RECOGNIZING_WITH_GRAMMAR2:
                    case STATE_LOADING_VOICETAG_IDX:
                    case STATE_ADDING_VOICETAGITEM:
                    case STATE_SETTING_PARAM_ON:
                    case STATE_SETTING_PARAM_OFF:
                    case STATE_COMPILING_GRAMMAR2:
                    case STATE_WAIT_SAVED_VOICETAG:
                    case STATE_SAVING_GRAMMAR2:
                    case STATE_SAVING_AUDIOSOURCE:
                        try {
                          //Thread.currentThread().sleep(5);
                          mutex.wait();
                        }
                        catch(InterruptedException e)
                        {
                          System.out.println("Exception from Thread.sleep: " + e.toString());
                          throw e;
                        }
                        break;
                        
                    case STATE_CREATE_RECOGNIZER: 
                        initApp();
                        setStateLocked(STATE_SET_PARAM_ON);
                        break;
                        
                    case STATE_SET_PARAM_ON:
                        setStateLocked(STATE_SETTING_PARAM_ON);                        
                        setParams(true);
                        break;
                        
                    case STATE_CREATE_GRAMMAR1:
                        grammar1 = createGrammar(pathG[0],new G1Listener());
                        setStateLocked(STATE_LOAD_GRAMMAR1);
                        break;
                        
                    case STATE_LOAD_GRAMMAR1:
                        setStateLocked(STATE_LOADING_GRAMMAR1);
                        grammar1.load(); //wait for G1Listener.onLoaded()
                        break;
                        
                    case STATE_RESET_ENROLLMENT:
                        synchronized(micMutex)
                        {
                           // Wait until record has been finished
                           if (isRecording) break; 
                        }
                        // Continue with next enrollment
                        audioStream = null;
                        audioToSave = null;
                        setStateLocked(STATE_MICROPHONE);
                        break;
                        
                    case STATE_MICROPHONE:
                        audioStream = mic.createAudio();
                        audioToSave = mic.createAudio();
                        setStateLocked(STATE_MICROPHONE_START);
                        break;
                        
                    case STATE_MICROPHONE_START:
                        startRecordRecognize();
                        setStateLocked(STATE_RECORD_RECOGNIZING);
                        break;
                            
                    case  STATE_SET_PARAM_OFF:
                        setStateLocked(STATE_SETTING_PARAM_OFF);                        
                        setParams(false);
                        break;
                       
                    case STATE_UNLOAD_GRAMMAR1:
                        mfwListener = null;
                        mfw = null; // We don't need it anymore.
                        setStateLocked(STATE_UNLOADING_GRAMMAR1);
                        grammar1.unload();//wait for G1Listener.onUnloaded()
                        break;
                        
                    case STATE_CREATE_GRAMMAR2:
                        grammar2 = createGrammar(pathG[1],new G2Listener());
                        setStateLocked(STATE_LOAD_GRAMMAR2);
                        break;
                     
                    case STATE_LOAD_GRAMMAR2:
                        setStateLocked(STATE_LOADING_GRAMMAR2);
                        grammar2.load();//wait for G2Listener.onLoaded()
                        break;        
                        
                    case STATE_SAVE_AUDIOSOURCE:
                        break;
                    
                    case STATE_SHOW_ITERATION:
                        iN++;
                        logDebug("---------------------------------------");
                        logDebug("Iteration:"+iN);
                        logDebug("---------------------------------------");                               
                        setStateLocked(STATE_RESET_ALL_SLOTS_GRAMMAR2);
                        break;
                  
                    case STATE_RESET_ALL_SLOTS_GRAMMAR2:
                        setStateLocked(STATE_RESETTING_SLOTS_GRAMMAR2);
                        int sz = names.length;
                        voicetags = new VoicetagItem[sz];
                        itemIdx = 0;
                        grammar2.resetAllSlots();
                        break;
                        
                    case STATE_LOAD_VOICETAG_IDX:
                        // Create the voicetag
                        String loadPath = QSDK +"/"+ names[itemIdx] + ".vtg";
                        logDebug("Load Path:" + loadPath);
                        voicetags[itemIdx] = VoicetagItem.create(loadPath, voicetagListener);
                        loadPath = null;
                        // Load the voicetag
                        voicetags[itemIdx].load();
                        setStateLocked(STATE_LOADING_VOICETAG_IDX);
                        break;
                   
                    case STATE_ADD_VOICETAGITEM_GRAMMAR2:
                        Vector<SrecGrammar.Item> Items = new Vector<SrecGrammar.Item>();
                        String slotName = "@Names";
                        int weight = 1;
                        for (int i = 0; i < names.length; i++)
                        {
                            logDebug("item "  + i + ": "  + names[i]);
                            String semanticValue = "V='"  + semanticValues[i] + "'";
                            SrecGrammar.Item item = new SrecGrammar.Item(voicetags[i],weight,semanticValue);
                            Items.add(item);
                        }
                        grammar2.addItemList(slotName, Items); // wait for G2Listener.onAddItemList()
                        Items.removeAllElements();
                        Items = null;
                        setStateLocked(STATE_ADDING_VOICETAGITEM);
                        break;

                    case STATE_COMPILE_GRAMMAR2:
                        itemIdx = 0;
                        grammar2.compileAllSlots();
                        setStateLocked(STATE_COMPILING_GRAMMAR2);
                        break;
                        
                    case STATE_SAVE_GRAMMAR2:
                        String grammarPath = QSDK +"/grammarVoicetag.g2g";
                        grammar2.save(grammarPath);
                        setStateLocked(STATE_SAVING_GRAMMAR2);
                        break;
                    
                    case STATE_RECOGNIZE_WITH_GRAMMAR2:
                         String soundPath = QSDK +"/VTAudio"+ names[itemIdx++] + ".raw";
                         setStateLocked(STATE_RECOGNIZING_WITH_GRAMMAR2);
                        // Pass sound file used to create the voicetag
                        // and grammar for recognition
                        recognize(soundPath,grammar2); //wait for RecognizerListener.onStopped()
                        break;   
                        
                    case STATE_RESET:
                        // Increment iteration number
                        iN = iN + 1;
                        // Clean all variables
                        Clean();
                        // Start again
                        setStateLocked(STATE_SHOW_ITERATION);
                        break;
                default:
                        break;
                }
            }
        }
        //TODO remove this once android fixed the shutdownHook for our automatic System.dispose
        android.speech.recognition.impl.System.getInstance().dispose();
    }
    public void setStateLocked(int newState)
    {
        logDebug(stateNames[m_State] + " --> " + stateNames[newState]);
        m_State = newState;
    }

    public void setState(int newState)
    {
        synchronized(mutex)
        {
            logDebug(stateNames[m_State] + " --> " + stateNames[newState]);
            m_State = newState;
            mutex.notify();
        }
    }
    
    public void setMicState(boolean recording)
    {
        synchronized(micMutex)
        {
           isRecording = recording; 
        }
    }
    public int getState()
    {
        int result = 0;
        synchronized(mutex)
        {
            result = m_State;
        }
        return result;
    }
    
    public void createRecognizer() throws Exception
    {
        logDebug("before EmbeddedRecognizer.getInstance()");
        recognizer = EmbeddedRecognizer.getInstance();
        logDebug("after  EmbeddedRecognizer.getInstance(): "  +
        recognizer.getClass().getName() + " "  + recognizer.toString());
        
        // Initialize Recognizer with configuration 11K
        String config = ESRSDK + "/config/en.us/baseline11k.par";
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
        logDebug("before EmbeddedRecognizer.setlistener()");
        try
        {
            recognizer.setListener(this);
        }
        catch (Exception e)
        {
            logDebug("exception from EmbeddedRecognizer.setListener()");
            throw e;
        }
        logDebug("after EmbeddedRecognizer.setlistener()");
    }

    public void initApp() throws Exception
    {
        // Create recognizer
        createRecognizer();
        // Set Listeners
        voicetagListener = new VTGListener();
        micListener = new MICListener();
        // Create mic                       
        mic = Microphone.getInstance();
        mic.setCodec(Codec.PCM_16BIT_11K);
        mic.setListener(micListener);
        
        mfwListener = new MFWListener();
        mfw = MediaFileWriter.create(mfwListener);
    }
     
    public void setParams(boolean turnOn)
    {
        System.out.println("Setting parameter");
        // Change the parameters
        Hashtable<String,String> lstSetParams = new Hashtable<String,String>();
        lstSetParams.put("enableGetWaveform",(turnOn)?"1":"0");
        recognizer.setParameters(lstSetParams);
        //wait for RecognizerListener.onParametersSet()
        lstSetParams.clear();
        lstSetParams = null;
    }
    
    public SrecGrammar createGrammar(
            java.lang.String grammarURI,
            SrecGrammarListener grammarListener) throws Exception
    {
        logDebug("before CreateGrammar("  + grammarURI + ")");
        SrecGrammar _grammar =
          (SrecGrammar) recognizer.createGrammar(grammarURI, grammarListener);
        logDebug("after recognizer.createGrammar("  + grammarURI + ")");
        return _grammar;
    }

    public void startRecordRecognize()
    {
        mic.start();
        logDebug("Start recording...");
        // Start recognition
        recognizer.recognize(audioStream, grammar1);
        logDebug("recognize()");
    }
     
    //asynchronous call
    public void recognize(String audioFile, SrecGrammar _grammar)
    throws Exception
    {
        logDebug("+recognize()");
        logDebug(audioFile);
        
        //cleanup other objects from previous recognition
        //just in case they remebered some state and we would need to get out of here
        //on exception
        audioSourceListener = null;
        mfr = null;
        audioStream = null;
        audioSourceListener = new MFRListener();
        mfr =MediaFileReader.create(audioFile, audioSourceListener);
        //make MediaFileReader file look more like microphone
        mfr.setMode(MediaFileReader.Mode.REAL_TIME);
        //create AudioStream 
        audioStream = mfr.createAudio();
        //start capturing audio
        mfr.start();
        // Start recognition
        recognizer.recognize(audioStream, _grammar);
        logDebug("-recognize()");
    }
    
    public void Clean()
    {
        grammar1 = null;
        itemIdx = 0;
        audioSourceListener = null;
        audioStream = null;
        audioToSave = null;
        mfr = null;
        currentVoicetag= null;
        voicetags = null;
    }
    
      
    public void logDebug(String msg)
    {
         System.out.println(msg);
    }
  
    /* -------------------------------------------------------
     * Media File Reader Listener 
     * -------------------------------------------------------*/
    private class MFRListener implements 
            MediaFileReaderListener
    {
        public void onError(Exception e)
        {
          logDebug("MFRListener: onError(): "  + e.toString());
          Logger.getInstance().error("MFRListener: onError(): "  + e.toString());
          setState(STATE_ERROR);
        }

        public void onStarted()
        {
          logDebug("MFRListener: onStarted()");
        }

        public void onStopped()
        {
           logDebug("MFRListener: onStopped()");
        }
    }
    
    /* -------------------------------------------------------
     * Media File Writer Listener
     * -------------------------------------------------------*/
    private class MFWListener implements MediaFileWriterListener
    {
        public void onError(Exception e)
        {
           logDebug("MFWListener: onError(): "  + e.toString());
           Logger.getInstance().error("MFWListener: onError(): "  + e.toString());
           setState(STATE_ERROR);
        }

        public void onStopped()
        {
          logDebug("MFWListener: onStopped()");
           
        }
    }
  
     /* -------------------------------------------------------
     * Microphone Listener 
     * -------------------------------------------------------*/
    public class MICListener implements MicrophoneListener
    {
      public void onStarted()
      {
        setMicState(true);
        logDebug("MICListener: onStarted()");
        logDebug("----------------------------");
        logDebug("        PLEASE SAY...");
        logDebug(names[itemIdx]);
        logDebug("----------------------------");
      }

      public void onStopped()
      {
        setMicState(false);
        logDebug("MICListener: onStopped()");
      }

      public void onError(Exception e)
      {
        logDebug("MICListener: onError(): "  + e.toString());
        Logger.getInstance().error("MICListener: onError(): "  + e.toString());
        setState(STATE_ERROR);
      }
    }
    
    /* -------------------------------------------------------
     * Voicetag Listener 
     * -------------------------------------------------------*/
    public class  VTGListener implements VoicetagItemListener
    {
        public void onSaved(String path)
        {
             logDebug("VTGListener: onSaved: "  + path);
             currentVoicetag = null;
             itemIdx++; //Next enrollment
             if (itemIdx<names.length)
                setState(STATE_RESET_ENROLLMENT); // goto create next voicetag
             else
             {
                itemIdx = 0;                      // reset value
                setState(STATE_SET_PARAM_OFF);    // disable recognize parameter
             }
        }

        public void onLoaded()
        {
            logDebug("VTGListener: onLoaded");
            itemIdx++; //Next 
            if (itemIdx<names.length)
               setState(STATE_LOAD_VOICETAG_IDX); // goto create load voicetag
            else
            {
               itemIdx = 0;                      // reset value
               setState(STATE_ADD_VOICETAGITEM_GRAMMAR2); 
            }
        }

        public void onError(Exception e)
        {
            currentVoicetag = null;
            logDebug("VTGListener: onError: "  + e.toString());
            Logger.getInstance().error("VTGListener: onError(): "  + e.toString());
            setState(STATE_ERROR); 
        }
    }
   /* -------------------------------------------------------
    * Grammar 1  Listener 
    * -------------------------------------------------------*/
    private class G1Listener extends AbstractSrecGrammarListener
        implements GrammarListener
    {
        @Override
        public void onError(Exception e)
        {
            logDebug("G1Listener: onError: "  + e.toString());
            Logger.getInstance().error("G1Listener: onError(): "  + e.toString());
            setState(STATE_ERROR);
        }
        
        @Override
        public void onCompileAllSlots()
        {
            logDebug("G1Listener: onCompileAllSlots");
            // Goto create microphone
            setState(STATE_MICROPHONE);
        }
        
        @Override
        public void onLoaded()
        {
             logDebug("G1Listener:  onLoaded");
             grammar1.compileAllSlots();  //wait for G1Listener.onCompileAllSlots()
             setState(STATE_COMPILING_GRAMMAR1);
        }
        @Override
        public void onUnloaded()
        {
            logDebug("G1Listener:   onUnloaded");
            setState(STATE_CREATE_GRAMMAR2);
        }
    }
    
    /* -------------------------------------------------------
     * Grammar 2 Listener 
     * -------------------------------------------------------*/
    private class G2Listener extends AbstractSrecGrammarListener
        implements GrammarListener
    {
        @Override
        public void onResetAllSlots()
        {
            logDebug("G2Listener: onResetAllSlots");
            setState(STATE_LOAD_VOICETAG_IDX);
        }
        @Override
        public void onCompileAllSlots()
        {
            logDebug("G2Listener: onCompileAllSlots");
            setState(STATE_SAVE_GRAMMAR2);
        }
        
        @Override
        public void onSaved(String path)
        {
            logDebug("G2Listener: onSaved :"+path);
            setState(STATE_RECOGNIZE_WITH_GRAMMAR2);
        }

        @Override
        public void onError(Exception e)
        {
            logDebug("G2Listener: onError: "  + e.toString());
            Logger.getInstance().error("G2Listener: onError(): "  + e.toString());
            setState(STATE_ERROR);
        }

        @Override
        public void onLoaded()
        {
            logDebug("G2Listener: onLoaded");
            setState(STATE_SHOW_ITERATION);
        }
     
        @Override
        public void onAddItemList()
        {
            logDebug("G2Listener: onAddItemList");
            logDebug("G2Listener: Finish adding voicetags to grammar slot");
            voicetags = null;
            setState(STATE_COMPILE_GRAMMAR2);
        }
    
        @Override
        public void onAddItemListFailure(int index, Exception e)
        {
            logDebug("G2Listener: onAddItemListFailure Item:"+index+ " E:"+e.toString());
            setState(STATE_ERROR);
        }
    }

    
    /* -------------------------------------------------------
     * Recognizer Listener 
     * -------------------------------------------------------*/
    @Override
    public void onParametersSetError(Hashtable<String, String> parameters,
    Exception e)
    {
       logDebug("onParametersSetError!\n "+e.toString()); 
       Logger.getInstance().error("Recognition Listener: onParametersSetError(): "  + e.toString());
       setState(STATE_ERROR);
    }

    @Override
    public void onParametersSet(Hashtable<String, String> parameters)
    {
       logDebug("onParametersSet: "); 
       Enumeration e = parameters.keys();
       String key = null;
       String value = null;
       while(e.hasMoreElements()) {
           key = (String) e.nextElement();
           value = parameters.get(key);
           logDebug(key+" = "+value);    
       } 
       key = null;
       value = null;
       if (getState()==STATE_SETTING_PARAM_ON) 
         setState(STATE_CREATE_GRAMMAR1);
       else
         setState(STATE_UNLOAD_GRAMMAR1);
    }

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
        logDebug("onExpectedError "+ reason.toString());
      
        // If could not recognize when enrollment then
        // goto end
        if (getState()==STATE_RECORD_RECOGNIZING)
        {
            Logger.getInstance().error("Recognition Failure: "  + reason.toString());
            mic.stop();
            try {
              System.out.println("Sleep 1800 ms (workaround for Sooner audio driver bug)");
              Thread.sleep(1800);  // workaround for bug in closing + restarting Android Sooner audio driver.
              System.out.println("Woke up");
            } catch (Exception err) {
              System.out.println("Exception from Thread.sleep(): " + err.toString());
            }               
            setState(STATE_ERROR);
        }
    }

    @Override
    public void onRecognitionSuccess(RecognitionResult result)
    {
      if( result instanceof NBestRecognitionResult )
      {
         // if enrollment recognition, create Voicetags
        if (getState()==STATE_RECORD_RECOGNIZING)
        {
            // Stop microphone
            mic.stop();
            try {
              System.out.println("Sleep 1800 ms (workaround for Sooner audio driver bug)");
              Thread.sleep(1800);  // workaround for bug in closing + restarting Android Sooner audio driver.
              System.out.println("Woke up");
            } catch (Exception err) {
              System.out.println("Exception from Thread.sleep(): " + err.toString());
            }               
            
            // Saving also the sound file we used for recognition
            String savePath = QSDK +"/VTAudio"+ names[itemIdx] + ".raw";
            logDebug("Saving..."+ savePath);
            mfw.save(audioToSave, savePath); //wait for MFWListener.onStopped()
            
            // Create a voicetag with id equal to the 
            // name 
            String id = names[itemIdx]+"-Id";
            currentVoicetag = ((NBestRecognitionResult)result).createVoicetagItem(id, voicetagListener);
            // Save the voicetag
            savePath = QSDK +"/"+ names[itemIdx] + ".vtg";
            currentVoicetag.save(savePath);
        
            setState(STATE_WAIT_SAVED_VOICETAG);
            id = null;
            savePath = null;
            return;
        }
        
        int numResults = ((NBestRecognitionResult)result).getSize();
        logDebug("onRecognitionResult: "  + numResults);
        NBestRecognitionResult.Entry entry;
        // Display the recognition results
        String literal = null;
        String meaning = null;
        String dspText = null;
        for (int i = 0; i < numResults; i++)
        {
          entry = ((NBestRecognitionResult)result).getEntry(i);
          if (entry!=null)
          {
              dspText = "result "  + (i + 1);
              literal = entry.getLiteralMeaning();
              meaning = (String) entry.getSemanticMeaning();
              if (literal!=null) dspText=dspText+"Literal:"+literal;
              dspText= dspText+ entry.getConfidenceScore();
              if (meaning!=null) dspText=dspText+"meaning:"+meaning;
              logDebug(dspText);
          }
        }
        literal = null;
        meaning = null;
        dspText = null;
        logDebug("================================");
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
        if (getState() == STATE_RECOGNIZING_WITH_GRAMMAR2)
        { 
            if (itemIdx<names.length)
                setState(STATE_RECOGNIZE_WITH_GRAMMAR2);
            else
                setState(STATE_RESET);
        }
        else 
            logDebug("State:"+stateNames[getState()]); 
        logDebug("-onStopped()");
    }

    @Override
    public void onError(java.lang.Exception e)
    {
        if (getState()==STATE_RECORD_RECOGNIZING)
        {
            mic.stop(); // Stop microphone
            try {
              System.out.println("Sleep 1800 ms (workaround for Sooner audio driver bug)");
              Thread.sleep(1800);  // workaround for bug in closing + restarting Android Sooner audio driver.
              System.out.println("Woke up");
            } catch (Exception err) {
              System.out.println("Exception from Thread.sleep(): " + err.toString());
            }                  
        }
        audioStream = null;
        audioToSave = null;
        logDebug("Recognition Listener onError:"+e.toString());
        Logger.getInstance().error("Recognition Listener: onError(): "  + e.toString());
        setState(STATE_ERROR);
    }
}
