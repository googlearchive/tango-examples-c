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

#include "tango_data.h"

// Temp hack.
static const int kMaxVertCount = 61440;

TangoData::TangoData() : config_(nullptr) , pointcloud_timestamp_(0.0f) {
  motion_pose = new TangoPoseData();
  
  depth_data_buffer_ = new float[kMaxVertCount * 3];
  depth_buffer_size_ = kMaxVertCount * 3;

  d_2_ss_mat_depth = glm::mat4(1.0f);
  d_2_imu_mat = glm::mat4(1.0f);
  c_2_imu_mat = glm::mat4(1.0f);
  
  float ss_2_ow_arr[16] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, -1.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };
  
  float oc_2_c_arr[16] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, -1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, -1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };
  
  memcpy(glm::value_ptr(ss_2_ow_mat), ss_2_ow_arr, sizeof(ss_2_ow_arr));
  memcpy(glm::value_ptr(oc_2_c_mat), oc_2_c_arr, sizeof(oc_2_c_arr));
}

/// Callback function when new XYZij data available, caller is responsible
/// for allocating the memory, and the memory will be released after the
/// callback function is over.
/// XYZij data updates in 5Hz.
static void onXYZijAvailable(void* context, const TangoXYZij* XYZ_ij) {
  float total_z = 0.0f;
  int vertices_count = XYZ_ij->xyz_count;
  for (int i = 0; i < vertices_count; i++) {
    total_z += XYZ_ij->xyz[i][2];
  }
  memcpy(TangoData::GetInstance().GetDepthBuffer(), XYZ_ij->xyz,
         vertices_count * 3 * sizeof(float));
  TangoData::GetInstance().SetDepthBufferSize(vertices_count * 3);
  TangoData::GetInstance().average_depth = total_z/(float)vertices_count;
  
  // Computing the callback delta time.
  TangoData::GetInstance().depth_frame_delta_time =
    (XYZ_ij->timestamp - TangoData::GetInstance().previous_frame_time_)*1000.0f;
  TangoData::GetInstance().previous_frame_time_ = XYZ_ij->timestamp;
  TangoData::GetInstance().GetPoseAtTime(XYZ_ij->timestamp);
}

// Tango event callback.
static void onTangoEvent(void* context, const TangoEvent* event) {
  sprintf(TangoData::GetInstance().event_string,
          "%s: %s", event->event_key, event->event_value);
}

// This callback function is called when new POSE updates become available.
static void onPoseAvailable(void* context, const TangoPoseData* pose) {
  TangoData::GetInstance().tango_position =
    glm::vec3(pose->translation[0], pose->translation[1],
              pose->translation[2]);
  TangoData::GetInstance().tango_rotation =
    glm::quat(pose->orientation[3], pose->orientation[0],
              pose->orientation[1], pose->orientation[2]);
  
  TangoData::GetInstance().cur_pose_status = pose->status_code;
  if(TangoData::GetInstance().prev_pose_status != pose->status_code) {
    TangoData::GetInstance().pose_status_count = 0;
  }
  TangoData::GetInstance().prev_pose_status = pose->status_code;
  TangoData::GetInstance().pose_status_count++;
  
  TangoData::GetInstance().frame_delta_time =
  TangoData::GetInstance().prev_pose_timestamp - pose->timestamp;
  TangoData::GetInstance().prev_pose_timestamp = pose->timestamp;
  
  TangoData::GetInstance().d_2_ss_mat_motion =
    glm::translate(glm::mat4(1.0f), TangoData::GetInstance().tango_position) *
    glm::mat4_cast(TangoData::GetInstance().tango_rotation);
}

bool TangoData::Initialize() {
  // Initialize Tango Service.
  if (TangoService_initialize() != TANGO_SUCCESS) {
    LOGE("TangoService_initialize(): Failed");
    return false;
  }
  return true;
}

bool TangoData::SetConfig() {
  // Get the default TangoConfig.
  config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (config_ == NULL) {
    LOGE("TangoService_getConfig(): Failed");
    return false;
  }

  // Enable depth.
  if (TangoConfig_setBool(config_, "config_enable_depth", true) != TANGO_SUCCESS) {
    LOGI("config_enable_depth Failed");
    return false;
  }
  
  // Disable motion tracking.
  if (TangoConfig_setBool(config_, "config_enable_motion_tracking", true) != TANGO_SUCCESS) {
    LOGI("config_disable_motion_tracking Failed");
    return false;
  }
  
  return true;
}

/// Connect to the Tango Service.
bool TangoData::Connect() {
  // Attach the onXYZijAvailable callback.
  if (TangoService_connectOnXYZijAvailable(onXYZijAvailable) != TANGO_SUCCESS) {
    LOGI("TangoService_connectOnXYZijAvailable(): Failed");
    return false;
  }
  
  //Set the reference frame pair after connect to service.
  //Currently the API will set this set below as default.
  TangoCoordinateFramePair pairs;
  pairs.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  pairs.target = TANGO_COORDINATE_FRAME_DEVICE;
  
  //Attach onPoseAvailable callback.
  if (TangoService_connectOnPoseAvailable(1, &pairs, onPoseAvailable)
      != TANGO_SUCCESS) {
    LOGI("TangoService_connectOnPoseAvailable(): Failed");
    return false;
  }
  
  // Set the event callback listener.
  if (TangoService_connectOnTangoEvent(onTangoEvent) != TANGO_SUCCESS) {
    LOGI("TangoService_connectOnTangoEvent(): Failed");
    return false;
  }
  
  if (TangoService_connect(nullptr, config_) != TANGO_SUCCESS) {
    LOGE("TangoService_connect(): Failed");
    return false;
  }
  SetExtrinsicsMatrics();
  return true;
}

