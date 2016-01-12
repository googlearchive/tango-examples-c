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

#include "tango-motion-tracking/motion_tracking_app.h"

namespace {
const int kVersionStringLength = 128;

// This function routes onPoseAvailable callbacks to the application object for
// handling.
//
// @param context, context will be a pointer to a MotionTrackingApplication
//        instance on which to call callbacks.
// @param pose, pose data to route to onPoseAvailable function.
void onPoseAvailableRouter(void* context, const TangoPoseData* pose) {
  using namespace tango_motion_tracking;
  MotiongTrackingApp* app = static_cast<MotiongTrackingApp*>(context);
  app->onPoseAvailable(pose);
}

// This function routes onTangoEvent callbacks to the application object for
// handling.
//
// @param context, context will be a pointer to a MotionTrackingApplication
//        instance on which to call callbacks.
// @param event, TangoEvent to route to onTangoEventAvailable function.
void onTangoEventAvailableRouter(void* context, const TangoEvent* event) {
  using namespace tango_motion_tracking;
  MotiongTrackingApp* app = static_cast<MotiongTrackingApp*>(context);
  app->onTangoEventAvailable(event);
}
}  // namespace

namespace tango_motion_tracking {
void MotiongTrackingApp::onPoseAvailable(const TangoPoseData* pose) {
  std::lock_guard<std::mutex> lock(pose_mutex_);
  pose_data_.UpdatePose(pose);
}

void MotiongTrackingApp::onTangoEventAvailable(const TangoEvent* event) {
  std::lock_guard<std::mutex> lock(tango_event_mutex_);
  tango_event_data_.UpdateTangoEvent(event);
}

MotiongTrackingApp::MotiongTrackingApp() {}

MotiongTrackingApp::~MotiongTrackingApp() {
  if (tango_config_ != nullptr) {
    TangoConfig_free(tango_config_);
  }
}

int MotiongTrackingApp::TangoInitialize(JNIEnv* env, jobject caller_activity) {
  // The first thing we need to do for any Tango enabled application is to
  // initialize the service. We'll do that here, passing on the JNI environment
  // and jobject corresponding to the Android activity that is calling us.
  return TangoService_initialize(env, caller_activity);
}

int MotiongTrackingApp::TangoSetupConfig(bool is_atuo_recovery) {
  // Here, we'll configure the service to run in the way we'd want. For this
  // application, we'll start from the default configuration
  // (TANGO_CONFIG_DEFAULT). This enables basic motion tracking capabilities.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE("MotiongTrackingApp: Failed to get default config form");
    return TANGO_ERROR;
  }

  // Set auto-recovery for motion tracking as requested by the user.
  int ret = TangoConfig_setBool(tango_config_, "config_enable_auto_recovery",
                                is_atuo_recovery);
  if (ret != TANGO_SUCCESS) {
    LOGE("MotiongTrackingApp: config_enable_auto_recovery() failed with error"
         "code: %d", ret);
    return ret;
  }

  // Get TangoCore version string from service.
  char tango_core_version[kVersionStringLength];
  ret = TangoConfig_getString(
      tango_config_, "tango_service_library_version",
      tango_core_version, kVersionStringLength);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MotiongTrackingApp: get tango core version failed with error"
        "code: %d",
        ret);
    return ret;
  }
  tango_core_version_string_ = tango_core_version;

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
    LOGE("MotiongTrackingApp: Failed to connect to pose callback with error"
         "code: %d", ret);
    return ret;
  }

  // Attach onEventAvailable callback.
  // The callback will be called after the service is connected.
  ret = TangoService_connectOnTangoEvent(onTangoEventAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE("MotiongTrackingApp: Failed to connect to event callback with error"
         "code: %d", ret);
    return ret;
  }
  return ret;
}

// Connect to Tango Service, service will start running, and
// pose can be queried.
int MotiongTrackingApp::TangoConnect() {
  TangoErrorType ret = TangoService_connect(this, tango_config_);
  if (ret != TANGO_SUCCESS) {
    LOGE("MotiongTrackingApp: Failed to connect to the Tango service with"
         "error code: %d", ret);
    return ret;
  }
  return ret;
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

void MotiongTrackingApp::TangoResetMotionTracking() {
  TangoService_resetMotionTracking();
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
    cur_pose = pose_data_.GetCurrentPoseData();
  }
  main_scene_.Render(cur_pose);
}

void MotiongTrackingApp::DeleteResources() { main_scene_.DeleteResources(); }

std::string MotiongTrackingApp::GetPoseString() {
  std::lock_guard<std::mutex> lock(pose_mutex_);
  return pose_data_.GetPoseDebugString();
}

std::string MotiongTrackingApp::GetEventString() {
  std::lock_guard<std::mutex> lock(tango_event_mutex_);
  return tango_event_data_.GetTangoEventString().c_str();
}

std::string MotiongTrackingApp::GetVersionString() {
  return tango_core_version_string_.c_str();
}

void MotiongTrackingApp::SetCameraType(
    tango_gl::GestureCamera::CameraType camera_type) {
  main_scene_.SetCameraType(camera_type);
}

void MotiongTrackingApp::OnTouchEvent(int touch_count,
                                      tango_gl::GestureCamera::TouchEvent event,
                                      float x0, float y0, float x1, float y1) {
  main_scene_.OnTouchEvent(touch_count, event, x0, y0, x1, y1);
}

}  // namespace tango_motion_tracking
