/*
 * Copyright 2015 Google Inc. All Rights Reserved.
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

#include "tango-plane-fitting/plane_fitting_application.h"

#include <limits>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/quaternion.hpp>
#include <tango-gl/camera.h>
#include <tango-gl/conversions.h>
#include <tango-gl/util.h>

#include "tango-plane-fitting/plane_fitting.h"
#include <tango_support.h>

namespace tango_plane_fitting {

namespace {
// The minimum Tango Core version required from this application.
constexpr int kTangoCoreMinimumVersion = 9377;
constexpr float kCubeScale = 0.05f;

const glm::mat4 kDepthTOpenGl(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                              0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

/**
 * This function will route callbacks to our application object via the context
 * parameter.
 *
 * @param context Will be a pointer to a PlaneFittingApplication instance on
 * which to call callbacks.
 * @param point_cloud The point cloud to pass on.
 */
void OnPointCloudAvailableRouter(void* context,
                                 const TangoPointCloud* point_cloud) {
  PlaneFittingApplication* app = static_cast<PlaneFittingApplication*>(context);
  app->OnPointCloudAvailable(point_cloud);
}

// This function does nothing. TangoService_connectOnTextureAvailable
// requires a callback function pointer, and it cannot be null.
void onTextureAvailableRouter(void*, TangoCameraId) { return; }

/**
 * Create an OpenGL perspective matrix from window size, camera intrinsics,
 * and clip settings.
 */
glm::mat4 ProjectionMatrixForCameraIntrinsics(
    const TangoCameraIntrinsics& intrinsics, float near, float far) {
  // Adjust camera intrinsics according to rotation
  double cx = intrinsics.cx;
  double cy = intrinsics.cy;
  double width = intrinsics.width;
  double height = intrinsics.height;
  double fx = intrinsics.fx;
  double fy = intrinsics.fy;

  return tango_gl::Camera::ProjectionMatrixForCameraIntrinsics(
      width, height, fx, fy, cx, cy, near, far);
}

}  // end namespace

void PlaneFittingApplication::OnPointCloudAvailable(
    const TangoPointCloud* point_cloud) {
  TangoSupport_updatePointCloud(point_cloud_manager_, point_cloud);
}

PlaneFittingApplication::PlaneFittingApplication()
    : depth_T_plane_(glm::mat4(1.0f)),
      plane_timestamp_(-1.0),
      last_gpu_timestamp_(0.0),
      is_service_connected_(false),
      is_gl_initialized_(false),
      is_scene_camera_configured_(false),
      is_cube_placed_(false) {}

PlaneFittingApplication::~PlaneFittingApplication() {
  TangoConfig_free(tango_config_);
  TangoSupport_freePointCloudManager(point_cloud_manager_);
  point_cloud_manager_ = nullptr;
}

void PlaneFittingApplication::OnCreate(JNIEnv* env, jobject activity) {
  // Check that we have the minimum required version of Tango.
  int version;
  TangoErrorType err = TangoSupport_getTangoVersion(env, activity, &version);
  if (err != TANGO_SUCCESS || version < kTangoCoreMinimumVersion) {
    LOGE(
        "PlaneFittingApplication::OnCreate, Tango Core version is out of "
        "date.");
    std::exit(EXIT_SUCCESS);
  }
}

void PlaneFittingApplication::OnTangoServiceConnected(JNIEnv* env,
                                                      jobject binder) {
  TangoErrorType ret = TangoService_setBinder(env, binder);
  if (ret != TANGO_SUCCESS) {
    LOGE("PlaneFittingApplication: TangoService_setBinder error");
    std::exit(EXIT_SUCCESS);
  }

  TangoSetupConfig();
  TangoConnectCallbacks();
  TangoConnect();
  is_service_connected_ = true;
}