void TangoData::GetPoseAtTime(double timestamp) {
  TangoCoordinateFramePair pairs;
  pairs.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  pairs.target = TANGO_COORDINATE_FRAME_DEVICE;
  TangoPoseData pose;
  if (TangoService_getPoseAtTime(timestamp, pairs, &pose) != TANGO_SUCCESS) {
    LOGE("TangoService_getPoseAtTime(): Failed");
  }
  glm::vec3 translation =
  glm::vec3(pose.translation[0], pose.translation[1],
            pose.translation[2]);
  glm::quat rotation =
  glm::quat(pose.orientation[3], pose.orientation[0],
            pose.orientation[1], pose.orientation[2]);
  
  d_2_ss_mat_depth =
    glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(rotation);
}

void TangoData::Disconnect() {
  // Disconnect application from Tango Service.
  TangoService_disconnect();
}

float* TangoData::GetDepthBuffer() {
  return depth_data_buffer_;
}

void TangoData::SetDepthBuffer(float* buffer) {
  depth_data_buffer_ = buffer;
}

int TangoData::GetDepthBufferSize() {
  return depth_buffer_size_;
}

glm::mat4 TangoData::GetOC2OWMat(bool is_depth_pose) {
  if (is_depth_pose) {
    return ss_2_ow_mat * d_2_ss_mat_depth * glm::inverse(d_2_imu_mat) * c_2_imu_mat * oc_2_c_mat;
  }
  else {
    return ss_2_ow_mat * d_2_ss_mat_motion * glm::inverse(d_2_imu_mat) * c_2_imu_mat * oc_2_c_mat;
  }
}

void TangoData::SetDepthBufferSize(int size) {
  depth_buffer_size_ = size;
}

char* TangoData::GetVersonString() {
  TangoConfig_getString(config_, "tango_service_library_version", lib_version, 26);
  return lib_version;
}

void TangoData::SetExtrinsicsMatrics() {
  TangoPoseData pose_data;
  TangoCoordinateFramePair frame_pair;
  glm::vec3 translation;
  glm::quat rotation;
  
  // Get device to imu matrix.
  frame_pair.base = TANGO_COORDINATE_FRAME_IMU;
  frame_pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  if (TangoService_getPoseAtTime(0.0, frame_pair, &pose_data) != TANGO_SUCCESS) {
    LOGE("TangoService_getPoseAtTime(): Failed");
  }
  translation = glm::vec3(pose_data.translation[0],
                          pose_data.translation[1],
                          pose_data.translation[2]);
  rotation = glm::quat(pose_data.orientation[3],
                       pose_data.orientation[0],
                       pose_data.orientation[1],
                       pose_data.orientation[2]);
  d_2_imu_mat = glm::translate(glm::mat4(1.0f), translation) *
    glm::mat4_cast(rotation);
  
  // Get color camera to imu matrix.
  frame_pair.base = TANGO_COORDINATE_FRAME_IMU;
  frame_pair.target = TANGO_COORDINATE_FRAME_CAMERA_COLOR;
  if (TangoService_getPoseAtTime(0.0, frame_pair, &pose_data) != TANGO_SUCCESS) {
    LOGE("TangoService_getPoseAtTime(: Failed");
  }
  translation = glm::vec3(pose_data.translation[0],
                          pose_data.translation[1],
                          pose_data.translation[2]);
  rotation = glm::quat(pose_data.orientation[3],
                       pose_data.orientation[0],
                       pose_data.orientation[1],
                       pose_data.orientation[2]);
  c_2_imu_mat = glm::translate(glm::mat4(1.0f), translation) *
    glm::mat4_cast(rotation);
}

char* TangoData::GetEventString() {
  return event_string;
}

char* TangoData::GetPoseDataString() {
  const char* status;
  switch (cur_pose_status) {
    case 0:
      status = "Initializing";
      break;
    case 1:
      status = "Valid";
      break;
    case 2:
      status = "Invalid";
      break;
    case 3:
      status = "Unknown";
      break;
    default:
      break;
  }
  
  sprintf(pose_string_,
          "Status:%s, Count:%d, Delta Time(ms): %4.3f, Pose(m): %4.3f, %4.3f, %4.3f, Quat: %4.3f, %4.3f, %4.3f, %4.3f",
          status, pose_status_count, frame_delta_time,
          tango_position[0], tango_position[1], tango_position[2],
          tango_rotation[0], tango_rotation[1], tango_rotation[2], tango_rotation[3]);
  
  return pose_string_;
}

TangoData::~TangoData() {
  delete[] depth_data_buffer_;
}
