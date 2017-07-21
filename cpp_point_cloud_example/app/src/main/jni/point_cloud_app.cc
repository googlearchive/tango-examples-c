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
#include <tango_support.h>
#include <tango_transform_helpers.h>

#include "tango-point-cloud/point_cloud_app.h"

namespace {
const int kVersionStringLength = 128;
// The minimum Tango Core version required from this application.
constexpr int kTangoCoreMinimumVersion = 9377;

// This function routes onPointCloudAvailable callbacks to the application
// object for handling.
//
// @param context, context will be a pointer to a PointCloudApp
//        instance on which to call callbacks.
// @param pose, pose data to route to onPoseAvailable function.
void onPointCloudAvailableRouter(void* context,
                                 const TangoPointCloud* point_cloud) {
  tango_point_cloud::PointCloudApp* app =
      static_cast<tango_point_cloud::PointCloudApp*>(context);
  app->onPointCloudAvailable(point_cloud);
}
}  // namespace

namespace tango_point_cloud {
void PointCloudApp::onPointCloudAvailable(const TangoPointCloud* point_cloud) {
  TangoSupport_updatePointCloud(point_cloud_manager_, point_cloud);
}

PointCloudApp::PointCloudApp()
    : screen_rotation_(0),
      is_service_connected_(false),
      is_gl_initialized_(false) {}

PointCloudApp::~PointCloudApp() {
  if (tango_config_ != nullptr) {
    TangoConfig_free(tango_config_);
  }
  if (point_cloud_manager_ != nullptr) {
    TangoSupport_freePointCloudManager(point_cloud_manager_);
    point_cloud_manager_ = nullptr;
  }
}

void PointCloudApp::OnCreate(JNIEnv* env, jobject activity) {
  // Check the installed version of the TangoCore.  If it is too old, then
  // it will not support the most up to date features.
  int version;
  TangoErrorType err = TangoSupport_getTangoVersion(env, activity, &version);
  if (err != TANGO_SUCCESS || version < kTangoCoreMinimumVersion) {
    LOGE("AugmentedRealityApp::OnCreate, Tango Core version is out of date.");
    std::exit(EXIT_SUCCESS);
  }
}

void PointCloudApp::OnTangoServiceConnected(JNIEnv* env, jobject iBinder) {
  TangoErrorType ret = TangoService_setBinder(env, iBinder);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointCloudApp: Failed to set Tango binder with"
        "error code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  TangoSetupConfig();
  TangoConnectCallbacks();
  TangoConnect();
  is_service_connected_ = true;
}

void PointCloudApp::TangoSetupConfig() {
  // Here, we'll configure the service to run in the way we'd want. For this
  // application, we'll start from the default configuration
  // (TANGO_CONFIG_DEFAULT). This enables basic motion tracking capabilities.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE("PointCloudApp: Failed to get default config form");
    std::exit(EXIT_SUCCESS);
  }

