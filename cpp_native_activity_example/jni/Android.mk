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
LOCAL_PATH:= $(call my-dir)/..

include $(CLEAR_VARS)
LOCAL_MODULE    := libcpp_native_activity_example
LOCAL_SHARED_LIBRARIES := tango_client_api tango_support_api
LOCAL_CFLAGS    := -std=c++11
LOCAL_SRC_FILES := jni/native_activity.cc \
                   jni/native_activity_application.cc \
                   ../tango_gl/bounding_box.cc \
                   ../tango_gl/camera.cc \
                   ../tango_gl/conversions.cc\
                   ../tango_gl/cube.cc\
                   ../tango_gl/drawable_object.cc \
                   ../tango_gl/frustum.cc \
                   ../tango_gl/grid.cc \
                   ../tango_gl/line.cc \
                   ../tango_gl/mesh.cc \
                   ../tango_gl/shaders.cc \
                   ../tango_gl/transform.cc\
                   ../tango_gl/util.cc \
                   ../tango_gl/video_overlay.cc
LOCAL_C_INCLUDES := ../tango_gl/include \
                    ../third_party/glm/
LOCAL_LDLIBS    := -llog -lGLESv2 -landroid -lEGL -L$(SYSROOT)/usr/lib
LOCAL_STATIC_LIBRARIES := android_native_app_glue
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,tango_client_api)
$(call import-module,tango_support_api)
