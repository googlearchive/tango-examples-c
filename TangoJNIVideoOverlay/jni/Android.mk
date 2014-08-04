LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libtango-prebuilt
LOCAL_SRC_FILES := ../../tango_service/libtango_api_client.so
LOCAL_EXPORT_C_INCLUDES := ../tango_service/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libtango-native-jni
LOCAL_SHARED_LIBRARIES := libtango-prebuilt
LOCAL_SRC_FILES := tango_video_overlay.c
LOCAL_LDLIBS    := -llog -lGLESv2
include $(BUILD_SHARED_LIBRARY)