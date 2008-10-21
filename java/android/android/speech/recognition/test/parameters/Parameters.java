/*---------------------------------------------------------------------------*
 *  Parameters.java                                                          *
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


package android.speech.recognition.test.parameters;

import android.speech.recognition.AbstractRecognizerListener;
import android.speech.recognition.EmbeddedRecognizer;
import java.util.Hashtable;
import java.util.Vector;
import java.util.Enumeration;

/**
 */
public class Parameters   extends AbstractRecognizerListener {

    private static final String ESRSDK = (System.getenv("ESRSDK") != null) ? System.getenv("ESRSDK") : "/system/usr/srec";
    private static final String QSDK = (System.getenv("QSDK") != null) ? System.getenv("QSDK") : "/system/usr/srec";
    private EmbeddedRecognizer recognizer;


    public static void main(String[] args) throws Exception
    {
      System.out.println("Robustness1: ESRSDK = " + ESRSDK);

      new Parameters();
    }

    public void allocate(java.lang.String config) throws Exception
    {
      System.out.println("before EmbeddedRecognizer.getInstance()");

      recognizer = EmbeddedRecognizer.getInstance();

      System.out.println("after EmbeddedRecognizer.getInstance(): "  +
        recognizer.getClass().getName() + " "  + recognizer.toString());

      System.out.println("before EmbeddedRecognizer.configure("  + config + ")");
      try
      {
        recognizer.configure(config);
      }
      catch (Exception e)
      {
        System.out.println("exception from EmbeddedRecognizer.configure()");
        throw e;
      }
      System.out.println("after EmbeddedRecognizer.configure()");

      System.out.println("before EmbeddedRecognizer.setlistener()");
      recognizer.setListener(this);
      System.out.println("after EmbeddedRecognizer.setlistener()");
    }

    @Override
    public void onParametersGetError(Vector<String> parameters, Exception e)
    {
      System.out.println("onParametersGetError!\n "+e.toString());
    }

    @Override
    public void onParametersSetError(Hashtable<String, String> parameters,
    Exception e)
    {
      System.out.println("onParametersSetError!\n "+e.toString());
    }

    @Override
    public void onParametersGet(Hashtable<String, String> parameters)
    {
      System.out.println("onParametersGet: ");
      Enumeration e = parameters.keys();
      String key = null;
      String value = null;
      while(e.hasMoreElements()) {
        key = (String) e.nextElement();
        value = parameters.get(key);
        System.out.println(key+" = "+value);
      }
      key = null;
      value = null;
    }

    @Override
    public void onParametersSet(Hashtable<String, String> parameters)
    {
      System.out.println("onParametersSet: ");
      Enumeration e = parameters.keys();
      String key = null;
      String value = null;
      while(e.hasMoreElements()) {
        key = (String) e.nextElement();
        value = parameters.get(key);
        System.out.println(key+" = "+value);
      }
      key = null;
      value = null;
    }

    public Parameters() throws Exception
    {
      System.out.println("Initialising...");

      try {
        String path = ESRSDK + "/config/en.us/baseline11k.par";
        allocate(path);
      } catch (Exception e) {
        System.out.println("Exception from ICLRecognizer.allocate(): " + e.toString());
        System.exit(1);
      }

      // Wait for callback to display parameters
      try {Thread.currentThread().sleep(1500);} catch(InterruptedException e){}

      System.out.println("Retrieving parameter values...");
      // Create list of parameters to retrieve
      Vector<String> lstGetParams = new Vector<String>();
      lstGetParams.add("SREC.Recognizer.utterance_timeout");
      lstGetParams.add("CREC.Recognizer.terminal_timeout");
      lstGetParams.add("CREC.Recognizer.optional_terminal_timeout");
      lstGetParams.add("CREC.Recognizer.non_terminal_timeout");
      lstGetParams.add("CREC.Recognizer.eou_threshold");
      lstGetParams.add("CREC.Frontend.samplerate");
      // Retrieve actual params. Result will be display inside the callback
      // function (CLRecognizer.java)
      recognizer.getParameters(lstGetParams);

      // Wait for callback to display parameters
      try {Thread.currentThread().sleep(1500);} catch(InterruptedException e){}

      System.out.println("Setting new parameter values...");
      // Change the parameters
      Hashtable<String,String> lstSetParams = new Hashtable<String,String>();
      lstSetParams.put("SREC.Recognizer.utterance_timeout","100");
      lstSetParams.put("CREC.Recognizer.terminal_timeout","200");
      lstSetParams.put("CREC.Recognizer.optional_terminal_timeout","300");
      lstSetParams.put("CREC.Recognizer.non_terminal_timeout","400");
      lstSetParams.put("CREC.Recognizer.eou_threshold","500");
      lstSetParams.put("CREC.Frontend.samplerate","8000"); // Change sample rate to 8K
      //lstSetParams.put("CREC.Frontend.samplerate","11025"); // Change sample rate to 11K
      
      recognizer.setParameters(lstSetParams);

      // Wait for callback to display parameters
      try {Thread.currentThread().sleep(1500);} catch(InterruptedException e){}

      // Retrieve new params.
      // Result will be display inside the callback
      // function (CLRecognizer.java)
      System.out.println("Getting new parameter values...");
      recognizer.getParameters(lstGetParams);

      // Wait for callback to display parameters
      try {Thread.currentThread().sleep(1500);} catch(InterruptedException e){}

      lstGetParams.clear();
      lstGetParams= null;
      lstSetParams.clear();
      lstSetParams = null;
    }
}
