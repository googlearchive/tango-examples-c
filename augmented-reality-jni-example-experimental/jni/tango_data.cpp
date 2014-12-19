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
    : tango_position(glm::vec3(0.0f, 0.0f, 0.0f)),
      tango_rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
      config_(nullptr) {}

// This is called when new pose updates become available. Pair was set to start-
// of-service with respect to ADF frame, which will be available only once
// localized against an ADF. Use this function to check localization status, and
// use GetPoseAtTime to get the current pose.
static void onPoseAvailable(void*, const TangoPoseData* pose) {
  pthread_mutex_lock(&TangoData::GetInstance().pose_mutex);
  // Update Tango localization status.
  if (pose->status_code == TANGO_POSE_VALID) {
    TangoData::GetInstance().is_localized = true;
  } else {
    TangoData::GetInstance().is_localized = false;
  }
  pthread_mutex_unlock(&TangoData::GetInstance().pose_mutex);
}

// Tango event callback.
static void onTangoEvent(void*, const TangoEvent* event) {
  pthread_mutex_lock(&TangoData::GetInstance().event_mutex);
  // Update the status string for debug display.
  std::stringstream string_stream;
  string_stream << event->event_key << ": " << event->event_value;
  TangoData::GetInstance().event_string = string_stream.str();
  pthread_mutex_unlock(&TangoData::GetInstance().event_mutex);
}

// Get status string based on the pose status code.
const char* TangoData::getStatusStringFromStatusCode(
    TangoPoseStatusType status) {
  const char* ret_string;
  switch (status) {
    case TANGO_POSE_INITIALIZING:
      ret_string = "Initializing";
      break;
    case TANGO_POSE_VALID:
      ret_string = "Valid";
      break;
    case TANGO_POSE_INVALID:
      ret_string = "Invalid";
      break;
    case TANGO_POSE_UNKNOWN:
      ret_string = "Unknown";
      break;
    default:
      ret_string = "Status_Code_Invalid";
      break;
  }
  if (static_cast<int>(status) < 3) {
    status_count[static_cast<int>(status)] += 1;
  }
  return ret_string;
}

TangoErrorType TangoData::Initialize(JNIEnv* env, jobject activity) {
  // Initialize Tango Service.
  // The initialize function perform API and Tango Service version check,
  // the there is a mis-match between API and Tango Service version, the
  // function will return TANGO_INVALID.
  return TangoService_initialize(env, activity);
}

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
    LOGE("config_enable_auto_recovery(): Failed");
    return false;
  }

  // Get library version string from service.
  TangoConfig_getString(config_, "tango_service_library_version",
                        const_cast<char*>(lib_version_string.c_str()),
                        kVersionStringLength);

  // Setting up the start of service to ADF frame for the onPoseAvailable
  // callback,
  // it will check the localization status.
  TangoCoordinateFramePair pair;
  pair.base = TANGO_COORDINATE_FRAME_AREA_DESCRIPTION;
  pair.target = TANGO_COORDINATE_FRAME_START_OF_SERVICE;

  // Attach onPoseAvailable callback.
  // The callback will be called after the service is connected.
  if (TangoService_connectOnPoseAvailable(1, &pair, onPoseAvailable) !=
      TANGO_SUCCESS) {
    LOGE("TangoService_connectOnPoseAvailable(): Failed");
    return false;
  }

  // Attach onEventAvailable callback.
  // The callback will be called after the service is connected.
  if (TangoService_connectOnTangoEvent(onTangoEvent) != TANGO_SUCCESS) {
    LOGE("TangoService_connectOnTangoEvent(): Failed");
    return false;
  }

  // Load the most recent ADF.
  char* uuid_list;

  // uuid_list will contain a comma separated list of UUIDs.
  if (TangoService_getAreaDescriptionUUIDList(&uuid_list) != TANGO_SUCCESS) {
    LOGI("TangoService_getAreaDescriptionUUIDList");
  }

  // Parse the uuid_list to get the individual uuids.
  if (uuid_list != NULL && uuid_list[0] != '\0') {
    std::vector<std::string> adf_list;

    char* parsing_char;
    char* saved_ptr;
    parsing_char = strtok_r(uuid_list, ",", &saved_ptr);
    while (parsing_char != NULL) {
      std::string s = std::string(parsing_char);
      adf_list.push_back(s);
      parsing_char = strtok_r(NULL, ",", &saved_ptr);
    }

    int list_size = adf_list.size();
    if (list_size == 0) {
      LOGE("List size is 0");
      return false;
    }
    cur_uuid = adf_list[list_size - 1];
    if (TangoConfig_setString(config_, "config_load_area_description_UUID",
                              adf_list[list_size - 1].c_str()) !=
        TANGO_SUCCESS) {
      LOGE("config_load_area_description_uuid Failed");
      return false;
    } else {
      LOGI("Load ADF: %s", adf_list[list_size - 1].c_str());
    }
  } else {
    LOGE("No area description file available, no file loaded.");
  }
  is_localized = false;
  return true;
}

