/*---------------------------------------------------------------------------*
 *  Robustness3.java                                                         *
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

import android.speech.recognition.AbstractSrecGrammarListener;
import android.speech.recognition.AbstractRecognizerListener;
import android.speech.recognition.AudioStream;
import android.speech.recognition.Codec;
import android.speech.recognition.EmbeddedRecognizer;
import android.speech.recognition.RecognitionResult;
import android.speech.recognition.SrecGrammarListener;
import android.speech.recognition.Grammar;
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
import android.speech.recognition.WordItem;
import android.speech.recognition.Logger;
import java.util.Hashtable;
import java.util.Vector;
import java.util.Enumeration;
import java.io.File;
/**
 */
public class Robustness3 extends AbstractRecognizerListener {

    private static final String ESRSDK = (System.getenv("ESRSDK") != null) ? System.getenv("ESRSDK") : "/system/usr/srec";
    private static final String QSDK = (System.getenv("QSDK") != null) ? System.getenv("QSDK") : "/system/usr/srec";
    private static final int STATE_IDLE                           = 0;
    private static final int STATE_CREATE_RECOGNIZER              = 1;
    private static final int STATE_SHOW_ITERATION                 = 2;
    private static final int STATE_CREATE_GRAMMAR1                = 3;
    private static final int STATE_CREATE_GRAMMAR2                = 4;
    private static final int STATE_LOAD_GRAMMAR1                  = 5;
    private static final int STATE_LOADING_GRAMMAR1               = 6;
    private static final int STATE_COMPILING_GRAMMAR1             = 7;
    private static final int STATE_SAVE_GRAMMAR1                  = 8;
    private static final int STATE_SAVING_GRAMMAR1                = 9;
    private static final int STATE_UNLOAD_GRAMMAR1                = 10;
    private static final int STATE_UNLOADING_GRAMMAR1             = 11;
    private static final int STATE_CREATE_GRAMMAR3                = 12;
    private static final int STATE_LOADING_GRAMMAR3               = 13;
    private static final int STATE_RECOGNIZE_WITH_GRAMMAR3        = 14;
    private static final int STATE_RECOGNIZING_WITH_GRAMMAR3      = 15;
    private static final int STATE_SAVING_CONTATCS_TEST_SOUND     = 16;
    private static final int STATE_UNLOAD_GRAMMAR3                = 17;
    private static final int STATE_UNLOADING_GRAMMAR3             = 18;
    private static final int STATE_LOAD_GRAMMAR2                  = 19;
    private static final int STATE_LOADING_GRAMMAR2               = 20;
    private static final int STATE_RECOGNIZE_WITH_GRAMMAR2        = 21;
    private static final int STATE_RECOGNIZING_WITH_GRAMMAR2      = 22;
    private static final int STATE_SAVING_ACTION_TEST_SOUND       = 23;
    private static final int STATE_UNLOAD_GRAMMAR2                = 24;
    private static final int STATE_UNLOADING_GRAMMAR2             = 25;
    private static final int STATE_MICROPHONE                     = 26;
    private static final int STATE_MICROPHONE_RECORDING           = 27;
    private static final int STATE_RECORDING_STOP                 = 28;
    private static final int STATE_STOPPING_MICROPHONE            = 29;
    private static final int STATE_SAVING_RECORDED_AUDIO          = 30;
    private static final int STATE_COMPARE_FILES                  = 31;
    private static final int STATE_GET_PARAMS                     = 32;
    private static final int STATE_GETTING_PARAMS                 = 33;
    private static final int STATE_SET_PARAMS                     = 34;
    private static final int STATE_SETTING_PARAMS                 = 35;
    private static final int STATE_RESET_PARAMS                   = 36;
    private static final int STATE_RESETTING_PARAMS               = 37;
    private static final int STATE_RESET                          = 38;
    private static final int STATE_QUIT                           = 39;
    private static final int STATE_ERROR                          = 40;
    private static final int STATE_ADD_SLOT_ITEM_LIST             = 41;
    private static final int STATE_MICROPHONE_STARTING            = 42;
    private static final int STATE_SAVE_RECORDED_AUDIO            = 43;

