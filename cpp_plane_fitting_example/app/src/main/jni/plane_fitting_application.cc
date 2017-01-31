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

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/quaternion.hpp>
#include <tango-gl/camera.h>
#include <tango-gl/conversions.h>
#include <tango-gl/util.h>

#include "tango-plane-fitting/plane_fitting.h"
#include <tango_support_api.h>

namespace tango_plane_fitting {

namespace {
// The minimum Tango Core version required from this application.
constexpr int kTangoCoreMinimumVersion = 9377;
constexpr float kCubeScale = 0.05f;

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
    : point_cloud_debug_render_(false),
      last_gpu_timestamp_(0.0),
      is_service_connected_(false),
      is_gl_initialized_(false),
      is_scene_camera_configured_(false) {}

PlaneFittingApplication::~PlaneFittingApplication() {
  TangoConfig_free(tango_config_);
  TangoSupport_freePointCloudManager(point_cloud_manager_);
  point_cloud_manager_ = nullptr;
}

void PlaneFittingApplication::OnCreate(JNIEnv* env, jobject activity) {
  // Check that we have the minimum required version of Tango.
  int version;
  TangoErrorType err = TangoSupport_GetTangoVersion(env, activity, &version);
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
  //
  // The drift corrected pose is is available through the frame pair with
  // base frame AREA_DESCRIPTION and target frame DEVICE.
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
  TangoSupport_initializeLibrary();
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
  point_cloud_renderer_ = new PointCloudRenderer();
  cube_ = new tango_gl::Cube();
  cube_->SetScale(glm::vec3(kCubeScale, kCubeScale, kCubeScale));
  cube_->SetColor(0.7f, 0.7f, 0.7f);

  is_gl_initialized_ = true;
}

void PlaneFittingApplication::SetRenderDebugPointCloud(bool on) {
  point_cloud_renderer_->SetRenderDebugColors(on);
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

  // Querying the GPU color image's frame transformation based its timestamp.
  TangoMatrixTransformData matrix_transform;
  TangoSupport_getMatrixTransformAtTime(
      last_gpu_timestamp_, TANGO_COORDINATE_FRAME_AREA_DESCRIPTION,
      TANGO_COORDINATE_FRAME_CAMERA_COLOR, TANGO_SUPPORT_ENGINE_OPENGL,
      TANGO_SUPPORT_ENGINE_OPENGL, display_rotation_, &matrix_transform);
  if (matrix_transform.status_code != TANGO_POSE_VALID) {
    LOGE(
        "PlaneFittingApplication: Could not find a valid matrix transform at "
        "time %lf for the color camera.",
        last_gpu_timestamp_);
  } else {
    const glm::mat4 area_description_T_color_camera =
        glm::make_mat4(matrix_transform.matrix);
    GLRender(area_description_T_color_camera);
  }
}

void PlaneFittingApplication::GLRender(
    const glm::mat4& area_description_T_color_camera) {
  glEnable(GL_CULL_FACE);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // We want to render from the perspective of the device, so we will set our
  // camera based on the transform that was passed in.
  glm::mat4 color_camera_T_area_description =
      glm::inverse(area_description_T_color_camera);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  video_overlay_->Render(glm::mat4(1.0), glm::mat4(1.0));
  glEnable(GL_DEPTH_TEST);

  TangoPointCloud* point_cloud = nullptr;
  TangoSupport_getLatestPointCloud(point_cloud_manager_, &point_cloud);
  if (point_cloud != nullptr) {
    const glm::mat4 area_description_opengl_T_depth_t1_tango =
        GetAreaDescriptionTDepthTransform(point_cloud->timestamp);
    const glm::mat4 projection_T_depth =
        projection_matrix_ar_ * color_camera_T_area_description *
        area_description_opengl_T_depth_t1_tango;
    point_cloud_renderer_->Render(projection_T_depth,
                                  area_description_opengl_T_depth_t1_tango,
                                  point_cloud);
  }

  glDisable(GL_BLEND);

  cube_->Render(projection_matrix_ar_, color_camera_T_area_description);
}

void PlaneFittingApplication::DeleteResources() {
  delete video_overlay_;
  delete cube_;
  delete point_cloud_renderer_;
  video_overlay_ = nullptr;
  cube_ = nullptr;
  point_cloud_renderer_ = nullptr;
}

void PlaneFittingApplication::OnDisplayChanged(int display_rotation) {
  display_rotation_ =
      static_cast<TangoSupportRotation>(display_rotation);
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

  // Calculate the conversion from the latest color camera position to the
  // most recent depth camera position. This corrects for screen lag between
  // the two systems.
  TangoPoseData pose_depth_camera_t0_T_color_camera_t1;
  int ret = TangoSupport_calculateRelativePose(
      point_cloud->timestamp, TANGO_COORDINATE_FRAME_CAMERA_DEPTH,
      last_gpu_timestamp_, TANGO_COORDINATE_FRAME_CAMERA_COLOR,
      &pose_depth_camera_t0_T_color_camera_t1);
  if (ret != TANGO_SUCCESS) {
    LOGE("%s: could not calculate relative pose", __func__);
    return;
  }
  glm::vec2 uv(x / screen_width_, y / screen_height_);

  double identity_translation[3] = {0.0, 0.0, 0.0};
  double identity_orientation[4] = {0.0, 0.0, 0.0, 1.0};
  glm::dvec3 double_depth_position;
  glm::dvec4 double_depth_plane_equation;
  if (TangoSupport_fitPlaneModelNearPoint(
          point_cloud, identity_translation, identity_orientation,
          glm::value_ptr(uv), display_rotation_,
          pose_depth_camera_t0_T_color_camera_t1.translation,
          pose_depth_camera_t0_T_color_camera_t1.orientation,
          glm::value_ptr(double_depth_position),
          glm::value_ptr(double_depth_plane_equation)) != TANGO_SUCCESS) {
    return;  // Assume error has already been reported.
  }

  const glm::vec3 depth_position =
      static_cast<glm::vec3>(double_depth_position);
  const glm::vec4 depth_plane_equation =
      static_cast<glm::vec4>(double_depth_plane_equation);

  const glm::mat4 area_description_opengl_T_depth_tango =
      GetAreaDescriptionTDepthTransform(point_cloud->timestamp);

  // Transform to Area Description coordinates
  const glm::vec4 area_description_position =
      area_description_opengl_T_depth_tango * glm::vec4(depth_position, 1.0f);

  glm::vec4 area_description_plane_equation;
  PlaneTransform(depth_plane_equation, area_description_opengl_T_depth_tango,
                 &area_description_plane_equation);

  point_cloud_renderer_->SetPlaneEquation(area_description_plane_equation);

  const glm::vec3 plane_normal(area_description_plane_equation);

  // Use world up as the second vector, unless they are nearly parallel.
  // In that case use world +Z.
  glm::vec3 normal_Y = glm::vec3(0.0f, 1.0f, 0.0f);
  const glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);
  const float kWorldUpThreshold = 0.5f;
  if (glm::dot(plane_normal, world_up) > kWorldUpThreshold) {
    normal_Y = glm::vec3(0.0f, 0.0f, 1.0f);
  }

