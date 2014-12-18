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

#ifndef MOTION_TRACKING_JNI_EXAMPLE_TANGO_DATA_H_
#define MOTION_TRACKING_JNI_EXAMPLE_TANGO_DATA_H_

#include <pthread.h>
#include <sstream>
#include <stdlib.h>
#include <string>

#include "tango_client_api.h"
#include "tango-gl-renderer/gl_util.h"

const int kMeterToMillimeter = 1000;
const int kVersionStringLength = 27;

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
  bool ConnectCallbacks();
  void Disconnect();
  void ResetMotionTracking();

  pthread_mutex_t pose_mutex;
  pthread_mutex_t event_mutex;

  glm::vec3 tango_position;
  glm::quat tango_rotation;

  int pose_status_count;
  TangoPoseStatusType cur_pose_status;
  TangoPoseStatusType prev_pose_status;

  float frame_delta_time;
  float prev_pose_timestamp;

  std::string event_string;
  std::string lib_version_string;
  std::string pose_string;

 private:
  TangoConfig config_;
};

#endif  // MOTION_TRACKING_JNI_EXAMPLE_TANGO_DATA_H_
