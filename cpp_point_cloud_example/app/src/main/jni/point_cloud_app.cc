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

#include "tango-point-cloud/point_cloud_app.h"

namespace {
const int kVersionStringLength = 128;
// The minimum Tango Core version required from this application.
constexpr int kTangoCoreMinimumVersion = 9377;

// This function routes onXYZijAvailable callbacks to the application object for
// handling.
//
// @param context, context will be a pointer to a PointCloudApp
//        instance on which to call callbacks.
// @param pose, pose data to route to onPoseAvailable function.
void onPointCloudAvailableRouter(void* context, const TangoXYZij* xyz_ij) {
  tango_point_cloud::PointCloudApp* app =
      static_cast<tango_point_cloud::PointCloudApp*>(context);
  app->onPointCloudAvailable(xyz_ij);
}
}  // namespace

namespace tango_point_cloud {
void PointCloudApp::onPointCloudAvailable(const TangoXYZij* xyz_ij) {
  TangoSupport_updatePointCloud(point_cloud_manager_, xyz_ij);
}

PointCloudApp::PointCloudApp() {}

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
  TangoErrorType err = TangoSupport_GetTangoVersion(env, activity, &version);
  if (err != TANGO_SUCCESS || version < kTangoCoreMinimumVersion) {
    LOGE("AugmentedRealityApp::OnCreate, Tango Core version is out of date.");
    std::exit(EXIT_SUCCESS);
  }
}

bool PointCloudApp::OnTangoServiceConnected(JNIEnv* env, jobject iBinder) {
  TangoErrorType ret = TangoService_setBinder(env, iBinder);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "AugmentedRealityApp: Failed to set Tango binder with"
        "error code: %d",
        ret);
    return false;
  }

  TangoSetupConfig();
  TangoConnectCallbacks();
  TangoConnect();

  return true;
}

int PointCloudApp::TangoSetupConfig() {
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

  return ret;
}

int PointCloudApp::TangoConnectCallbacks() {
  // Attach the OnXYZijAvailable callback.
  // The callback will be called after the service is connected.
  int ret = TangoService_connectOnXYZijAvailable(onPointCloudAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointCloudApp: Failed to connect to point cloud callback with error"
        "code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  return ret;
}

// Connect to the Tango Service, the service will start running:
// poses can be queried and callbacks will be called.
bool PointCloudApp::TangoConnect() {
  TangoErrorType err = TangoService_connect(this, tango_config_);
  if (err != TANGO_SUCCESS) {
    LOGE(
        "PointCloudApp: Failed to connect to the Tango service with"
        "error code: %d",
        err);
    std::exit(EXIT_SUCCESS);
  }

  // Initialize TangoSupport context.
  TangoSupport_initialize(TangoService_getPoseAtTime);

  return true;
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
}

void PointCloudApp::InitializeGLContent() { main_scene_.InitGLContent(); }

void PointCloudApp::SetViewPort(int width, int height) {
  main_scene_.SetupViewPort(width, height);
}

void PointCloudApp::Render() {
  // Query the latest pose transformation and point cloud frame transformation.
  // Point cloud data comes in with a specific timestamp, in order to get the
  // closest pose for the point cloud, we will need to use the
  // TangoService_getMatrixTransformAtTime() to query a transform at timestamp.

  // Get the last point cloud data.
  UpdateCurrentPointData();

  // Get the last device transform to start of service frame in OpenGL
  // convention.
  TangoDoubleMatrixTransformData matrix_transform;
  TangoSupport_getDoubleMatrixTransformAtTime(
      0, TANGO_COORDINATE_FRAME_START_OF_SERVICE, TANGO_COORDINATE_FRAME_DEVICE,
      TANGO_SUPPORT_ENGINE_OPENGL, TANGO_SUPPORT_ENGINE_OPENGL,
      static_cast<TangoSupportDisplayRotation>(screen_rotation_),
      &matrix_transform);
  if (matrix_transform.status_code == TANGO_POSE_VALID) {
    start_service_T_device_ = glm::make_mat4(matrix_transform.matrix);
  } else {
    LOGE(
        "PointCloudExample: Could not find a valid matrix transform at "
        "time %lf for the device.",
        0.0);
  }

  // Compute the average depth value.
  float average_depth_ = 0.0f;
  size_t iteration_size = front_cloud_->xyz_count;
  size_t vertices_count = 0;
  for (size_t i = 0; i < iteration_size; i++) {
    average_depth_ += front_cloud_->xyz[i][2];
    vertices_count++;
  }
  if (vertices_count) {
    average_depth_ /= vertices_count;
  }
  point_cloud_average_depth_ = average_depth_;

  std::vector<float> vertices;
  // Get depth camera transform to start of service frame in OpenGL convention
  // at the point cloud timestamp.
  TangoSupport_getDoubleMatrixTransformAtTime(
      front_cloud_->timestamp, TANGO_COORDINATE_FRAME_START_OF_SERVICE,
      TANGO_COORDINATE_FRAME_CAMERA_DEPTH, TANGO_SUPPORT_ENGINE_OPENGL,
      TANGO_SUPPORT_ENGINE_TANGO, ROTATION_0, &matrix_transform);
  if (matrix_transform.status_code == TANGO_POSE_VALID) {
    start_service_opengl_T_depth_tango_ =
        glm::make_mat4(matrix_transform.matrix);
    TangoXYZij ow_point_cloud;
    ow_point_cloud.xyz = new float[front_cloud_->xyz_count][3];
    ow_point_cloud.xyz_count = front_cloud_->xyz_count;
    // Transform point cloud to OpenGL world
    TangoSupport_doubleTransformPointCloud(matrix_transform.matrix,
                                           front_cloud_, &ow_point_cloud);
    size_t point_cloud_size = front_cloud_->xyz_count * 3;
    vertices.resize(point_cloud_size);
    std::copy(ow_point_cloud.xyz[0], ow_point_cloud.xyz[0] + point_cloud_size,
              vertices.begin());
  } else {
    LOGE(
        "PointCloudExample: Could not find a valid matrix transform at "
        "time %lf for the depth camera.",
        front_cloud_->timestamp);
  }

  main_scene_.Render(start_service_T_device_, vertices);
}

void PointCloudApp::DeleteResources() { main_scene_.DeleteResources(); }

int PointCloudApp::GetPointCloudVerticesCount() {
  if (front_cloud_ != nullptr) {
    return front_cloud_->xyz_count;
  }
  return 0;
}

float PointCloudApp::GetAverageZ() { return point_cloud_average_depth_; }

void PointCloudApp::SetCameraType(
    tango_gl::GestureCamera::CameraType camera_type) {
  main_scene_.SetCameraType(camera_type);
}

void PointCloudApp::OnTouchEvent(int touch_count,
                                 tango_gl::GestureCamera::TouchEvent event,
                                 float x0, float y0, float x1, float y1) {
  main_scene_.OnTouchEvent(touch_count, event, x0, y0, x1, y1);
}

void PointCloudApp::UpdateCurrentPointData() {
  TangoSupport_getLatestPointCloud(point_cloud_manager_, &front_cloud_);
}

void PointCloudApp::SetScreenRotation(int screen_rotation) {
  screen_rotation_ = screen_rotation;
}
}  // namespace tango_point_cloud
