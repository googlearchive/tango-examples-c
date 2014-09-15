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
  bool LockConfig();
  bool UnlockConfig();
  bool Connect();
  void ResetMotionTracking();
  void Disconnect();
  void ConnectTexture(GLuint texture_id);
  void UpdateColorTexture();

  glm::vec3 GetTangoPosition();
  glm::quat GetTangoRotation();
  char GetTangoPoseStatus();
  char* PoseToString();
  char* GetVersionString();
  void SetTangoPosition(glm::vec3 position);
  void SetTangoRotation(glm::quat rotation);
  void SetTangoPoseStatus(TangoPoseStatusType status);
  void UpdateEvent(int index);
  int statusCount[3];
  int eventCount[5];
  double timestamp;
  double prevTimestamp;
  bool isMTAutoReset;
  char eventString[30];
  char lib_version[26];
  //Extrinsic
  glm::vec3 dToIMU_position;
  glm::quat dToIMU_rotation;
  glm::vec3 cToIMU_position;
  glm::quat cToIMU_rotation;

  //Intrinsic
  int cc_Width;
  int cc_Height;
  double cc_fx;
  double cc_fy;
  double cc_cx;
  double cc_cy;
  double cc_distortion[5];


 private:
  glm::vec3 tango_position_;
  glm::quat tango_rotation_;
  TangoPoseStatusType tango_pose_status_;
  TangoConfig* config_;
  char poseString_[100];
};

#endif  // Tango_Data_H
