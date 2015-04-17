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

TangoData::TangoData() : config_(nullptr), timestamp(0.0) {}

TangoErrorType TangoData::Initialize(JNIEnv* env, jobject activity) {
  // Initialize Tango Service.
  // The initialize function perform API and Tango Service version check,
  // the there is a mis-match between API and Tango Service version, the
  // function will return TANGO_INVALID.
  return TangoService_initialize(env, activity);
}

bool TangoData::SetConfig() {
  // Get the default TangoConfig.
  // We get the default config first and change the config
  // flag as needed.
  config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (config_ == NULL) {
    LOGE("TangoService_getConfig(): Failed");
    return false;
  }
  // Enable color camera.
  if (TangoConfig_setBool(config_, "config_enable_color_camera", true) !=
      TANGO_SUCCESS) {
    LOGE("config_enable_color_camera Failed");
    return false;
  }
  return true;
}

void TangoData::ConnectTexture(GLuint texture_id) {
  if (TangoService_connectTextureId(TANGO_CAMERA_COLOR, texture_id, nullptr,
                                    nullptr) != TANGO_SUCCESS) {
    LOGE("TangoService_connectTextureId(): Failed");
  }
}

/// Connect to the Tango Service.
/// Note: connecting Tango service will start the motion
/// tracking automatically.
bool TangoData::Connect() {
  if (TangoService_connect(nullptr, config_) != TANGO_SUCCESS) {
    LOGE("TangoService_connect(): Failed");
    return false;
  }
  return true;
}

void TangoData::UpdateColorTexture() {
  // TangoService_updateTexture() updates target camera's
  // texture and timestamp.
  if (TangoService_updateTexture(TANGO_CAMERA_COLOR, &timestamp) !=
      TANGO_SUCCESS) {
    LOGE("TangoService_updateTexture(): Failed");
  }
}

void TangoData::Disconnect() {
  TangoConfig_free(config_);
  config_ = NULL;
  TangoService_disconnect();
}

TangoData::~TangoData() {}
