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
  depth_data_buffer_ = new float[kMaxVertCount * 3];
  depth_buffer_size_ = kMaxVertCount * 3;
  lib_version_ = new char[26];
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
  struct timeval time;
  gettimeofday(&time, NULL);
  float current_frame_time = (float)((time.tv_sec * 1000) + (time.tv_usec / 1000));
  TangoData::GetInstance().depth_fps = 1000.0f/
    (current_frame_time - TangoData::GetInstance().previous_frame_time_);
  TangoData::GetInstance().previous_frame_time_ = current_frame_time;
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
  // Allocate a TangoConfig object.
  if ((config_ = TangoConfig_alloc()) == NULL) {
    LOGE("TangoService_allocConfig(): Failed");
    return false;
  }

  // Get the default TangoConfig.
  if (TangoService_getConfig(TANGO_CONFIG_DEFAULT, config_) != TANGO_SUCCESS) {
    LOGE("TangoService_getConfig(): Failed");
    return false;
  }

  // Enable depth.
  if (TangoConfig_setBool(config_, "config_enable_depth", true) != TANGO_SUCCESS) {
    LOGI("config_enable_depth Failed");
    return false;
  }
  
  // Disable motion tracking.
  if (TangoConfig_setBool(config_, "config_enable_motion_tracking", false) != TANGO_SUCCESS) {
    LOGI("config_disable_motion_tracking Failed");
    return false;
  }

  // Attach the onXYZijAvailable callback.
  if (TangoService_connectOnXYZijAvailable(onXYZijAvailable) != TANGO_SUCCESS) {
    LOGI("TangoService_connectOnXYZijAvailable(): Failed");
    return false;
  }

  return true;
}

bool TangoData::LockConfig() {
  // Lock in this configuration.
  if (TangoService_lockConfig(config_) != TANGO_SUCCESS) {
    LOGE("TangoService_lockConfig(): Failed");
    return false;
  }
  return true;
}

bool TangoData::UnlockConfig() {
  // Unlock current configuration.
  if (TangoService_unlockConfig() != TANGO_SUCCESS) {
    LOGE("TangoService_unlockConfig(): Failed");
    return false;
  }
  return true;
}

/// Connect to the Tango Service.
/// Note: connecting Tango service will start the motion
/// tracking automatically.
bool TangoData::Connect() {
  if (TangoService_connect(nullptr) != TANGO_SUCCESS) {
    LOGE("TangoService_connect(): Failed");
    return false;
  }
  return true;
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

void TangoData::SetDepthBufferSize(int size) {
  depth_buffer_size_ = size;
}

char* TangoData::GetVersonString() {
  TangoConfig_getString(config_, "tango_service_library_version", lib_version_, 26);
  return lib_version_;
}

TangoData::~TangoData() {
  delete[] depth_data_buffer_;
  delete[] lib_version_;
}