bool TangoData::GetPoseAtTime() {
  // Set the reference frame pair after connect to service.
  // Currently the API will set this set below as default.
  TangoPoseData pose;

  // Device to Start of Service frames pair.
  TangoCoordinateFramePair d_to_ss_pair;
  d_to_ss_pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  d_to_ss_pair.target = TANGO_COORDINATE_FRAME_DEVICE;

  // Device to ADF frames pair.
  TangoCoordinateFramePair d_to_adf_pair;
  d_to_adf_pair.base = TANGO_COORDINATE_FRAME_AREA_DESCRIPTION;
  d_to_adf_pair.target = TANGO_COORDINATE_FRAME_DEVICE;

  const char* frame_pair = "Target->Device, Base->Start: ";
  // Before localized, use device to start of service frame pair,
  // after localized, use device to ADF frame pair.
  if (!is_localized) {
    // Should use timestamp, but currently updateTexture() only returns
    // 0 for timestamp, if set to 0.0, the most
    // recent pose estimate for the target-base pair will be returned.
    if (TangoService_getPoseAtTime(0.0, d_to_ss_pair, &pose) != TANGO_SUCCESS)
    return false;
  } else {
    frame_pair = "Target->Device, Base->ADF: ";
    if (TangoService_getPoseAtTime(0.0, d_to_adf_pair, &pose) != TANGO_SUCCESS)
      return false;
  }
  tango_position =
      glm::vec3(pose.translation[0], pose.translation[1], pose.translation[2]);
  tango_rotation = glm::quat(pose.orientation[3], pose.orientation[0],
                             pose.orientation[1], pose.orientation[2]);
  std::stringstream string_stream;
  string_stream.setf(std::ios_base::fixed, std::ios_base::floatfield);
  string_stream.precision(2);
  string_stream << "Tango system event: " << event_string << "\n" << frame_pair
                << "\n"
                << "  status: "
                << getStatusStringFromStatusCode(pose.status_code)
                << ", count: " << status_count[pose.status_code]
                << ", timestamp(ms): " << timestamp << ", position(m): ["
                << pose.translation[0] << ", " << pose.translation[1] << ", "
                << pose.translation[2] << "]"
                << ", orientation: [" << pose.orientation[0] << ", "
                << pose.orientation[1] << ", " << pose.orientation[2] << ", "
                << pose.orientation[3] << "]\n"
                << "Color Camera Intrinsics(px):\n"
                << "  width: " << cc_width << ", height: " << cc_height
                << ", fx: " << cc_fx << ", fy: " << cc_fy;
  pose_string = string_stream.str();
  return true;
}

bool TangoData::GetExtrinsics() {
  // Retrieve the Extrinsic
  TangoPoseData poseData;
  TangoCoordinateFramePair pair;
  pair.base = TANGO_COORDINATE_FRAME_IMU;
  pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  if (TangoService_getPoseAtTime(0.0, pair, &poseData) != TANGO_SUCCESS) {
    LOGE("TangoService_getPoseAtTime(): Failed");
    return false;
  }
  d_to_imu_position =
      glm::vec3(poseData.translation[0], poseData.translation[1],
                poseData.translation[2]);
  d_to_imu_rotation =
      glm::quat(poseData.orientation[3], poseData.orientation[0],
                poseData.orientation[1], poseData.orientation[2]);

  pair.target = TANGO_COORDINATE_FRAME_CAMERA_COLOR;
  if (TangoService_getPoseAtTime(0.0, pair, &poseData) != TANGO_SUCCESS) {
    LOGE("TangoService_getPoseAtTime(): Failed");
    return false;
  }
  c_to_imu_position =
      glm::vec3(poseData.translation[0], poseData.translation[1],
                poseData.translation[2]);
  c_to_imu_rotation =
      glm::quat(poseData.orientation[3], poseData.orientation[0],
                poseData.orientation[1], poseData.orientation[2]);
  return true;
}

bool TangoData::GetIntrinsics() {
  // Retrieve the Intrinsic
  TangoCameraIntrinsics ccIntrinsics;
  if (TangoService_getCameraIntrinsics(TANGO_CAMERA_COLOR, &ccIntrinsics)
      != TANGO_SUCCESS) {
    LOGE("TangoService_getCameraIntrinsics(): Failed");
    return false;
  }

  // Color camera's image plane width.
  cc_width = ccIntrinsics.width;
  // Color camera's image plane height.
  cc_height = ccIntrinsics.height;
  // Color camera's x axis focal length.
  cc_fx = ccIntrinsics.fx;
  // Color camera's y axis focal length.
  cc_fy = ccIntrinsics.fy;
  // Principal point x coordinate on the image.
  cc_cx = ccIntrinsics.cx;
  // Principal point y coordinate on the image.
  cc_cy = ccIntrinsics.cy;
  for (int i = 0; i < 5; i++) {
    cc_distortion[i] = ccIntrinsics.distortion[i];
  }
  return true;
}

void TangoData::ConnectTexture(GLuint texture_id) {
  if (TangoService_connectTextureId(TANGO_CAMERA_COLOR, texture_id, nullptr,
                                    nullptr) == TANGO_SUCCESS) {
    LOGI("TangoService_connectTextureId(): Success!");
  } else {
    LOGE("TangoService_connectTextureId(): Failed!");
  }
}

void TangoData::UpdateColorTexture() {
  if (TangoService_updateTexture(TANGO_CAMERA_COLOR, &timestamp)
      != TANGO_SUCCESS) {
    LOGE("TangoService_updateTexture(): Failed");
  }
}

// Reset the Motion Tracking.
void TangoData::ResetMotionTracking() {
  TangoService_resetMotionTracking();
}

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
  TangoService_disconnect();
}
