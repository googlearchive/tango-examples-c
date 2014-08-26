LOCAL_PATH:= $(call my-dir)/..

include $(CLEAR_VARS)
LOCAL_MODULE := libtango-prebuilt
LOCAL_SRC_FILES := ../tango-service-sdk/libtango_client_api.so
LOCAL_EXPORT_C_INCLUDES := ../tango-service-sdk/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libtango-native-jni
LOCAL_SHARED_LIBRARIES := libtango-prebuilt libtangogl
LOCAL_CFLAGS    := -Werror -std=c++11
LOCAL_SRC_FILES := jni/tango_motion_tracking.cpp jni/tango_data.cpp ../tango-gl-renderer/camera.cpp ../tango-gl-renderer/gl_util.cpp ../tango-gl-renderer/drawable_object.cpp ../tango-gl-renderer/grid.cpp ../tango-gl-renderer/axis.cpp ../tango-gl-renderer/frustum.cpp ../tango-gl-renderer/trace.cpp
LOCAL_C_INCLUDES := ../tango-gl-renderer ../third-party/glm/glm
LOCAL_LDLIBS    := -llog -lGLESv2 -L$(SYSROOT)/usr/lib
include $(BUILD_SHARED_LIBRARY)