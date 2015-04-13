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

#include "hello-tango-jni/tango_handler.h"

namespace hello_tango_jni {
static void onPoseAvailable(void*, const TangoPoseData* pose) {
  LOGI("Position: %f, %f, %f. Orientation: %f, %f, %f, %f",
       pose->translation[0], pose->translation[1], pose->translation[2],
       pose->orientation[0], pose->orientation[2], pose->orientation[3],
       pose->orientation[3]);
}

TangoHandler::TangoHandler() : tango_config_(nullptr) {}

TangoHandler::~TangoHandler() {
  tango_config_ = nullptr;
};

TangoErrorType TangoHandler::Initialize(JNIEnv* env, jobject activity) {
  return TangoService_initialize(env, activity);
}

TangoErrorType TangoHandler::SetupConfig() {
  // TANGO_CONFIG_DEFAULT is enabling Motion Tracking and disabling Depth
  // Perception.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    return TANGO_ERROR;
  }
  return TANGO_SUCCESS;
}

TangoErrorType TangoHandler::ConnectPoseCallback() {
  // TangoCoordinateFramePair is used to tell Tango Service about the frame of
  // references that the applicaion would like to listen to.
  TangoCoordinateFramePair pair;
  pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  return TangoService_connectOnPoseAvailable(1, &pair, onPoseAvailable);
}

TangoErrorType TangoHandler::ConnectService() {
  return TangoService_connect(nullptr, tango_config_);
}

void TangoHandler::DisconnectService() {
  TangoConfig_free(tango_config_);
  tango_config_ = nullptr;
  TangoService_disconnect();
}
}  // namespace hello_tango_jni
