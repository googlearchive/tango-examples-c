LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := libtango-native-jni
LOCAL_C_INCLUDES := ./tango_service/include/
LOCAL_SRC_FILES := tango_native.c
LOCAL_LDLIBS    := -llog -ltango_service_api
include $(BUILD_SHARED_LIBRARY)