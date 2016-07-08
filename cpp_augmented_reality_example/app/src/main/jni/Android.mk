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
LOCAL_PATH := $(call my-dir)
PROJECT_ROOT_FROM_JNI:= ../../../../..
PROJECT_ROOT:= $(call my-dir)/../../../../..

include $(CLEAR_VARS)
LOCAL_MODULE    := libcpp_augmented_reality_example
LOCAL_SHARED_LIBRARIES := tango_client_api tango_support_api
LOCAL_STATIC_LIBRARIES := png
LOCAL_CFLAGS    := -std=c++11

LOCAL_SRC_FILES := augmented_reality_app.cc \
                   jni_interface.cc \
                   scene.cc \
                   tango_event_data.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/bounding_box.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/camera.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/conversions.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/drawable_object.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/frustum.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/gesture_camera.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/line.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/meshes.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/shaders.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/tango_gl.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/texture.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/transform.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/util.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/video_overlay.cc

LOCAL_C_INCLUDES := $(PROJECT_ROOT)/tango_gl/include \
                    $(PROJECT_ROOT)/third_party/glm/ \
                    $(PROJECT_ROOT)/third_party/libpng/include/

LOCAL_LDLIBS    := -llog -lGLESv2 -L$(SYSROOT)/usr/lib -lz -landroid
include $(BUILD_SHARED_LIBRARY)

$(call import-add-path, $(PROJECT_ROOT))
$(call import-add-path, $(PROJECT_ROOT)/third_party)
$(call import-module,libpng)
$(call import-module,tango_client_api)
$(call import-module,tango_support_api)
