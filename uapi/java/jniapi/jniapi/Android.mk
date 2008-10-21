# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# common settings for all UAPI builds, exports some variables for sub-makes
include $(UAPI_MAKE_DIR)/Makefile.defs

PUBLIC_HEADERS =

LOCAL_SRC_FILES:= \
	CWrapperDeviceSpeakerListener.cpp \
	CWrapperEmbeddedGrammarListener.cpp \
	CWrapperGrammarListener.cpp \
	CWrapperMediaFileReaderListener.cpp \
	CWrapperMediaFileWriterListener.cpp \
	CWrapperMicrophoneListener.cpp \
	CWrapperRecognizerListener.cpp \
	CWrapperSrecGrammarListener.cpp \
	CWrapperVoicetagListener.cpp \
	jniapi.cpp \
	JNIHelper.cpp \
	android_speech_recognition_impl_AudioStreamImpl.cpp \
	android_speech_recognition_impl_DeviceSpeakerImpl.cpp \
	android_speech_recognition_impl_EmbeddedGrammarImpl.cpp \
	android_speech_recognition_impl_EmbeddedRecognizerImpl.cpp \
	android_speech_recognition_impl_EntryImpl.cpp \
	android_speech_recognition_impl_GrammarImpl.cpp \
	android_speech_recognition_impl_LoggerImpl.cpp \
	android_speech_recognition_impl_MediaFileReaderImpl.cpp \
	android_speech_recognition_impl_MediaFileWriterImpl.cpp \
	android_speech_recognition_impl_MicrophoneImpl.cpp \
	android_speech_recognition_impl_NBestRecognitionResultImpl.cpp \
	android_speech_recognition_impl_SrecGrammarImpl.cpp \
	android_speech_recognition_impl_System.cpp \
	android_speech_recognition_impl_VoicetagItemImpl.cpp \
	android_speech_recognition_impl_WordItemImpl.cpp \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../../../cpp/api/include \
	$(LOCAL_PATH)/../../../cpp/grammar/include \
	$(LOCAL_PATH)/../../../cpp/recognizer/srec/include \
	$(LOCAL_PATH)/../../../cpp/utilities/include \
	$(JNI_H_INCLUDE)

LOCAL_CFLAGS := \
	-DJNI_EXPORTS \

LOCAL_CFLAGS += \
	$(UAPI_GLOBAL_DEFINES) \
	$(UAPI_GLOBAL_CPPFLAGS) \

LOCAL_WHOLE_STATIC_LIBRARIES := \
	libESR_Shared \
	libESR_Portable \
	libSR_AcousticModels \
	libSR_AcousticState \
	libSR_AudioIn \
	libSR_Core \
	libSR_EventLog \
	libSR_Grammar \
	libSR_G2P \
	libSR_Nametag \
	libSR_Recognizer \
	libSR_Semproc \
	libSR_Session \
	libSR_Vocabulary \
	libUAPI_audio \
	libUAPI_audiolinux \
	libUAPI_grammar \
	libUAPI_uapi \
	libUAPI_utilities \
	libUAPI_srec \

LOCAL_STATIC_LIBRARIES := \
	libzipfile \
	libunz \

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libhardware \
	libcutils \
	libmedia

ifneq ($(TARGET_SIMULATOR),true)
	LOCAL_SHARED_LIBRARIES += libdl
endif

LOCAL_MODULE:= libUAPI_jni

LOCAL_LDLIBS += \
	-lpthread \
	-ldl

include $(BUILD_SHARED_LIBRARY)