void PlaneFittingApplication::TangoSetupConfig() {
  // Here, we will configure the service to run in the way we would want. For
  // this application, we will start from the default configuration
  // (TANGO_CONFIG_DEFAULT). This enables basic motion tracking capabilities.
  // In addition to motion tracking, however, we want to run with depth so that
  // we can measure things. As such, we are going to set an additional flag
  // "config_enable_depth" to true.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE(
        "PlaneFittingApplication::TangoSetupConfig, Unable to get tango "
        "config");
    std::exit(EXIT_SUCCESS);
  }

  TangoErrorType ret =
      TangoConfig_setBool(tango_config_, "config_enable_depth", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("PlaneFittingApplication::TangoSetupConfig, Failed to enable depth.");
    std::exit(EXIT_SUCCESS);
  }

  ret = TangoConfig_setInt32(tango_config_, "config_depth_mode",
                             TANGO_POINTCLOUD_XYZC);
  if (ret != TANGO_SUCCESS) {
    LOGE("PlaneFittingApplication::TangoSetupConfig, Failed to configure to "
         "XYZC.");
    std::exit(EXIT_SUCCESS);
  }

  ret = TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("PlaneFittingApplication::TangoSetupConfig, Failed to enable color "
         "camera.");
    std::exit(EXIT_SUCCESS);
  }

  // Note that it is super important for AR applications that we enable low
  // latency IMU integration so that we have pose information available as
  // quickly as possible. Without setting this flag, you will often receive
  // invalid poses when calling getPoseAtTime() for an image.
  ret = TangoConfig_setBool(tango_config_,
                            "config_enable_low_latency_imu_integration", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("PlaneFittingApplication::TangoSetupConfig, Failed to enable low "
         "latency imu integration.");
    std::exit(EXIT_SUCCESS);
  }

  // Drift correction allows motion tracking to recover after it loses tracking.
  ret = TangoConfig_setBool(tango_config_, "config_enable_drift_correction",
                            true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PlaneFittingApplication::TangoSetupConfig, enabling "
        "config_enable_drift_correction failed with error code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  if (point_cloud_manager_ == nullptr) {
    int32_t max_point_cloud_elements;
    ret = TangoConfig_getInt32(tango_config_, "max_point_cloud_elements",
                               &max_point_cloud_elements);
    if (ret != TANGO_SUCCESS) {
      LOGE(
          "PlaneFittingApplication::TangoSetupConfig, Failed to query maximum "
          "number of point cloud elements.");
      std::exit(EXIT_SUCCESS);
    }

    ret = TangoSupport_createPointCloudManager(max_point_cloud_elements,
                                               &point_cloud_manager_);
    if (ret != TANGO_SUCCESS) {
      LOGE(
          "PlaneFittingApplication::TangoSetupConfig, Failed to create a "
          "point cloud manager.");
      std::exit(EXIT_SUCCESS);
    }
  }
}

void PlaneFittingApplication::TangoConnectCallbacks() {
  // Register for depth notification.
  TangoErrorType ret =
      TangoService_connectOnPointCloudAvailable(OnPointCloudAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to connected to depth callback.");
    std::exit(EXIT_SUCCESS);
  }

  // The Tango service allows you to connect an OpenGL texture directly to its
  // RGB and fisheye cameras. This is the most efficient way of receiving
  // images from the service because it avoids copies. You get access to the
  // graphic buffer directly. As we are interested in rendering the color image
  // in our render loop, we will be polling for the color image as needed.
  ret = TangoService_connectOnTextureAvailable(TANGO_CAMERA_COLOR, this,
                                               onTextureAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PlaneFittingApplication: Failed to connect texture callback with"
        "error code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }
}

void PlaneFittingApplication::TangoConnect() {
  // Here, we will connect to the TangoService and set up to run. Note that
  // we are passing in a pointer to ourselves as the context which will be
  // passed back in our callbacks.
  TangoErrorType ret = TangoService_connect(this, tango_config_);
  if (ret != TANGO_SUCCESS) {
    LOGE("PlaneFittingApplication: Failed to connect to the Tango service.");
    std::exit(EXIT_SUCCESS);
  }

  // Get the intrinsics for the color camera and pass them on to the depth
  // image. We need these to know how to project the point cloud into the color
  // camera frame.
  ret = TangoService_getCameraIntrinsics(TANGO_CAMERA_COLOR,
                                         &color_camera_intrinsics_);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PlaneFittingApplication: Failed to get the intrinsics for the color"
        "camera.");
    std::exit(EXIT_SUCCESS);
  }

  // Initialize TangoSupport context.
  TangoSupport_initialize(TangoService_getPoseAtTime,
                          TangoService_getCameraIntrinsics);
}

void PlaneFittingApplication::OnPause() {
  is_service_connected_ = false;
  is_gl_initialized_ = false;
  TangoDisconnect();
  DeleteResources();
}

void PlaneFittingApplication::TangoDisconnect() { TangoService_disconnect(); }

