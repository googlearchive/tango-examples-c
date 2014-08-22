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

#ifndef TANGO_DATA_H
#define TANGO_DATA_H
#define GLM_FORCE_RADIANS

#include <tango_client_api.h>

#include "gl_util.h"

class TangoData {
 public:
  static TangoData& GetInstance() {
    static TangoData instance;
    return instance;
  }
  TangoData();
  ~TangoData();

  bool Initialize();
  bool SetConfig();
  bool LockConfig();
  bool UnlockConfig();
  bool Connect();
  void Disconnect();

  float* GetDepthBuffer();
  void SetDepthBuffer(float *buffer);
  int GetDepthBufferSize();
  void SetDepthBufferSize(int size);

 private:
  TangoConfig* config_;
  double pointcloud_timestamp_;
  float* depth_data_buffer_;
  int depth_buffer_size_;
};

#endif  // TANGO_DATA_H
