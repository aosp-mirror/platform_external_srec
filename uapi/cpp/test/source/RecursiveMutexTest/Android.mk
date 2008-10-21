# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# common settings for all ASR builds, exports some variables for sub-makes
include $(ASR_MAKE_DIR)/Makefile.defs

LOCAL_SRC_FILES:= \
	RecursiveMutexTest.cpp \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../include \
	$(LOCAL_PATH)/../../../api/include \
	$(LOCAL_PATH)/../../../uapi/include \
	$(LOCAL_PATH)/../../../utilities/include \

LOCAL_CFLAGS += \
	$(UAPI_GLOBAL_DEFINES) \
	$(UAPI_GLOBAL_CPPFLAGS) \

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libUAPI_jni \

LOCAL_MODULE:= RecursiveMutexTest

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
