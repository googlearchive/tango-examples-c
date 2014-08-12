LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libtango-prebuilt
LOCAL_SRC_FILES := ../../TangoDevUpdate-0618/libs/libtango_api.so
LOCAL_EXPORT_C_INCLUDES := ../TangoDevUpdate-0618/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libtango-native-jni
LOCAL_SHARED_LIBRARIES := libtango-prebuilt 
LOCAL_SRC_FILES := tango_motion_tracking.cpp
LOCAL_C_INCLUDES := ../glm/glm
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -lGLESv2
LOCAL_ARM_MODE := arm
include $(BUILD_SHARED_LIBRARY)