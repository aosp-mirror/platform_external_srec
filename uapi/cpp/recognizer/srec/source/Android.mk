# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# common settings for all UAPI builds, exports some variables for sub-makes
include $(UAPI_MAKE_DIR)/Makefile.defs

PUBLIC_HEADERS =

LOCAL_SRC_FILES:= \
	SrecGrammarImpl.cpp \
	SrecHelper.cpp \
	SrecRecognitionResultImpl.cpp \
	SrecRecognizerImpl.cpp \
	SrecVoicetagItem.cpp \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../../../api/include \
	$(LOCAL_PATH)/../../../audio/include \
	$(LOCAL_PATH)/../../../grammar/include \
	$(LOCAL_PATH)/../../../uapi/include \
	$(LOCAL_PATH)/../../../utilities/include \
	$(ASR_ROOT_DIR)/shared/include \
	$(ASR_ROOT_DIR)/portable/include \
	$(ASR_ROOT_DIR)/srec/include \
	$(ASR_ROOT_DIR)/srec/cr \
	$(ASR_ROOT_DIR)/srec/EventLog/include \
	$(ASR_ROOT_DIR)/srec/Session/include \
	$(ASR_ROOT_DIR)/srec/Semproc/include \
	$(ASR_ROOT_DIR)/srec/Recognizer/include \
	$(ASR_ROOT_DIR)/srec/Grammar/include \
	$(ASR_ROOT_DIR)/srec/Nametag/include \
	$(ASR_ROOT_DIR)/srec/Vocabulary/include \
	$(ASR_ROOT_DIR)/srec/AcousticModels/include \
	$(ASR_ROOT_DIR)/srec/AcousticState/include \

LOCAL_CFLAGS := \
	-DSREC_EXPORTS \
	-DUAPI_EXPORTS \

LOCAL_CFLAGS += \
	$(UAPI_GLOBAL_DEFINES) \
	$(UAPI_GLOBAL_CPPFLAGS) \

LOCAL_LDLIBS += \
	-lpthread \
	-ldl \

LOCAL_MODULE:= libUAPI_srec

include $(BUILD_STATIC_LIBRARY)