    private String[] stateNames = { 
    "IDLE",
    "CREATE_RECOGNIZER",
    "SHOW_ITERATION",
    "CREATE_GRAMMAR1",
    "CREATE_GRAMMAR2",
    "LOAD_GRAMMAR1",
    "LOADING_GRAMMAR1",
    "COMPILING_GRAMMAR1",
    "SAVE_GRAMMAR1",
    "SAVING_GRAMMAR1",
    "UNLOAD_GRAMMAR1",
    "UNLOADING_GRAMMAR1",
    "CREATE_GRAMMAR3",
    "LOADING_GRAMMAR3",
    "RECOGNIZE_WITH_GRAMMAR3",
    "RECOGNIZING_WITH_GRAMMAR3",
    "SAVING_CONTATCS_TEST_SOUND",
    "UNLOAD_GRAMMAR3",
    "UNLOADING_GRAMMAR3",
    "LOAD_GRAMMAR2",
    "LOADING_GRAMMAR2",
    "RECOGNIZE_WITH_GRAMMAR2",
    "RECOGNIZING_WITH_GRAMMAR2",
    "SAVING_ACTION_TEST_SOUND",
    "UNLOAD_GRAMMAR2",
    "UNLOADING_GRAMMAR2",
    "MICROPHONE",
    "MICROPHONE_RECORDING",
    "RECORDING_STOP",
    "STOPPING_MICROPHONE",
    "SAVING_RECORDED_AUDIO",
    "COMPARE_FILES",
    "GET_PARAMS",
    "GETTING_PARAMS",
    "SET_PARAMS",
    "SETTING_PARAMS",
    "RESET_PARAMS",
    "RESETTING_PARAMS",
    "RESET",
    "QUIT",
    "ERROR",
    "ADD_SLOT_ITEM_LIST",
    "STATE_MICROPHONE_STARTING",
    "STATE_SAVE_RECORDED_AUDIO"
    };
  
    private int iN = 0;
    
    private int m_State = STATE_IDLE;
    
    private int loadedGrammar = 0;
    private Object mutex;
    private EmbeddedRecognizer recognizer;  
    private SrecGrammar grammar1;
    private SrecGrammar grammar2;
    private SrecGrammar grammar3;
   
    private String      recognitionFailureReason;
    private Exception   recognitionException;
    private boolean     recognitionCancelled = false;
    private MFRListener audioSourceListener;
    private AudioStream audioStream;
    private AudioStream audioToSave;
    private MediaFileReader mfr;
    private MediaFileWriter mfw;
    private Microphone mic;
    private String[]    pathG;
    private String[]    lstSoundsG;   
    private String[]    recognitionResults;
    private String[]    names;
    private MICListener micListener;
    private MFWListener mfwListener;
  
    public static void main(String[] args) throws Exception
    {
        System.out.println("Robustness3: ESRSDK = " + ESRSDK);
        Logger.getInstance().info("Robustness3");
        //android.speech.recognition.Logger.getInstance().setLoggingLevel(android.speech.recognition.Logger.LogLevel.LEVEL_TRACE);
        new Robustness3();
    }
    