void PlaneFittingApplication::OnSurfaceCreated() {
  video_overlay_ = new tango_gl::VideoOverlay();
  video_overlay_->SetDisplayRotation(display_rotation_);
  cube_ = new tango_gl::Cube();
  cube_->SetColor(0.7f, 0.7f, 0.7f);

  axis_ = new tango_gl::Axis();
  axis_->SetPosition(glm::vec3(-1.0f, -1.0f, -1.0f));
  axis_->SetParent(cube_);
  axis_->SetLineWidth(30.0f);

  is_gl_initialized_ = true;
}

void PlaneFittingApplication::OnSurfaceChanged(int width, int height) {
  screen_width_ = static_cast<float>(width);
  screen_height_ = static_cast<float>(height);

  is_scene_camera_configured_ = false;
}

void PlaneFittingApplication::OnDrawFrame() {
  // If tracking is lost, further down in this method Scene::Render
  // will not be called. Prevent flickering that would otherwise
  // happen by rendering solid black as a fallback.
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  if (!is_gl_initialized_ || !is_service_connected_) {
    return;
  }

  if (!is_scene_camera_configured_) {
    SetViewportAndProjectionGLThread();
    is_scene_camera_configured_ = true;
  }

  // We need to make sure that we update the texture associated with the color
  // image.
  if (TangoService_updateTextureExternalOes(
          TANGO_CAMERA_COLOR, video_overlay_->GetTextureId(),
          &last_gpu_timestamp_) != TANGO_SUCCESS) {
    LOGE("PlaneFittingApplication: Failed to get a color image.");
    return;
  }

  glEnable(GL_CULL_FACE);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  video_overlay_->Render(glm::mat4(1.0), glm::mat4(1.0));
  glEnable(GL_DEPTH_TEST);

  // Querying the GPU color image's frame transformation based its timestamp.
  TangoSupport_MatrixTransformData opengl_T_color_camera_matrix_transform;
  TangoSupport_getMatrixTransformAtTime(
      last_gpu_timestamp_, TANGO_COORDINATE_FRAME_START_OF_SERVICE,
      TANGO_COORDINATE_FRAME_CAMERA_COLOR, TANGO_SUPPORT_ENGINE_OPENGL,
      TANGO_SUPPORT_ENGINE_OPENGL, display_rotation_,
      &opengl_T_color_camera_matrix_transform);
  if (opengl_T_color_camera_matrix_transform.status_code != TANGO_POSE_VALID) {
    LOGE(
        "PlaneFittingApplication: Could not find a valid matrix transform at "
        "time %lf for the color camera.",
        last_gpu_timestamp_);
    return;
  }

  if (is_cube_placed_) {
    // To make sure drift correct pose is also applied to virtual
    // object (cube).
    // We need to re-query the Start of Service to Depth camera
    // pose every frame. Note that you will need to use the timestamp
    // at the time when the points were measured to query the pose.
    TangoSupport_MatrixTransformData opengl_T_depth_matrix_transform;
    TangoSupport_getMatrixTransformAtTime(
        plane_timestamp_, TANGO_COORDINATE_FRAME_START_OF_SERVICE,
        TANGO_COORDINATE_FRAME_CAMERA_DEPTH, TANGO_SUPPORT_ENGINE_OPENGL,
        TANGO_SUPPORT_ENGINE_TANGO, TANGO_SUPPORT_ROTATION_IGNORED,
        &opengl_T_depth_matrix_transform);

    if (opengl_T_depth_matrix_transform.status_code == TANGO_POSE_VALID) {
      glm::mat4 opengl_T_depth =
          glm::make_mat4(opengl_T_depth_matrix_transform.matrix);
      glm::mat4 opengl_T_plane =
          opengl_T_depth * depth_T_plane_ * kDepthTOpenGl;

      cube_->SetTransformationMatrix(opengl_T_plane);
      cube_->SetScale(glm::vec3(kCubeScale, kCubeScale, kCubeScale));

      const glm::mat4 view_matrix = glm::inverse(
          glm::make_mat4(opengl_T_color_camera_matrix_transform.matrix));
      cube_->Render(projection_matrix_ar_, view_matrix);
      axis_->Render(projection_matrix_ar_, view_matrix);
    }
  }
}

void PlaneFittingApplication::DeleteResources() {
  delete video_overlay_;
  delete cube_;
  video_overlay_ = nullptr;
  cube_ = nullptr;
}

void PlaneFittingApplication::OnDisplayChanged(int display_rotation) {
  display_rotation_ = static_cast<TangoSupport_Rotation>(display_rotation);
  is_scene_camera_configured_ = false;
}

