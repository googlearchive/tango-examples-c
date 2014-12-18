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

#ifndef POINT_CLOUD_JNI_EXAMPLE_TANGO_DATA_H_
#define POINT_CLOUD_JNI_EXAMPLE_TANGO_DATA_H_
#define GLM_FORCE_RADIANS

#include <pthread.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <tango_client_api.h>
#include "tango-gl-renderer/gl_util.h"

const int kMeterToMillimeter = 1000;
const int kVersionStringLength = 27;
const float kSecondToMillisecond = 1000.0f;

// Opengl camera to color camera matrix.
const glm::mat4 oc_2_c_mat =
    glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
              -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
// Start service to opengl world matrix.
const glm::mat4 ss_2_ow_mat =
    glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
              0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

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
  TangoErrorType Connect();
  bool ConnectCallbacks();
  void Disconnect();

  bool SetupExtrinsicsMatrices();
  glm::mat4 GetOC2OWMat(bool is_depth);

  void UpdatePoseData();
  void UpdateXYZijData();

  pthread_mutex_t pose_mutex;
  pthread_mutex_t xyzij_mutex;
  pthread_mutex_t event_mutex;

  float* depth_buffer;
  uint32_t depth_buffer_size;
  bool is_xyzij_dirty;

  TangoPoseData cur_pose_data;
  TangoPoseData prev_pose_data;
  bool is_pose_dirty;

  int pose_status_count;
  float pose_frame_delta_time;

  float depth_average_length;
  float depth_frame_delta_time;

  uint32_t max_vertex_count;

  // Device to start service matrix.
  glm::mat4 d_2_ss_mat_depth;
  // Device to start service matrix.
  glm::mat4 d_2_ss_mat_motion;
  // Device to IMU matrix.
  glm::mat4 d_2_imu_mat;
  // Color camera to IMU matrix.
  glm::mat4 c_2_imu_mat;

  std::string event_string;
  std::string lib_version_string;
  std::string pose_string;

 private:
  TangoConfig config_;
};

#endif  // POINT_CLOUD_JNI_EXAMPLE_TANGO_DATA_H_
