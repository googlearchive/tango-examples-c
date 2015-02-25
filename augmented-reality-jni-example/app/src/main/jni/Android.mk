#
# Copyright 2014 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH:= $(call my-dir)
PROJECT_ROOT:= $(call my-dir)/../../../../..

include $(CLEAR_VARS)
LOCAL_MODULE := libtango-prebuilt
LOCAL_SRC_FILES := $(PROJECT_ROOT)/tango-service-sdk/libtango_client_api.so
LOCAL_EXPORT_C_INCLUDES := $(PROJECT_ROOT)/tango-service-sdk/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libaugmented_reality_jni_example
LOCAL_SHARED_LIBRARIES := libtango-prebuilt
LOCAL_CFLAGS    := -std=c++11

LOCAL_C_INCLUDES := $(PROJECT_ROOT)/tango-service-sdk/include/ \
                    $(PROJECT_ROOT)/tango-gl/include \
                    $(PROJECT_ROOT)/third-party/glm/

LOCAL_SRC_FILES := tango_augmented_reality.cpp \
                   tango_data.cpp \
                   marker.cpp \
                   $(PROJECT_ROOT)/tango-gl/ar_ruler.cpp \
                   $(PROJECT_ROOT)/tango-gl/axis.cpp \
                   $(PROJECT_ROOT)/tango-gl/camera.cpp \
                   $(PROJECT_ROOT)/tango-gl/conversions.cpp \
                   $(PROJECT_ROOT)/tango-gl/cube.cpp \
                   $(PROJECT_ROOT)/tango-gl/frustum.cpp \
                   $(PROJECT_ROOT)/tango-gl/grid.cpp \
                   $(PROJECT_ROOT)/tango-gl/polygon.cpp \
                   $(PROJECT_ROOT)/tango-gl/trace.cpp \
                   $(PROJECT_ROOT)/tango-gl/transform.cpp \
                   $(PROJECT_ROOT)/tango-gl/util.cpp \
                   $(PROJECT_ROOT)/tango-gl/video_overlay.cpp

LOCAL_LDLIBS    := -llog -lGLESv2 -L$(SYSROOT)/usr/lib
include $(BUILD_SHARED_LIBRARY)
