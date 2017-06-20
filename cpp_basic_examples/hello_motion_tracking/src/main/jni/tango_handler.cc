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

#include <cstdlib>

#include "hello_motion_tracking/tango_handler.h"

namespace {
constexpr int kTangoCoreMinimumVersion = 9377;
void onPoseAvailable(void*, const TangoPoseData* pose) {
  LOGI("Position: %f, %f, %f. Orientation: %f, %f, %f, %f",
       pose->translation[0], pose->translation[1], pose->translation[2],
       pose->orientation[0], pose->orientation[1], pose->orientation[2],
       pose->orientation[3]);
}
}  // anonymous namespace.

namespace hello_motion_tracking {

void TangoHandler::OnCreate(JNIEnv* env, jobject caller_activity) {
  // Check the installed version of the TangoCore.  If it is too old, then
  // it will not support the most up to date features.
  int version = 0;
  TangoErrorType err =
      TangoSupport_getTangoVersion(env, caller_activity, &version);
  if (err != TANGO_SUCCESS || version < kTangoCoreMinimumVersion) {
    LOGE("TangoHandler::CheckVersion, Tango Core version is out of date.");
    std::exit(EXIT_SUCCESS);
  }
}

void TangoHandler::OnTangoServiceConnected(JNIEnv* env, jobject iBinder) {
  if (TangoService_setBinder(env, iBinder) != TANGO_SUCCESS) {
    LOGE("TangoHandler::ConnectTango, TangoService_setBinder error");
    std::exit(EXIT_SUCCESS);
  }

  // TANGO_CONFIG_DEFAULT is enabling Motion Tracking and disabling Depth
  // Perception.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE("TangoHandler::ConnectTango, TangoService_getConfig error.");
    std::exit(EXIT_SUCCESS);
  }

  // TangoCoordinateFramePair is used to tell Tango Service about the frame of
  // references that the applicaion would like to listen to.
  TangoCoordinateFramePair pair;
  pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  if (TangoService_connectOnPoseAvailable(1, &pair, onPoseAvailable) !=
      TANGO_SUCCESS) {
    LOGE("TangoHandler::ConnectTango, connectOnPoseAvailable error.");
    std::exit(EXIT_SUCCESS);
  }

  if (TangoService_connect(nullptr, tango_config_) != TANGO_SUCCESS) {
    LOGE("TangoHandler::ConnectTango, TangoService_connect error.");
    std::exit(EXIT_SUCCESS);
  }
}

void TangoHandler::OnPause() {
  TangoConfig_free(tango_config_);
  tango_config_ = nullptr;
  TangoService_disconnect();
}
}  // namespace hello_motion_tracking
