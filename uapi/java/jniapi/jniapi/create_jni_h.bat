set CLASSPATH="%QSDK%\java\api\dist\api.jar"

"%JAVA_HOME%\bin\javah" -jni -classpath %CLASSPATH% android.speech.recognition.impl.AudioStreamImpl
"%JAVA_HOME%\bin\javah" -jni -classpath %CLASSPATH% android.speech.recognition.impl.DeviceSpeakerImpl
"%JAVA_HOME%\bin\javah" -jni -classpath %CLASSPATH% android.speech.recognition.impl.EmbeddedGrammarImpl
"%JAVA_HOME%\bin\javah" -jni -classpath %CLASSPATH% android.speech.recognition.impl.EmbeddedRecognizerImpl
"%JAVA_HOME%\bin\javah" -jni -classpath %CLASSPATH% android.speech.recognition.impl.MediaFileReaderImpl
"%JAVA_HOME%\bin\javah" -jni -classpath %CLASSPATH% android.speech.recognition.impl.MediaFileWriterImpl
"%JAVA_HOME%\bin\javah" -jni -classpath %CLASSPATH% android.speech.recognition.impl.MicrophoneImpl
"%JAVA_HOME%\bin\javah" -jni -classpath %CLASSPATH% android.speech.recognition.impl.NBestRecognitionResultImpl
"%JAVA_HOME%\bin\javah" -jni -classpath %CLASSPATH% android.speech.recognition.impl.EntryImpl
"%JAVA_HOME%\bin\javah" -jni -classpath %CLASSPATH% android.speech.recognition.impl.SrecGrammarImpl
"%JAVA_HOME%\bin\javah" -jni -classpath %CLASSPATH% android.speech.recognition.impl.GrammarImpl
"%JAVA_HOME%\bin\javah" -jni -classpath %CLASSPATH% android.speech.recognition.impl.WordItemImpl
"%JAVA_HOME%\bin\javah" -jni -classpath %CLASSPATH% android.speech.recognition.impl.VoicetagItemImpl
"%JAVA_HOME%\bin\javah" -jni -classpath %CLASSPATH% android.speech.recognition.impl.LoggerImpl
"%JAVA_HOME%\bin\javah" -jni -classpath %CLASSPATH% android.speech.recognition.impl.System

