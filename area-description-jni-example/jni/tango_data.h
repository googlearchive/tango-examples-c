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

#ifndef TangoData_H
#define TangoData_H

#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>

#include "tango_client_api.h"
#include "tango-gl-renderer/gl_util.h"

const int kMeterToMillimeter = 1000;
const int kVersionStringLength = 27;
const float kSecondToMillisecond = 1000.0f;

using namespace std;

class TangoData {
 public:
  static TangoData& GetInstance() {
    static TangoData instance;
    return instance;
  }
  TangoData();

  void ResetData();

  TangoErrorType Initialize(JNIEnv* env, jobject activity);
  bool SetConfig(bool is_learning, bool is_load_adf);
  TangoErrorType Connect();
  bool ConnectCallbacks();
  void Disconnect();

  bool SaveADF();
  char* GetAllUUIDs();
  char* GetUUIDMetadataValue(const char* uuid, const char* key);
  void SetUUIDMetadataValue(const char* uuid, const char* key, int value_size,
                            const char* value);
  void DeleteADF(const char* uuid);

  void LogAllUUIDs();

  pthread_mutex_t pose_mutex;
  pthread_mutex_t event_mutex;

  // Index 0: device with respect to start frame.
  // Index 1: device with respect to adf frame.
  // Index 2: start with respect to adf frame.
  // Index 3: adf with respect to start frame.
  glm::vec3 tango_position[4];
  glm::quat tango_rotation[4];

  int prev_pose_status[4];
  int current_pose_status[4];
  float frame_delta_time[4];
  float prev_frame_time[4];
  int frame_count[4];

  bool is_relocalized;

  string cur_uuid;
  string event_string;
  string lib_version_string;

 private:
  TangoConfig config_;
};

#endif  // TangoData_H
