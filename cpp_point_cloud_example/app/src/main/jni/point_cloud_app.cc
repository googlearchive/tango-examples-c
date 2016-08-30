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
    : screen_rotation_(0), is_service_connected_(false) {}

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

  is_service_connected_ = true;

  // Initialize TangoSupport context.
  TangoSupport_initializeLibrary();
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
}

void PointCloudApp::OnDrawFrame() {
  main_scene_.ClearRender();

  if (!is_service_connected_) {
    return;
  }

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
    return;
  }

  // Compute the average depth value.
  float average_depth_ = 0.0f;
  for (size_t i = 0; i < front_cloud_->num_points; i++) {
    average_depth_ += front_cloud_->points[i][2];
  }
  if (front_cloud_->num_points) {
    average_depth_ /= front_cloud_->num_points;
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
    TangoPointCloud ow_point_cloud;
    ow_point_cloud.points = new float[front_cloud_->num_points][4];
    ow_point_cloud.num_points = front_cloud_->num_points;
    // Transform point cloud to OpenGL world
    TangoSupport_doubleTransformPointCloud(matrix_transform.matrix,
                                           front_cloud_, &ow_point_cloud);
    vertices.resize(front_cloud_->num_points * 4);
    std::copy(&ow_point_cloud.points[0][0],
              &ow_point_cloud.points[ow_point_cloud.num_points][0],
              vertices.begin());
  } else {
    LOGE(
        "PointCloudExample: Could not find a valid matrix transform at "
        "time %lf for the depth camera.",
        front_cloud_->timestamp);
    return;
  }

  main_scene_.Render(start_service_T_device_, vertices);
}

void PointCloudApp::DeleteResources() { main_scene_.DeleteResources(); }

int PointCloudApp::GetPointCloudVerticesCount() {
  if (front_cloud_ != nullptr) {
    return front_cloud_->num_points;
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
