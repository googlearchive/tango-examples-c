LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libtango-prebuilt
LOCAL_SRC_FILES := ../../tango_service/libtango_api_client.so
LOCAL_EXPORT_C_INCLUDES := ../tango_service/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libtango-native-jni
LOCAL_SHARED_LIBRARIES := libtango-prebuilt
LOCAL_CFLAGS    := -Werror -std=c++11
LOCAL_SRC_FILES := tango_pointcloud.cpp camera.cpp gl_util.cpp pointcloud.cpp axis.cpp drawable_object.cpp tango_data.cpp grid.cpp
LOCAL_C_INCLUDES := ../glm/glm
LOCAL_LDLIBS    := -llog -lGLESv2 -L$(SYSROOT)/usr/lib
include $(BUILD_SHARED_LIBRARY)