LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := Nativeshell
LOCAL_SRC_FILES := Nativeshell.c
LOCAL_LDLIBS    := -lm -llog -landroid
include $(BUILD_SHARED_LIBRARY)