void PlaneFittingApplication::SetViewportAndProjectionGLThread() {
  if (!is_gl_initialized_ || !is_service_connected_) {
    return;
  }

  TangoSupport_getCameraIntrinsicsBasedOnDisplayRotation(
      TANGO_CAMERA_COLOR, display_rotation_, &color_camera_intrinsics_);

  video_overlay_->SetDisplayRotation(display_rotation_);
  video_overlay_->SetTextureOffset(
      screen_width_, screen_height_,
      static_cast<float>(color_camera_intrinsics_.width),
      static_cast<float>(color_camera_intrinsics_.height));

  glViewport(0, 0, screen_width_, screen_height_);

  constexpr float kNearPlane = 0.1f;
  constexpr float kFarPlane = 100.0f;
  projection_matrix_ar_ = ProjectionMatrixForCameraIntrinsics(
      color_camera_intrinsics_, kNearPlane, kFarPlane);
}

// We assume the Java layer ensures this function is called on the GL thread.
void PlaneFittingApplication::OnTouchEvent(float x, float y) {
  if (!is_gl_initialized_ || !is_service_connected_) {
    return;
  }

  // Get the latest point cloud
  TangoPointCloud* point_cloud = nullptr;
  TangoSupport_getLatestPointCloud(point_cloud_manager_, &point_cloud);
  if (point_cloud == nullptr) {
    return;
  }

  // Get pose from depth camera to color camera.
  TangoPoseData pose_depth_T_color;
  TangoErrorType ret;

  ret = TangoSupport_getPoseAtTime(
      last_gpu_timestamp_, TANGO_COORDINATE_FRAME_CAMERA_DEPTH,
      TANGO_COORDINATE_FRAME_CAMERA_COLOR, TANGO_SUPPORT_ENGINE_TANGO,
      TANGO_SUPPORT_ENGINE_TANGO, TANGO_SUPPORT_ROTATION_IGNORED,
      &pose_depth_T_color);
  if (ret != TANGO_SUCCESS) {
    LOGE("%s: could not get openglTcolor pose for last_gpu_timestamp_ %f.",
         __func__, last_gpu_timestamp_);
    return;
  }

  // Touch location in [0,1] range.
  glm::vec2 uv(x / screen_width_, y / screen_height_);
  double zero_vector[3] = {0.0f, 0.0f, 0.0f};
  double identity_quaternion[4] = {0.0f, 0.0f, 0.0f, 1.0f};

  glm::dvec4 out_plane_model;
  glm::dvec3 out_plane_intersect;

  if (TangoSupport_fitPlaneModelNearPoint(
          point_cloud, zero_vector, identity_quaternion, glm::value_ptr(uv),
          display_rotation_, pose_depth_T_color.translation,
          pose_depth_T_color.orientation, glm::value_ptr(out_plane_intersect),
          glm::value_ptr(out_plane_model)) != TANGO_SUCCESS) {
    // Assuming errors have already been reported.
    return;
  }

  plane_timestamp_ = last_gpu_timestamp_;

  // Use world up as the second vector unless they are nearly parallel, in
  // which case use world +Z.
  const glm::vec3 plane_normal(static_cast<float>(out_plane_model.x),
                               static_cast<float>(out_plane_model.y),
                               static_cast<float>(out_plane_model.z));

  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
  const glm::vec3 z_axis = glm::normalize(plane_normal);
  const glm::vec3 x_axis = glm::normalize(glm::cross(up, z_axis));
  const glm::vec3 y_axis = glm::normalize(glm::cross(z_axis, x_axis));

  glm::vec3 normal_offeset = z_axis * kCubeScale;
  depth_T_plane_[0] = glm::vec4(x_axis.x, x_axis.y, x_axis.z, 0.0f);
  depth_T_plane_[1] = glm::vec4(y_axis.x, y_axis.y, y_axis.z, 0.0f);
  depth_T_plane_[2] = glm::vec4(z_axis.x, z_axis.y, z_axis.z, 0.0f);
  depth_T_plane_[3][0] =
      static_cast<float>(out_plane_intersect.x) + normal_offeset.x;
  depth_T_plane_[3][1] =
      static_cast<float>(out_plane_intersect.y) + normal_offeset.y;
  depth_T_plane_[3][2] =
      static_cast<float>(out_plane_intersect.z) + normal_offeset.z;
  is_cube_placed_ = true;
}

}  // namespace tango_plane_fitting
