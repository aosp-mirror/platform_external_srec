
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_MODULE := srec-java-tests 

LOCAL_MODULE_TAGS := tests

include $(BUILD_JAVA_LIBRARY)