  const glm::vec3 normal_Z = glm::normalize(glm::cross(plane_normal, normal_Y));
  normal_Y = glm::normalize(glm::cross(normal_Z, plane_normal));

  glm::mat3 rotation_matrix;
  rotation_matrix[0] = plane_normal;
  rotation_matrix[1] = normal_Y;
  rotation_matrix[2] = normal_Z;
  const glm::quat rotation = glm::toQuat(rotation_matrix);

  cube_->SetRotation(rotation);
  cube_->SetPosition(glm::vec3(area_description_position) +
                     plane_normal * kCubeScale);
}

glm::mat4 PlaneFittingApplication::GetAreaDescriptionTDepthTransform(
    double timestamp) {
  glm::mat4 area_description_opengl_T_depth_tango;
  TangoMatrixTransformData matrix_transform;

  // When drift correction mode is enabled in config file, we need to query
  // the device with respect to Area Description pose in order to use the
  // drift corrected pose.
  //
  // Note that if you don't want to use the drift corrected pose, the
  // normal device with respect to start of service pose is still available.
  TangoSupport_getMatrixTransformAtTime(
      timestamp, TANGO_COORDINATE_FRAME_AREA_DESCRIPTION,
      TANGO_COORDINATE_FRAME_CAMERA_DEPTH, TANGO_SUPPORT_ENGINE_OPENGL,
      TANGO_SUPPORT_ENGINE_TANGO, ROTATION_IGNORED, &matrix_transform);
  if (matrix_transform.status_code != TANGO_POSE_VALID) {
    // When the pose status is not valid, it indicates the tracking has
    // been lost. In this case, we simply stop rendering.
    //
    // This is also the place to display UI to suggest the user walk
    // to recover tracking.
    LOGE(
        "PlaneFittingApplication: Could not find a valid matrix transform at "
        "time %lf for the color camera.",
        last_gpu_timestamp_);
  } else {
    area_description_opengl_T_depth_tango =
        glm::make_mat4(matrix_transform.matrix);
  }
  return area_description_opengl_T_depth_tango;
}

}  // namespace tango_plane_fitting