    /*-------------------------------------------------------
    * Robustness 3 Diagram Flow
    * -------------------------------------------------------
    * 1) Create recognizer
    * 2) (start)Show Iteration - Create Grammar1
    * 3) Create Grammar2
    * 4) Load Grammar1 - add WordItems - Compile Grammar1
    * 5) Save Grammar as Grammar3 - Unload Grammar 1
    * 6) Load Grammar 3
    * 7) MFR:Load sound (take one sound of a list A  ex. lstASound[i%3]).
    * 8) Recognize with Grammar 3
    * 9) MFW:Save sound used in recognition A
    * 10)Unload Grammar 3
    * 11)Load Grammar 2
    * 12)MFR:Load sound (take one sound of a list B ex. lstBSound[i%3]).
    * 13)Recognize with Grammar 2
    * 14)MFW:Save sound used in recognition B
    * 15)Unload Grammar 2
    * 16)Create Microphone
    * 17)Start Mic 
    * 18)Record for 2 secs
    * 19)Stop Mic
    * 20)MFW:Save recorded sound
    * 21)Compare size with a ref size (#seconds of (18))
    * 22)Recognizer Get Params
    * 23)Recognizer Set Params
    * 24)Delete variables 
    * 25)Goto (2)
    * -------------------------------------------------------*/
    public Robustness3() throws Exception
    {
        // Store two grammar paths
        pathG = new String[2];
        pathG[0] = ESRSDK + "/config/en.us/grammars/bothtags5.g2g"; 
        pathG[1] = ESRSDK + "/config/en.us/grammars/bothtags5.g2g";

        // Store six sounds files paths (3 for each grammar)
        lstSoundsG = new String[6];
        // Sounds for grammar 1
        lstSoundsG[0] = ESRSDK + "/config/en.us/audio/v139/v139_113.nwv";
        //phone delete jen parker
        lstSoundsG[1] = ESRSDK + "/config/en.us/audio/v139/v139_103.nwv";
        //phone delete john martinez
        lstSoundsG[2] = ESRSDK + "/config/en.us/audio/v139/v139_254.nwv";
        //andrew evans
        
        // Sounds for grammar 2
        lstSoundsG[3]= ESRSDK + "/config/en.us/audio/m252/m252a11e.nwv";
        //traffic information
        lstSoundsG[4]= ESRSDK + "/config/en.us/audio/m252/m252a10e.nwv";
        //changer
        lstSoundsG[5]= ESRSDK + "/config/en.us/audio/m252/m252a12e.nwv";
        //forward
    
        names = new String[3];
        names[0] = "Jen Parker";
        names[1] = "John Martinez";
        names[2] = "Andrew Evans";
        
        // Create object that will be used to synch the states
        mutex = new Object();
       
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
                    case STATE_SAVING_GRAMMAR1:
                    case STATE_UNLOADING_GRAMMAR1:
                    case STATE_LOADING_GRAMMAR3:
                    case STATE_RECOGNIZING_WITH_GRAMMAR3:
                    case STATE_SAVING_CONTATCS_TEST_SOUND:
                    case STATE_UNLOADING_GRAMMAR3:
                    case STATE_LOADING_GRAMMAR2:
                    case STATE_RECOGNIZING_WITH_GRAMMAR2:
                    case STATE_SAVING_ACTION_TEST_SOUND:
                    case STATE_UNLOADING_GRAMMAR2:
                    case STATE_STOPPING_MICROPHONE:
                    case STATE_SAVING_RECORDED_AUDIO:
                    case STATE_GETTING_PARAMS:
                    case STATE_SETTING_PARAMS:
                    case STATE_RESETTING_PARAMS:
                    case STATE_ADD_SLOT_ITEM_LIST:
                    case STATE_MICROPHONE_STARTING:
                        // Idle state
                        try {
                          //Thread.currentThread().sleep(5);
                            mutex.wait();
                        }
                        catch(InterruptedException e)
                        {
                            System.out.println("Exception from Thread.sleep: " +
                                           e.toString());
                            throw e;
                        }
                        break;
                    case STATE_CREATE_RECOGNIZER: 
                        createRecognizer();
                        createMicrophone();
                        setStateLocked(STATE_SHOW_ITERATION);
                        break;
                        
                    case STATE_SHOW_ITERATION:
                        logDebug("---------------------------------------");
                        logDebug("Iteration:"+iN);
                        logDebug("---------------------------------------");                               
                        setStateLocked(STATE_CREATE_GRAMMAR1);
                        break;
                    case STATE_CREATE_GRAMMAR1:
                        grammar1 = createGrammar(pathG[0],new G1Listener());
                        setStateLocked(STATE_CREATE_GRAMMAR2);
                        break;
                        
                    case STATE_CREATE_GRAMMAR2:
                        grammar2 = createGrammar(pathG[1], new G2Listener());
                        setStateLocked(STATE_LOAD_GRAMMAR1);
                        break;
                    
                    case STATE_LOAD_GRAMMAR1:
                        setStateLocked(STATE_LOADING_GRAMMAR1);
                        loadedGrammar = 1;
                        grammar1.load(); //wait for G1Listener.onLoaded()
                        break;
                   
                    case STATE_SAVE_GRAMMAR1:
                        setStateLocked(STATE_SAVING_GRAMMAR1);
                        grammar1.save(QSDK+"/grammar3.g2g"); //wait for G1Listener.onSaved()
                        break;
                        
                    case STATE_UNLOAD_GRAMMAR1:
                        setStateLocked(STATE_UNLOADING_GRAMMAR1);
                        grammar1.unload();//wait for G1Listener.onUnloaded()
                        break;
                    
                    case STATE_CREATE_GRAMMAR3:
                        setStateLocked(STATE_LOADING_GRAMMAR3);
                        grammar3 = createGrammar(QSDK+"/grammar3.g2g",new G2Listener());
                        loadedGrammar = 3;
                        grammar3.load();//wait for G2Listener.onLoaded()
                        break;
                        
                    case STATE_RECOGNIZE_WITH_GRAMMAR3:
                        setStateLocked(STATE_RECOGNIZING_WITH_GRAMMAR3);
                        // Pass sound file and grammar use for
                        // recognition
                        recognize(lstSoundsG[iN%3],grammar3); //wait for RecognizerListener.onStopped()
                        break;   

                    case STATE_UNLOAD_GRAMMAR3:
                        setStateLocked(STATE_UNLOADING_GRAMMAR3);
                        grammar3.unload(); //wait for G2Listener.onUnloaded()
                        break;
                        
                    case STATE_LOAD_GRAMMAR2:
                        setStateLocked(STATE_LOADING_GRAMMAR2);
                        loadedGrammar = 2;
                        grammar2.load();//wait for G2Listener.onLoaded()
                        break;     
                   
                    case STATE_RECOGNIZE_WITH_GRAMMAR2:
                         setStateLocked(STATE_RECOGNIZING_WITH_GRAMMAR2);
                        // Pass sound file and grammar use for
                        // recognition
                        recognize(lstSoundsG[3+(iN%3)],grammar2); //wait for RecognizerListener.onStopped()
                        break;   
                        
                    case STATE_UNLOAD_GRAMMAR2:
                        setStateLocked(STATE_UNLOADING_GRAMMAR2);
                        grammar2.unload();//wait for G2Listener.onUnloaded()
                        break;  

                    case STATE_MICROPHONE:
                        loadedGrammar = 0; // No grammar is loaded
                        MicRecord();
                        break;
                        
                    case STATE_MICROPHONE_RECORDING:
                        //record for 2 seconds
                        try {Thread.sleep(2000);}catch(InterruptedException e)
                        {
                            System.out.println("Exception from Thread.sleep: " +
                                           e.toString());
                            throw e;
                        }
                        // Lets stop the recording
                        setStateLocked(STATE_RECORDING_STOP);
                        break;

                    case STATE_RECORDING_STOP:
                        mic.stop();//wait for MICListener.onStopped();
                        setStateLocked(STATE_STOPPING_MICROPHONE);  
                        break;

                    case STATE_SAVE_RECORDED_AUDIO:
                        try {
                          System.out.println("Sleep 1800 ms (workaround for Sooner audio driver bug)");
                          Thread.sleep(1800);  // workaround for bug in closing + restarting Android Sooner audio driver.
                          System.out.println("Woke up");
                        } catch (Exception e) {
                          System.out.println("Exception from Thread.sleep(): " + e.toString());
                        }
                        logDebug("MICListener: Saving record sound R3_microphoneTestSound.raw");
                        String savePath = QSDK + "/R3_microphoneTestSound.raw";
                        mfw.save(audioToSave, savePath); //wait for MFWListener.onStopped();
                        setStateLocked(STATE_SAVING_RECORDED_AUDIO);
                        break;
                        
                    case STATE_COMPARE_FILES:
                        CompareMicFiles();
                        setStateLocked(STATE_GET_PARAMS);
                        break;
                    case STATE_GET_PARAMS:
                        setStateLocked(STATE_GETTING_PARAMS);
                        getParams();
                        break;
                    
                    case STATE_SET_PARAMS:
                        setStateLocked(STATE_SETTING_PARAMS);                        
                        setParams(false);
                        break;
                        
                    case STATE_RESET_PARAMS:
                        setStateLocked(STATE_RESETTING_PARAMS);  
                        setParams(true);
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
        logDebug("after EmbeddedRecognizer.getInstance(): "  +
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
      
    public void createMicrophone()
    {
        micListener = new MICListener();
        mic = Microphone.getInstance();
        mic.setListener(micListener);
        mic.setCodec(Codec.PCM_16BIT_11K);
        
        mfwListener = new MFWListener();
        mfw = MediaFileWriter.create(mfwListener);
        
    }
    
    public SrecGrammar createGrammar(
            java.lang.String grammarURI,
            GrammarListener grammarListener) throws Exception
    {
        logDebug("before CreateGrammar("  + grammarURI + ")");
        SrecGrammar _grammar =
          (SrecGrammar) recognizer.createGrammar(grammarURI, grammarListener);
        logDebug("after recognizer.createGrammar("  + grammarURI + ")");
        return _grammar;
    }

    //asynchronous call
    public void recognize(String audioFile, SrecGrammar _grammar)
    throws Exception
    {
        logDebug("+recognize()");
        logDebug(audioFile);
        //clear results from previous recognition
        recognitionResults = null;
        recognitionFailureReason = null;
        recognitionException = null;

        //cleanup other objects from previous recognition
        //just in case they remebered some state and we would need to get out of here
        //on exception
        audioSourceListener = null;
        mfr = null;
        //if (audioStream!=null) audioStream.dispose();
        audioStream = null;
        //if (audioToSave!=null) audioToSave.dispose();
        audioToSave = null;
        audioSourceListener = new MFRListener();

        mfr = MediaFileReader.create(audioFile, audioSourceListener);

        //make MediaFileReader file look more like microphone
        mfr.setMode(MediaFileReader.Mode.REAL_TIME);

        //create AudioStream 
        audioStream = mfr.createAudio();
        audioToSave = mfr.createAudio();
        //start capturing audio
        mfr.start();
        // Start recognition
        recognizer.recognize(audioStream, _grammar);
        logDebug("-recognize()");
    }
    
    public void Clean()
    {
        loadedGrammar = 0;
        grammar1 = null;
        grammar2 = null;
        grammar3 = null;
        recognitionFailureReason = null;
        recognitionException = null;
        recognitionCancelled = false;
        audioSourceListener = null;
        audioStream = null;
        audioToSave = null;
        mfr = null;
      
    }
    
    public void MicRecord()
    {
        // Clear audio source
        //if (audioToSave != null) audioToSave.dispose();
        audioToSave = null;
        audioToSave = mic.createAudio();
        logDebug("Start recording...");
        mic.start();
        setState(STATE_MICROPHONE_STARTING);
    }
    
    public void CompareMicFiles()
    {
        logDebug("Compare MIC Files");
        String path = QSDK + "/R3_microphoneTestSound.raw";
        logDebug(path);
        File file = new File(path);
        boolean exists = file.exists();
        if (exists) {
            long length = file.length();
            logDebug("Size:"+length);
            if (length<40000 || length>45000)
            {
                logDebug("***** WARNING!!! File size not correct, maybe there is an error.");
                //Logger.getInstance().error("***** WARNING!!! File size:"+ length +" not correct, maybe there is an error.");
            }
        } else {
            logDebug("File or directory does not exist!");
            Logger.getInstance().error("File or directory does not exist:"+QSDK + "/R3_microphoneTestSound.raw");
        }
    }
    
    public void getParams()
    {
        logDebug("Retrieving parameter values...");
        // Create list of parameters to retrieve
        Vector<String> lstGetParams = new Vector<String>();
        lstGetParams.add("SREC.Recognizer.utterance_timeout");
        lstGetParams.add("CREC.Recognizer.terminal_timeout");
        lstGetParams.add("CREC.Recognizer.optional_terminal_timeout");
        lstGetParams.add("CREC.Recognizer.non_terminal_timeout");
        lstGetParams.add("CREC.Recognizer.eou_threshold");
        // Retrieve actual params. Result will be display inside the callback
        // function (CLRecognizer.java)
        recognizer.getParameters(lstGetParams); //wait for RecognizerListener.onParametersGet()
        lstGetParams.clear();
        lstGetParams = null;
    }
    
    public void setParams(boolean reset)
    {
        System.out.println("Setting new parameter values...");
        // Change the parameters
        Hashtable<String,String> lstSetParams = new Hashtable<String,String>();
        lstSetParams.put("SREC.Recognizer.utterance_timeout",(reset)?"400":"100");
        lstSetParams.put("CREC.Recognizer.terminal_timeout",(reset)?"30":"200");
        lstSetParams.put("CREC.Recognizer.optional_terminal_timeout",(reset)?"45":"300");
        lstSetParams.put("CREC.Recognizer.non_terminal_timeout",(reset)?"90":"400");
        lstSetParams.put("CREC.Recognizer.eou_threshold",(reset)?"120":"500");
        recognizer.setParameters(lstSetParams);//wait for RecognizerListener.onParametersSet()
        lstSetParams.clear();
        lstSetParams = null;
    }
    
    public void logDebug(String msg)
    {
         System.out.println(msg);
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
          if (loadedGrammar == 3)
            setState(STATE_UNLOAD_GRAMMAR3);
          else if (loadedGrammar == 2)
            setState(STATE_UNLOAD_GRAMMAR2);
          else
            setState(STATE_COMPARE_FILES);   
        }
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
     * Microphone Listener 
     * -------------------------------------------------------*/
    public class MICListener implements MicrophoneListener
    {
     
      public void onStarted()
      {
        logDebug("MICListener: onStarted()");
        logDebug("Record for 2 secs.");
        setState(STATE_MICROPHONE_RECORDING);
      }

      public void onStopped()
      {
        logDebug("MICListener: onStopped()");
        setState(STATE_SAVE_RECORDED_AUDIO);
      }

      public void onError(Exception e)
      {
        logDebug("MICListener: onError(): "  + e.toString());
        Logger.getInstance().error("MICListener: onError(): "  + e.toString());
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
        public void onCompileAllSlots()
        {
            logDebug("G1Listener: onCompileAllSlots");
            // Save the grammar 1 to file
            setState(STATE_SAVE_GRAMMAR1);
        }

        @Override
        public void onError(Exception e)
        {
            logDebug("G1Listener: onError: "  + e.toString());
            Logger.getInstance().error("G1Listener: onError(): "  + e.toString());
            setState(STATE_ERROR);
        }

        @Override
        public void onLoaded()
        {
            try
            {
                logDebug("G1Listener: onLoaded");
                //add words here
                logDebug("G1Listener: Adding words to grammar slot");
                
                Vector<SrecGrammar.Item> WordItems = new Vector<SrecGrammar.Item>();
                
                String slotName = "@Names";
                for (int i = 0; i < names.length; i++)
                {
                    logDebug("item "  + i + ": "  + names[i]);
                    WordItem word =
                      WordItem.valueOf(names[i], (String) null);
                    int weight = 1;
                    String semanticValue =
                      "V='"  + names[i] + "'";
                    SrecGrammar.Item item = new SrecGrammar.Item(word,weight,semanticValue);
                    WordItems.add(item);
                }
		grammar1.addItemList(slotName, WordItems);
                
                WordItems.removeAllElements();
                logDebug("G1Listener: Starting adding words to grammar slot");
                setState(STATE_ADD_SLOT_ITEM_LIST);
            }
            catch (Exception e)
            {
                logDebug("G1Listener: onLoaded(): "  + e.toString());
                Logger.getInstance().error("G1Listener: onLoaded(): "  + e.toString());
                setState(STATE_ERROR);
            }
        }
        @Override
        public void onSaved(String path)
        {
            logDebug("G1Listener: onSaved");
            // Unload Grammar 1
            setState(STATE_UNLOAD_GRAMMAR1);
        }

        @Override
        public void onUnloaded()
        {
            logDebug("G1Listener: onUnloaded");
            // Create Grammar 3
            setState(STATE_CREATE_GRAMMAR3);
        }
        
        @Override
        public void onAddItemList()
        {
            logDebug("G1Listener: onAddItemList");
            logDebug("G1Listener: Finish adding words to grammar slot");
            setState(STATE_COMPILING_GRAMMAR1);
            grammar1.compileAllSlots();  //wait for G1Listener.onCompileAllSlots()
     }
    
        @Override
        public void onAddItemListFailure(int index, Exception e)
        {
            logDebug("G1Listener: onAddItemListFailure Item:"+index+ " E:"+e.toString());
            setState(STATE_ERROR);
        }
    }
    /* -------------------------------------------------------
     * Grammar 2  Listener 
     * -------------------------------------------------------*/
    private class G2Listener extends AbstractSrecGrammarListener
        implements GrammarListener
    {
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
             logDebug("G2Listener:  onLoaded");
             if (loadedGrammar == 3)
                setState(STATE_RECOGNIZE_WITH_GRAMMAR3);
             else
                setState(STATE_RECOGNIZE_WITH_GRAMMAR2);
        }
        @Override
        public void onUnloaded()
        {
            logDebug("G2Listener:   onUnloaded");
            if (loadedGrammar == 3)
                setState(STATE_LOAD_GRAMMAR2);
            else
                setState(STATE_MICROPHONE);
        }
    }
    
    /* -------------------------------------------------------
     * Recognizer Listener 
     * -------------------------------------------------------*/
    @Override
    public void onParametersGetError(Vector<String> parameters, Exception e)
    {
      logDebug("onParametersGetError!\n "+e.toString()); 
      Logger.getInstance().error("Recognition Listener: onParametersGetError(): "  + e.toString());
      setState(STATE_ERROR);
    }

    @Override
    public void onParametersSetError(Hashtable<String, String> parameters,
    Exception e)
    {
       logDebug("onParametersSetError!\n "+e.toString()); 
       Logger.getInstance().error("Recognition Listener: onParametersSetError(): "  + e.toString());
       setState(STATE_ERROR);
    }

    @Override
    public void onParametersGet(Hashtable<String, String> parameters)
    {
       logDebug("onParametersGet: "); 
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
       
       setState(STATE_SET_PARAMS);
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

       if (getState()==STATE_SETTING_PARAMS) 
         setState(STATE_RESET_PARAMS);
       else
         setState(STATE_RESET);
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
        logDebug("onExpectedError");
        //save the result for future use
        recognitionFailureReason = new String(reason.toString());
        //Logger.getInstance().error("Recognition Failure: "  + recognitionFailureReason);
    }

    @Override
    public void onRecognitionSuccess(RecognitionResult result)
    {
      if( result instanceof NBestRecognitionResult )
      {
        int numResults = ((NBestRecognitionResult)result).getSize();
        logDebug("onRecognitionResult: "  + numResults);

        NBestRecognitionResult.Entry entry;
        recognitionResults = new String[numResults];

        for (int i = 0; i < numResults; i++)
        {
          entry = ((NBestRecognitionResult)result).getEntry(i);
          if (entry!=null)
          {
            recognitionResults[i] = new String((String) entry.getSemanticMeaning());
            logDebug("result "  + (i + 1) + ":"  + entry.getLiteralMeaning() + ":"  +
                entry.getSemanticMeaning() + ":" + entry.getConfidenceScore());
          }
          else
          {
            logDebug("Entry is null.");
          }
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
        
        String savePath = QSDK;
        if (loadedGrammar == 3)
        {
            savePath = savePath + "/R3_contactsTestSound.raw";
            setState(STATE_SAVING_CONTATCS_TEST_SOUND);
        }
        else
        {
            savePath = savePath + "/R3_actionTestSound.raw";
            setState(STATE_SAVING_ACTION_TEST_SOUND);
        }
        logDebug("Saving..."+ savePath);
        mfw.save(audioToSave, savePath); //wait for MFWListener.onStopped()
        logDebug("-onStopped()");
    }

    @Override
    public void onError(java.lang.Exception e)
    {
        logDebug("Recognition Listener onError:"+e.toString());
        Logger.getInstance().error("Recognition Listener: onError(): "  + e.toString());
        recognitionException = e;
        setState(STATE_ERROR);
    }
}
