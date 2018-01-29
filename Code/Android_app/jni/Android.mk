LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OPENCV_LIB_TYPE:=STATIC
OPENCV_CAMERA_MODULES:=off

include /home/shashwat/NVPACK/OpenCV-android-sdk/sdk/native/jni/OpenCV.mk

LOCAL_MODULE    := jni_imgPros
LOCAL_SRC_FILES := jni_imgPros.cpp
LOCAL_LDLIBS += -llog -ldl

include $(BUILD_SHARED_LIBRARY)