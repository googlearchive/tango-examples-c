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

#include <tango-gl/conversions.h>
#include <tango_support_api.h>

#include "tango-motion-tracking/motion_tracking_app.h"

namespace {
// This function routes onPoseAvailable callbacks to the application object for
// handling.
//
// @param context, context will be a pointer to a MotionTrackingApplication
//        instance on which to call callbacks.
// @param pose, pose data to route to onPoseAvailable function.
void onPoseAvailableRouter(void* context, const TangoPoseData* pose) {
  tango_motion_tracking::MotiongTrackingApp* app =
      static_cast<tango_motion_tracking::MotiongTrackingApp*>(context);
  app->onPoseAvailable(pose);
}
}  // namespace

namespace tango_motion_tracking {
void MotiongTrackingApp::onPoseAvailable(const TangoPoseData* pose) {
  std::lock_guard<std::mutex> lock(pose_mutex_);
  callback_pose_ = *pose;
}

MotiongTrackingApp::MotiongTrackingApp() {}

MotiongTrackingApp::~MotiongTrackingApp() {
  if (tango_config_ != nullptr) {
    TangoConfig_free(tango_config_);
  }
}

bool MotiongTrackingApp::CheckTangoVersion(JNIEnv* env, jobject activity,
                                           int min_tango_version) {
  // Check the installed version of the TangoCore.  If it is too old, then
  // it will not support the most up to date features.
  int version;
  TangoErrorType err = TangoSupport_GetTangoVersion(env, activity, &version);
  return err == TANGO_SUCCESS && version >= min_tango_version;
}

int MotiongTrackingApp::TangoSetupConfig() {
  // Here, we'll configure the service to run in the way we'd want. For this
  // application, we'll start from the default configuration
  // (TANGO_CONFIG_DEFAULT). This enables basic motion tracking capabilities.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE("MotiongTrackingApp: Failed to get default config form");
    return TANGO_ERROR;
  }

  // Set auto-recovery for motion tracking as requested by the user.
  int ret =
      TangoConfig_setBool(tango_config_, "config_enable_auto_recovery", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MotiongTrackingApp: config_enable_auto_recovery() failed with error"
        "code: %d",
        ret);
    return ret;
  }

  return ret;
}

int MotiongTrackingApp::TangoConnectCallbacks() {
  // Setting up the frame pair for the onPoseAvailable callback.
  TangoCoordinateFramePair pairs;
  pairs.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  pairs.target = TANGO_COORDINATE_FRAME_DEVICE;

  // Attach onPoseAvailable callback.
  // The callback will be called after the service is connected.
  int ret =
      TangoService_connectOnPoseAvailable(1, &pairs, onPoseAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MotiongTrackingApp: Failed to connect to pose callback with error"
        "code: %d",
        ret);
    return ret;
  }
  return ret;
}

// Connect to Tango Service, service will start running, and
// pose can be queried.
bool MotiongTrackingApp::TangoConnect() {
  TangoErrorType ret = TangoService_connect(this, tango_config_);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MotiongTrackingApp: Failed to connect to the Tango service with"
        "error code: %d",
        ret);
    return false;
  }
  return true;
}

void MotiongTrackingApp::TangoDisconnect() {
  // When disconnecting from the Tango Service, it is important to make sure to
  // free your configuration object. Note that disconnecting from the service,
  // resets all configuration, and disconnects all callbacks. If an application
  // resumes after disconnecting, it must re-register configuration and
  // callbacks with the service.
  TangoConfig_free(tango_config_);
  tango_config_ = nullptr;
  TangoService_disconnect();
}

void MotiongTrackingApp::InitializeGLContent() { main_scene_.InitGLContent(); }

void MotiongTrackingApp::SetViewPort(int width, int height) {
  main_scene_.SetupViewPort(width, height);
}

void MotiongTrackingApp::Render() {
  // Query current pose data.
  TangoPoseData cur_pose;
  {
    std::lock_guard<std::mutex> lock(pose_mutex_);
    cur_pose = callback_pose_;
  }
  main_scene_.Render(cur_pose, screen_rotation_);
}

void MotiongTrackingApp::DeleteResources() { main_scene_.DeleteResources(); }

void MotiongTrackingApp::SetScreenRotation(int screen_rotation) {
  screen_rotation_ = screen_rotation;
}

// Initialize Tango.
bool MotiongTrackingApp::InitializeTango(JNIEnv* env, jobject iBinder) {
  TangoErrorType ret = TangoService_setBinder(env, iBinder);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MotiongTrackingApp: Failed to initialize Tango service with"
        "error code: %d",
        ret);
    return false;
  }
  return true;
}

}  // namespace tango_motion_tracking
