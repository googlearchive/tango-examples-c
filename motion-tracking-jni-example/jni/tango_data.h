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

#ifndef Tango_Data_H
#define Tango_Data_H

#include <stdlib.h>
#include <string>
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
  bool SetConfig(bool isAutoReset);
  bool Connect();
  void Disconnect();
  
  void ResetMotionTracking();
  
  char* GetPoseDataString();
  char* GetEventString();
  char* GetVersionString();
  
  glm::vec3 tango_position;
  glm::quat tango_rotation;
  
  int pose_status_count;
  TangoPoseStatusType cur_pose_status;
  TangoPoseStatusType prev_pose_status;
  
  float frame_delta_time;
  double prev_pose_timestamp;

  char event_string[30];
  char lib_version[26];
private:
  TangoConfig* config_;
  char pose_string_[100];
};

#endif  // Tango_Data_H
