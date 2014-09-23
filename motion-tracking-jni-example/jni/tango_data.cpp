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
TangoData::TangoData()
    : config_(nullptr),
      tango_position(glm::vec3(0.0f, 0.0f, 0.0f)),
      tango_rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)) {
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
}

// Tango event callback.
static void onTangoEvent(void* context, const TangoEvent* event) {
  sprintf(TangoData::GetInstance().event_string,
          "%s: %s", event->event_key, event->event_value);
}

bool TangoData::Initialize() {
  // Initialize Tango Service.
  if (TangoService_initialize() != TANGO_SUCCESS) {
    LOGE("TangoService_initialize(): Failed");
    return false;
  }
  return true;
}

bool TangoData::SetConfig(bool is_auto_reset) {
  // Get the default TangoConfig.
  config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (config_ == NULL) {
    LOGE("TangoService_getConfig(): Failed");
    return false;
  }

  if (TangoConfig_setBool(config_, "config_enable_auto_reset", is_auto_reset)
      != TANGO_SUCCESS) {
    LOGE("config_enable_auto_reset(): Failed");
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
  TangoConfig_getString(config_, "tango_service_library_version",TangoData::GetInstance().lib_version, 26);
  return true;
}

void TangoData::ResetMotionTracking() {
  // Manually reset the Motion Tracking
  TangoService_resetMotionTracking();
}

// Connect to Tango Service, service will start running, and
// POSE can be queried.
bool TangoData::Connect() {
  if (TangoService_connect(nullptr, config_) != TANGO_SUCCESS) {
    LOGE("TangoService_connect(): Failed");
    return false;
  }
  return true;
}

void TangoData::Disconnect() {
  // Disconnect Tango Service.
  TangoService_disconnect();
}

char* TangoData::GetVersionString() {
  return lib_version;
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
