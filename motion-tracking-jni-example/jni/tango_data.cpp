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
      tango_position_(glm::vec3(0.0f, 0.0f, 0.0f)),
      tango_rotation_(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)) {
}

// This callback function is called when new POSE updates become available.
static void onPoseAvailable(const TangoPoseData* pose) {
  TangoData::GetInstance().SetTangoPosition(
      glm::vec3(pose->translation[0], pose->translation[1],
                pose->translation[2]));
  TangoData::GetInstance().SetTangoRotation(
      glm::quat(pose->orientation[3], pose->orientation[0],
                pose->orientation[1], pose->orientation[2]));

  TangoData::GetInstance().SetTangoPoseStatus(pose->status_code);

//  glm::vec3 euler = glm::eulerAngles(
//      glm::quat(pose->orientation[3], pose->orientation[0],
//                pose->orientation[1], pose->orientation[2]));
//  LOGI("%4.2f,%4.2f,%4.2f,%4.2f,%4.2f,%4.2f", pose->translation[0],
//       pose->translation[1], pose->translation[2], euler.x * 57.32f,
//       euler.y * 57.32f, euler.z * 57.32f);
}

bool TangoData::Initialize() {
  // Initialize Tango Service.
  if (TangoService_initialize() != 0) {
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
  if (TangoService_getConfig(TANGO_CONFIG_DEFAULT, config_) != 0) {
    LOGE("TangoService_getConfig(): Failed");
    return false;
  }

  //Attach onPoseAvailable callback.
  if (TangoService_connectOnPoseAvailable(onPoseAvailable) != 0) {
    LOGI("TangoService_connectOnPoseAvailable(): Failed");
    return false;
  }
  return true;
}

bool TangoData::LockConfig() {
  // Lock in this configuration.
  if (TangoService_lockConfig(config_) != 0) {
    LOGE("TangoService_lockConfig(): Failed");
    return false;
  }
  return true;
}

bool TangoData::UnlockConfig() {
  // Unlock current configuration.
  if (TangoService_unlockConfig() != 0) {
    LOGE("TangoService_unlockConfig(): Failed");
    return false;
  }
  return true;
}

// Connect to Tango Service, service will start running, and
// POSE can be queried.
bool TangoData::Connect() {
  if (TangoService_connect() != 0) {
    LOGE("TangoService_connect(): Failed");
    return false;
  }

  //Set the reference frame pair after connect to service.
  //Currently the API will set this set below as default.
  TangoCoordinateFramePair pairs;
  pairs.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  pairs.target = TANGO_COORDINATE_FRAME_DEVICE;
  if (TangoService_setPoseListenerFrames(1, &pairs) != 0) {
    LOGE("TangoService_setPoseListenerFrames(): Failed");
    return false;
  }
  return true;
}

void TangoData::Disconnect() {
  // Disconnect Tango Service.
  TangoService_disconnect();
}

glm::vec3 TangoData::GetTangoPosition() {
  return tango_position_;
}

glm::quat TangoData::GetTangoRotation() {
  return tango_rotation_;
}

char TangoData::GetTangoPoseStatus() {
  switch (tango_pose_status_) {
    case TANGO_POSE_INITIALIZING:
      return 1;
    case TANGO_POSE_VALID:
      return 2;
    case TANGO_POSE_INVALID:
      return 3;
    default:
      return 0;
      break;
  }
}

void TangoData::SetTangoPosition(glm::vec3 position) {
  tango_position_ = position;
}

void TangoData::SetTangoRotation(glm::quat rotation) {
  tango_rotation_ = rotation;
}

void TangoData::SetTangoPoseStatus(TangoPoseStatusType status) {
  tango_pose_status_ = status;
}
