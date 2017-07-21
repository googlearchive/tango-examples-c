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
LOCAL_MODULE    := libcpp_mesh_builder_example
LOCAL_SHARED_LIBRARIES := tango_client_api tango_3d_reconstruction tango_support
LOCAL_STATIC_LIBRARIES := png
LOCAL_CFLAGS    := -std=c++11

LOCAL_C_INCLUDES := $(PROJECT_ROOT)/tango-service-sdk/include/ \
                    $(PROJECT_ROOT)/tango_gl/include \
                    $(PROJECT_ROOT)/third_party/glm/ \
                    $(PROJECT_ROOT)/third_party/libpng/include/

LOCAL_SRC_FILES := jni_interface.cc \
                   mesh_builder_app.cc \
                   scene.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/src/camera.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/src/conversions.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/src/drawable_object.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/src/gesture_camera.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/src/grid.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/src/line.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/src/shaders.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/src/tango_gl.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/src/transform.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/src/texture.cc \
                   $(PROJECT_ROOT_FROM_JNI)/tango_gl/src/util.cc

LOCAL_LDLIBS    := -llog -lGLESv2 -L$(SYSROOT)/usr/lib -lz -landroid
include $(BUILD_SHARED_LIBRARY)

$(call import-add-path, $(PROJECT_ROOT))
$(call import-add-path, $(PROJECT_ROOT)/third_party)
$(call import-module,libpng)
$(call import-module,tango_client_api)
$(call import-module,tango_3d_reconstruction)
$(call import-module,tango_support)
