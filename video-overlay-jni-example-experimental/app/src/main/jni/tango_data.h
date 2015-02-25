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

#ifndef VIDEO_OVERLAY_JNI_EXAMPLE_EXPERIMENTAL_TANGO_DATA_H_
#define VIDEO_OVERLAY_JNI_EXAMPLE_EXPERIMENTAL_TANGO_DATA_H_
#define GLM_FORCE_RADIANS

#include <sys/time.h>
#include <tango_client_api.h>

#include "tango-gl/util.h"

class TangoData {
 public:
  static TangoData& GetInstance() {
    static TangoData instance;
    return instance;
  }
  TangoData();
  ~TangoData();

  TangoErrorType Initialize(JNIEnv* env, jobject activity);
  bool SetConfig();
  bool Connect();
  void Disconnect();

  void ConnectTexture(GLuint texture_id);
  void UpdateColorTexture();

 private:
  TangoConfig config_;
  double timestamp;
};

#endif  // VIDEO_OVERLAY_JNI_EXAMPLE_EXPERIMENTAL_TANGO_DATA_H_
