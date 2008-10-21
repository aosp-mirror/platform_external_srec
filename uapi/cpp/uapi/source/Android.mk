# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# common settings for all UAPI builds, exports some variables for sub-makes
include $(UAPI_MAKE_DIR)/Makefile.defs

PUBLIC_HEADERS =

LOCAL_SRC_FILES:= \
	Codec.cpp \
	EmbeddedRecognizer.cpp \
	EmbeddedRecognizerImpl.cpp \
	NBestRecognitionResult.cpp \
	ParametersListener.cpp \
	RecognitionResult.cpp \
	Recognizer.cpp \
	RecognizerListener.cpp \
	ReturnCode.cpp \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../api/include \
	$(LOCAL_PATH)/../../audio/include \
	$(LOCAL_PATH)/../../audio/linux/include \
	$(LOCAL_PATH)/../../grammar/include \
	$(LOCAL_PATH)/../../uapi/include \
	$(LOCAL_PATH)/../../utilities/include \
	$(LOCAL_PATH)/../../utilities/linux/include \
	$(LOCAL_PATH)/../../../java/jniapi/jniapi \
	$(ASR_ROOT_DIR)/audio/AudioIn/UNIX/include \

LOCAL_CFLAGS := \
	-DUAPI_EXPORTS \

LOCAL_CFLAGS += \
	$(UAPI_GLOBAL_DEFINES) \
	$(UAPI_GLOBAL_CPPFLAGS) \

LOCAL_LDLIBS += \
	-lpthread \
	-ldl \

LOCAL_MODULE:= libUAPI_uapi

include $(BUILD_STATIC_LIBRARY)
