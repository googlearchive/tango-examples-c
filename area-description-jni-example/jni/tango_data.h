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

#ifndef TangoData_H
#define TangoData_H

#include <tango_client_api.h>
#include "gl_util.h"

class TangoData {
 public:
  static TangoData& GetInstance() {
    static TangoData instance;
    return instance;
  }
  TangoData();

  bool Initialize();
  bool SetConfig(bool is_learning, bool is_load_adf);
  bool LockConfig();
  bool UnlockConfig();
  bool Connect();
  void Disconnect();

  bool SaveADF();
  void RemoveAllAdfs();

  void LogAllUUIDs();

  // 0: device_wrt_start
  // 1: device_wrt_adf
  // 2: start_wrt_adf
  // 3: adf_wrt_start
  glm::vec3 tango_position_[4];
  glm::quat tango_rotation_[4];
  float current_timestamp_[4];
  int current_pose_status_[4];

  bool is_learning_mode_enabled;
  bool is_relocalized;

  char uuid_[36];

 private:
  TangoConfig* config_;
};

#endif  // TangoData_H
