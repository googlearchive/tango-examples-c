LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libtango-prebuilt
LOCAL_SRC_FILES := ../../tango-service-sdk/libtango_client_api.so
LOCAL_EXPORT_C_INCLUDES := ../tango-service-sdk/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libtango-native-jni
LOCAL_SHARED_LIBRARIES := libtango-prebuilt
LOCAL_SRC_FILES := tango_native.c
LOCAL_LDLIBS    := -llog
include $(BUILD_SHARED_LIBRARY)