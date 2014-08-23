/*
 * Copyright 2014 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libtango-prebuilt
LOCAL_SRC_FILES := ../../tango_service/libtango_client_api.so
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