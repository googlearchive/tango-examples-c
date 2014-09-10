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
static void onPoseAvailable(void* context, const TangoPoseData* pose) {
  TangoData::GetInstance().SetTangoPosition(
      glm::vec3(pose->translation[0], pose->translation[1],
                pose->translation[2]));
  TangoData::GetInstance().SetTangoRotation(
      glm::quat(pose->orientation[3], pose->orientation[0],
                pose->orientation[1], pose->orientation[2]));

  TangoData::GetInstance().SetTangoPoseStatus(pose->status_code);
  TangoData::GetInstance().prevTimestamp=TangoData::GetInstance().timestamp;
  TangoData::GetInstance().timestamp = pose->timestamp;
  //LOGI("%d", (int) pose->status_code);
  //  glm::vec3 euler = glm::eulerAngles(
  //      glm::quat(pose->orientation[3], pose->orientation[0],
  //                pose->orientation[1], pose->orientation[2]));
  //  LOGI("%4.2f,%4.2f,%4.2f,%4.2f,%4.2f,%4.2f", pose->translation[0],
  //       pose->translation[1], pose->translation[2], euler.x * 57.32f,
  //       euler.y * 57.32f, euler.z * 57.32f);
}

// Tango event callback.
static void onTangoEvent(void* context, const TangoEvent* event) {
  if (strstr(event->description, "Exposed") != 0) {
    strncpy( TangoData::GetInstance().eventString,event->description, 30);
    LOGI("JEvent: %s", event->description);
  }
  if (strstr(event->description, "FOVOver") != 0) {
    TangoData::GetInstance().UpdateEvent(0);
    return;
  }
  if (strstr(event->description, "FOVUnder") != 0) {
    TangoData::GetInstance().UpdateEvent(1);
    return;
  }
  if (strstr(event->description, "ColorOver") != 0) {
    TangoData::GetInstance().UpdateEvent(2);
    return;
  }
  if (strstr(event->description, "ColorUnder") != 0) {
    TangoData::GetInstance().UpdateEvent(3);
    return;
  }
  if (strstr(event->description, "Too") != 0) {
    TangoData::GetInstance().UpdateEvent(4);
    return;
  }
}

bool TangoData::Initialize() {
  // Initialize Tango Service.
  if (TangoService_initialize() != TANGO_SUCCESS) {
    LOGE("TangoService_initialize(): Failed");
    return false;
  }
  return true;
}

bool TangoData::SetConfig(bool isAutoReset) {
  isMTAutoReset = isAutoReset;
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

  if (TangoConfig_setBool(config_, "config_enable_auto_reset", isMTAutoReset)
      != TANGO_SUCCESS) {
    if (isMTAutoReset)
      LOGE("Set to Auto Reset Failed");
    else
      LOGE("Set Manual Reset Failed");
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

void TangoData::ResetMotionTracking() {
  // Manually reset the Motion Tracking
  TangoService_resetMotionTracking();
}

// Connect to Tango Service, service will start running, and
// POSE can be queried.
bool TangoData::Connect() {
  if (TangoService_connect(nullptr) != TANGO_SUCCESS) {
    LOGE("TangoService_connect(): Failed");
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
  return tango_pose_status_;
}

void TangoData::SetTangoPosition(glm::vec3 position) {
  tango_position_ = position;
}

void TangoData::SetTangoRotation(glm::quat rotation) {
  tango_rotation_ = rotation;
}

void TangoData::SetTangoPoseStatus(TangoPoseStatusType status) {
  tango_pose_status_ = status;
  if ((int) status < 3)
    statusCount[(int) status]++;
}

void TangoData::UpdateEvent(int index) {
  eventCount[index]++;
}

char* TangoData::PoseToString() {
  sprintf(
      poseString_,
      "Status Count (frames):  Initializing:%d   Valid:%d   Invalid:%d\nPosition (m):  x:%4.2f   y:%4.2f   z:%4.2f\nRotation (quat):  x:%4.3f   y:%4.3f   z:%4.3f   w:%4.3f\nFrame Delta Time (ms):  %f\n\nFOver:%d\nFUnder:%d\nCOver:%d\nCUnder:%d\nTooFewFeature:%d",
      statusCount[0], statusCount[1], statusCount[2], tango_position_.x,
      tango_position_.y, tango_position_.z, tango_rotation_.x,
      tango_rotation_.y, tango_rotation_.z, tango_rotation_.w, timestamp-prevTimestamp,eventCount[0], eventCount[1], eventCount[2],eventCount[3], eventCount[4]);
  return poseString_;
}
