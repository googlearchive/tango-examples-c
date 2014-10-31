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

// Get status string based on the pose status code.
static const char* getStatusStringFromStatusCode(TangoPoseStatusType status) {
  const char* ret_string;
  switch (status) {
    case TANGO_POSE_INITIALIZING:
      ret_string = "initializing";
      break;
    case TANGO_POSE_VALID:
      ret_string = "valid";
      break;
    case TANGO_POSE_INVALID:
      ret_string = "invalid";
      break;
    case TANGO_POSE_UNKNOWN:
      ret_string = "unknown";
      break;
    default:
      ret_string = "status_code_invalid";
      break;
  }
  return ret_string;
}

// This callback function is called when new POSE updates become available.
static void onPoseAvailable(void*, const TangoPoseData* pose) {
  pthread_mutex_lock(&TangoData::GetInstance().pose_mutex);
  // Update Tango pose data.
  TangoData::GetInstance().tango_position =
      glm::vec3(pose->translation[0], pose->translation[1],
                pose->translation[2]);
  TangoData::GetInstance().tango_rotation =
      glm::quat(pose->orientation[3], pose->orientation[0],
                pose->orientation[1], pose->orientation[2]);

  // Update the current status and status count.
  // Status count is for counting how many frames happened in
  // this status.
  TangoData::GetInstance().cur_pose_status = pose->status_code;
  if(TangoData::GetInstance().prev_pose_status != pose->status_code) {
    TangoData::GetInstance().pose_status_count = 0;
  }
  TangoData::GetInstance().prev_pose_status = pose->status_code;
  ++TangoData::GetInstance().pose_status_count;

  // Update frame delta time.
  // The frame delta time should be around 30 ms since the
  // pose is updating on 30Hz.
  TangoData::GetInstance().frame_delta_time =
      (pose->timestamp - TangoData::GetInstance().prev_pose_timestamp) *
      kMeterToMillimeter;
  TangoData::GetInstance().prev_pose_timestamp = (float)pose->timestamp;

  stringstream string_stream;
  string_stream.setf(std::ios_base::fixed, std::ios_base::floatfield);
  string_stream.precision(3);
  string_stream << "status: "
                << getStatusStringFromStatusCode(pose->status_code)
                << ", count: " << TangoData::GetInstance().pose_status_count
                << ", delta time (ms): "
                << TangoData::GetInstance().frame_delta_time
                << ", position (m): [" << pose->translation[0] << ", "
                << pose->translation[1] << ", " << pose->translation[2] << "]"
                << ", quat: [" << pose->orientation[0] << ", "
                << pose->orientation[1] << ", " << pose->orientation[2] << ", "
                << pose->orientation[3] << "]";
  TangoData::GetInstance().pose_string = string_stream.str();
  pthread_mutex_unlock(&TangoData::GetInstance().pose_mutex);
}

// Tango event callback.
static void onTangoEvent(void*, const TangoEvent* event) {
  pthread_mutex_lock(&TangoData::GetInstance().event_mutex);
  // Update the status string for debug display.
  stringstream string_stream;
  string_stream << event->event_key << ": " << event->event_value;
  TangoData::GetInstance().event_string = string_stream.str();
  pthread_mutex_unlock(&TangoData::GetInstance().event_mutex);
}

TangoErrorType TangoData::Initialize(JNIEnv* env, jobject activity) {
  // Initialize Tango Service.
  // The initialize function perform API and Tango Service version check,
  // the there is a mis-match between API and Tango Service version, the
  // function will return TANGO_INVALID.
  return TangoService_initialize(env, activity);
}

// Set up Tango configuration handle.
bool TangoData::SetConfig(bool is_auto_recovery) {
  // Get the default TangoConfig.
  // We get the default config first and change the config
  // flag as needed.
  config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (config_ == NULL) {
    LOGE("TangoService_getConfig(): Failed");
    return false;
  }

  // Turn on auto recovery for motion tracking.
  // Note that the auto-recovery is on by default.
  if (TangoConfig_setBool(config_, "config_enable_auto_recovery",
                          is_auto_recovery) != TANGO_SUCCESS) {
    LOGE("config_enable_auto_recovery() failed");
    return false;
  }

  // Setting up the frame pair for the onPoseAvailable callback.
  TangoCoordinateFramePair pairs;
  pairs.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  pairs.target = TANGO_COORDINATE_FRAME_DEVICE;

  // Attach onPoseAvailable callback.
  // The callback will be called after the service is connected.
  if (TangoService_connectOnPoseAvailable(1, &pairs, onPoseAvailable) !=
      TANGO_SUCCESS) {
    LOGE("TangoService_connectOnPoseAvailable(): failed");
    return false;
  }

  // Attach onEventAvailable callback.
  // The callback will be called after the service is connected.
  if (TangoService_connectOnTangoEvent(onTangoEvent) != TANGO_SUCCESS) {
    LOGE("TangoService_connectOnTangoEvent(): Failed");
    return false;
  }

  // Get library version string from service.
  TangoConfig_getString(
      config_, "tango_service_library_version",
      const_cast<char*>(TangoData::GetInstance().lib_version_string.c_str()),
      kVersionStringLength);
  return true;
}

// Reset the Motion Tracking.
void TangoData::ResetMotionTracking() { TangoService_resetMotionTracking(); }

// Connect to Tango Service, service will start running, and
// POSE can be queried.
TangoErrorType TangoData::Connect() {
  return TangoService_connect(nullptr, config_);
}

// Disconnect Tango Service.
// Disconnect will disconnect all callback from Tango Service,
// after resume, the application should re-connect all callback
// and connect to service.
// Disconnect will also reset all configuration to default.
// Before disconnecting the service, the application is reponsible to
// free the config_ handle as well.
//
// When running two Tango applications, the first application needs to
// disconnect the service, so that second application could connect the
// service with a new configration handle. The disconnect function will
// reset the configuration each time it's being called.
void TangoData::Disconnect() {
  TangoConfig_free(config_);
  config_ = NULL;
  TangoService_disconnect();
}
