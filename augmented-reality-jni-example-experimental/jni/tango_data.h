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

#include <pthread.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>

#include "tango_client_api.h"
#include "gl_util.h"

const int kVersionStringLength = 27;

using namespace std;

class TangoData {
 public:
  static TangoData& GetInstance() {
    static TangoData instance;
    return instance;
  }
  TangoData();

  TangoErrorType Initialize(JNIEnv* env, jobject activity);
  bool SetConfig(bool isAutoReset);
  TangoErrorType Connect();
  void Disconnect();
  void ResetMotionTracking();
  void ConnectTexture(GLuint texture_id);
  void UpdateColorTexture();
  bool GetPoseAtTime();
  bool GetIntrinsics();
  bool GetExtrinsics();
  const char* getStatusStringFromStatusCode(TangoPoseStatusType status);

  pthread_mutex_t pose_mutex;

  glm::vec3 tango_position;
  glm::quat tango_rotation;

  int status_count[3];
  double timestamp;
  string event_string;
  string lib_version_string;
  string pose_string;

  // Tango Extrinsic for device to IMU frame and
  // color camera to IMU frame.
  glm::vec3 d_to_imu_position;
  glm::quat d_to_imu_rotation;
  glm::vec3 c_to_imu_position;
  glm::quat c_to_imu_rotation;

  // Tango Intrinsic for color camera.
  int cc_width;
  int cc_height;
  double cc_fx;
  double cc_fy;
  double cc_cx;
  double cc_cy;
  double cc_distortion[5];

  // Localization status.
  bool is_localized;
  string cur_uuid;

 private:
  TangoConfig config_;
};

#endif  // Tango_Data_H

