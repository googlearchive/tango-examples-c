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
    : config_(nullptr) {
  is_learning_mode_enabled = false;
  for (int i = 0; i < 4; i++) {
    current_pose_status[i] = 0;
  }
}

// This callback function is called when new POSE updates become available.
static void onPoseAvailable(void* context, const TangoPoseData* pose) {
  int current_index = -1;
  // Set pose for device wrt start.
  if (pose->frame.base == TANGO_COORDINATE_FRAME_START_OF_SERVICE
      && pose->frame.target == TANGO_COORDINATE_FRAME_DEVICE) {
    current_index = 0;
  }

  // Set pose for device wrt ADF.
  else if (pose->frame.base == TANGO_COORDINATE_FRAME_AREA_DESCRIPTION
      && pose->frame.target == TANGO_COORDINATE_FRAME_DEVICE) {
    current_index = 1;
  }
  // Set pose for start wrt ADF.
  else if (pose->frame.base == TANGO_COORDINATE_FRAME_AREA_DESCRIPTION
      && pose->frame.target == TANGO_COORDINATE_FRAME_START_OF_SERVICE) {
    current_index = 2;
    LOGI("%d", (int)pose
         ->status_code);
  }
  // Set pose for device wrt previous pose.
  else if (pose->frame.base == TANGO_COORDINATE_FRAME_PREVIOUS_DEVICE_POSE
      && pose->frame.target == TANGO_COORDINATE_FRAME_DEVICE) {
    current_index = 3;
  } else {
    return;
  }
  TangoData::GetInstance().tango_position[current_index] = glm::vec3(
      pose->translation[0], pose->translation[1], pose->translation[2]);

  TangoData::GetInstance().tango_rotation[current_index] = glm::quat(
      pose->orientation[3], pose->orientation[0], pose->orientation[1],
      pose->orientation[2]);

  // Calculate delta frame time.
  TangoData::GetInstance().frame_delta_time[current_index] = pose->timestamp - TangoData::GetInstance().prev_frame_time[current_index];
  TangoData::GetInstance().prev_frame_time[current_index] = pose->timestamp;
  
  // Set/reset frame counter.
  if (pose->status_code == TANGO_POSE_VALID) {
    TangoData::GetInstance().frame_count[current_index]++;
  }
  else {
    TangoData::GetInstance().frame_count[current_index] = 0;
  }
  
  // Set pose status.
  TangoData::GetInstance().current_pose_status[current_index] = (int) pose
      ->status_code;
}

bool TangoData::Initialize() {
  // Initialize Tango Service.
  if (TangoService_initialize() != TANGO_SUCCESS) {
    LOGE("TangoService_initialize(): Failed");
    return false;
  }
  return true;
}

bool TangoData::SetConfig(bool is_learning, bool is_load_adf) {
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

  // Define is recording or loading a map.
  if (is_learning) {
    if (TangoConfig_setBool(config_, "config_enable_learning_mode", true)
        != TANGO_SUCCESS) {
      LOGI("config_enable_learning_mode Failed");
      return false;
    }
  }

  // If load ADF, load the most recent one.
  if (is_load_adf) {
    UUID_list uuid_list;
    TangoService_getAreaDescriptionUUIDList(&uuid_list);
    if (TangoConfig_setString(config_, "config_load_area_description_UUID",
                              uuid_list.uuid[uuid_list.count - 1].data)
        != TANGO_SUCCESS) {
      LOGI("config_load_area_description_uuid Failed");
      return false;
    }
    LOGI("Loaded map: %s", uuid_list.uuid[uuid_list.count - 1].data);
    memcpy(uuid_, uuid_list.uuid[uuid_list.count - 1].data, UUID_LEN * sizeof(char));
  }

  // Set listening pairs. Connenct pose callback.
  TangoCoordinateFramePair pairs[4] =
      {
          { TANGO_COORDINATE_FRAME_START_OF_SERVICE,
              TANGO_COORDINATE_FRAME_DEVICE }, {
              TANGO_COORDINATE_FRAME_AREA_DESCRIPTION,
              TANGO_COORDINATE_FRAME_DEVICE }, {
              TANGO_COORDINATE_FRAME_AREA_DESCRIPTION,
              TANGO_COORDINATE_FRAME_START_OF_SERVICE }, {
              TANGO_COORDINATE_FRAME_PREVIOUS_DEVICE_POSE,
              TANGO_COORDINATE_FRAME_DEVICE } };
  if (TangoService_connectOnPoseAvailable(4, pairs, onPoseAvailable)
      != TANGO_SUCCESS) {
    LOGI("TangoService_connectOnPoseAvailable(): Failed");
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

// Connect to Tango Service, service will start running, and
// POSE can be queried.
bool TangoData::Connect() {
  if (TangoService_connect(nullptr) != TANGO_SUCCESS) {
    LOGE("TangoService_connect(): Failed");
    return false;
  }
  LogAllUUIDs();
  return true;
}

char* TangoData::SaveADF() {
  UUID uuid;
  if (TangoService_saveAreaDescription(&uuid) != TANGO_SUCCESS) {
    LOGE("TangoService_saveAreaDescription(): Failed");
    return nullptr;
  }
  memcpy(uuid_, uuid.data, UUID_LEN * sizeof(char));
  LOGI("ADF Saved, uuid: %s", uuid_);
  return uuid_;
}

void TangoData::RemoveAllAdfs() {
  LOGI("Removing all ADFs");
  UUID_list uuid_list;
  TangoService_getAreaDescriptionUUIDList(&uuid_list);
  if (&uuid_list != nullptr) {
    TangoService_destroyAreaDescriptionUUIDList(&uuid_list);
  }
}

void TangoData::Disconnect() {
  // Disconnect Tango Service.
  TangoService_disconnect();
}

void TangoData::LogAllUUIDs() {
  UUID_list uuid_list;
  TangoService_getAreaDescriptionUUIDList(&uuid_list);
  LOGI("List of maps: ");
  for (int i = 0; i < uuid_list.count; i++) {
    LOGI("%d: %s", i, uuid_list.uuid[i].data);
  }
}
