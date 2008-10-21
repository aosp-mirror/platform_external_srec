# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# common settings for all UAPI builds, exports some variables for sub-makes
include $(UAPI_MAKE_DIR)/Makefile.defs

PUBLIC_HEADERS =

LOCAL_SRC_FILES:= \
	ConditionVariable.cpp \
	ConditionVariableImpl.cpp \
	DoublyLinkedList.cpp \
	FileASCII.cpp \
	LibraryLoader.cpp \
	Logger.cpp \
	LoggerImpl.cpp \
	Mutex.cpp \
	MutexImpl.cpp \
	Queue.cpp \
	RefCounted.cpp \
	RefCounter.cpp \
	Runnable.cpp \
	SmartProxy.cpp \
	System.cpp \
	Task.cpp \
	ThreadLocal.cpp \
	TimeInstant.cpp \
	WorkerQueueImpl.cpp \
	WorkerQueueFactory.cpp \
	WorkerQueueFactoryImpl.cpp \

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

LOCAL_MODULE:= libUAPI_utilities

include $(BUILD_STATIC_LIBRARY)