  // Set auto-recovery for motion tracking as requested by the user.
  int ret =
      TangoConfig_setBool(tango_config_, "config_enable_auto_recovery", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointCloudApp: config_enable_auto_recovery() failed with error"
        "code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  // Enable depth.
  ret = TangoConfig_setBool(tango_config_, "config_enable_depth", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointCloudApp: config_enable_depth() failed with error"
        "code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  // Need to specify the depth_mode as XYZC.
  ret = TangoConfig_setInt32(tango_config_, "config_depth_mode",
                             TANGO_POINTCLOUD_XYZC);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointCloudApp: 'config_depth_mode' configuration flag with error"
        " code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  if (point_cloud_manager_ == nullptr) {
    int32_t max_point_cloud_elements;
    ret = TangoConfig_getInt32(tango_config_, "max_point_cloud_elements",
                               &max_point_cloud_elements);
    if (ret != TANGO_SUCCESS) {
      LOGE("Failed to query maximum number of point cloud elements.");
      std::exit(EXIT_SUCCESS);
    }

    ret = TangoSupport_createPointCloudManager(max_point_cloud_elements,
                                               &point_cloud_manager_);
    if (ret != TANGO_SUCCESS) {
      std::exit(EXIT_SUCCESS);
    }
  }
}

void PointCloudApp::TangoConnectCallbacks() {
  // Attach the OnPointCloudAvailable callback.
  // The callback will be called after the service is connected.
  int ret =
      TangoService_connectOnPointCloudAvailable(onPointCloudAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointCloudApp: Failed to connect to point cloud callback with error"
        "code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }
}

// Connect to the Tango Service, the service will start running:
// poses can be queried and callbacks will be called.
void PointCloudApp::TangoConnect() {
  TangoErrorType err = TangoService_connect(this, tango_config_);
  if (err != TANGO_SUCCESS) {
    LOGE(
        "PointCloudApp: Failed to connect to the Tango service with"
        "error code: %d",
        err);
    std::exit(EXIT_SUCCESS);
  }

  // Initialize TangoSupport context.
  TangoSupport_initialize(TangoService_getPoseAtTime,
                          TangoService_getCameraIntrinsics);
}

void PointCloudApp::OnPause() {
  TangoDisconnect();
  DeleteResources();
}

void PointCloudApp::TangoDisconnect() {
  // When disconnecting from the Tango Service, it is important to make sure to
  // free your configuration object. Note that disconnecting from the service,
  // resets all configuration, and disconnects all callbacks. If an application
  // resumes after disconnecting, it must re-register configuration and
  // callbacks with the service.
  TangoConfig_free(tango_config_);
  tango_config_ = nullptr;
  TangoService_disconnect();
  is_service_connected_ = false;
}

void PointCloudApp::OnSurfaceCreated() { main_scene_.InitGLContent(); }

void PointCloudApp::OnSurfaceChanged(int width, int height) {
  main_scene_.SetupViewPort(width, height);
  is_gl_initialized_ = true;
}

void PointCloudApp::OnDrawFrame() {
  main_scene_.ClearRender();

  if (!is_service_connected_ || !is_gl_initialized_) {
    return;
  }

  // Get the last point cloud data and associated pose.
  TangoPointCloud* point_cloud = nullptr;
  TangoPoseData pose;
  TangoErrorType ret_val = TangoSupport_getLatestPointCloudWithPose(
      point_cloud_manager_, TANGO_COORDINATE_FRAME_START_OF_SERVICE,
      TANGO_SUPPORT_ENGINE_OPENGL, TANGO_SUPPORT_ENGINE_TANGO,
      TANGO_SUPPORT_ROTATION_IGNORED, &point_cloud, &pose);
  if (ret_val != TANGO_SUCCESS || point_cloud == nullptr) {
    return;
  }

  // Get the last device transform to start of service frame in OpenGL
  // convention.
  TangoSupport_DoubleMatrixTransformData matrix_transform;
  TangoSupport_getDoubleMatrixTransformAtTime(
      point_cloud->timestamp, TANGO_COORDINATE_FRAME_START_OF_SERVICE,
      TANGO_COORDINATE_FRAME_DEVICE, TANGO_SUPPORT_ENGINE_OPENGL,
      TANGO_SUPPORT_ENGINE_OPENGL,
      static_cast<TangoSupport_Rotation>(screen_rotation_), &matrix_transform);
  if (matrix_transform.status_code == TANGO_POSE_VALID) {
    start_service_T_device_ = glm::make_mat4(matrix_transform.matrix);
  } else {
    LOGE(
        "PointCloudExample: Could not find a valid matrix transform at "
        "time %lf for the device.",
        0.0);
    return;
  }

  TangoPointCloud ow_point_cloud;
  ow_point_cloud.points = new float[point_cloud->num_points][4];
  ow_point_cloud.num_points = point_cloud->num_points;
  ret_val = TangoTransformHelpers_transformPointCloudWithPose(
      point_cloud, &pose, &ow_point_cloud);
  if (ret_val != TANGO_SUCCESS) {
    LOGE(
        "PointCloudExample: Could not transform point cloud into pose "
        "coordinate space.");
    return;
  }

  std::vector<float> vertices =
      std::vector<float>(&ow_point_cloud.points[0][0],
                         &ow_point_cloud.points[ow_point_cloud.num_points][0]);
  main_scene_.Render(start_service_T_device_, vertices);

  delete[] ow_point_cloud.points;
}

void PointCloudApp::DeleteResources() { main_scene_.DeleteResources(); }

void PointCloudApp::SetCameraType(
    tango_gl::GestureCamera::CameraType camera_type) {
  main_scene_.SetCameraType(camera_type);
}

void PointCloudApp::OnTouchEvent(int touch_count,
                                 tango_gl::GestureCamera::TouchEvent event,
                                 float x0, float y0, float x1, float y1) {
  if (!is_service_connected_ || !is_gl_initialized_) {
    return;
  }
  main_scene_.OnTouchEvent(touch_count, event, x0, y0, x1, y1);
}

void PointCloudApp::SetScreenRotation(int screen_rotation) {
  screen_rotation_ = screen_rotation;
}
}  // namespace tango_point_cloud
