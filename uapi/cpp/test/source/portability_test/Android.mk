# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# common settings for all UAPI builds, exports some variables for sub-makes
include $(UAPI_MAKE_DIR)/Makefile.defs

PUBLIC_HEADERS =

LOCAL_SRC_FILES:= \
	PortabilityTest.cpp \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../include \
	$(LOCAL_PATH)/../../../api/include \
	$(LOCAL_PATH)/../../../uapi/include \
	$(LOCAL_PATH)/../../../utilities/include \

LOCAL_CFLAGS := \
	-DSKIP_TEST1 \
	-DSKIP_TEST2 \

LOCAL_CFLAGS += \
	$(UAPI_GLOBAL_DEFINES) \
	$(UAPI_GLOBAL_CPPFLAGS) \

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libUAPI_jni \

LOCAL_MODULE:= UAPI_PortabilityTest

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
