/*
 * Copyright 2016 Google Inc. All Rights Reserved.
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

#include "tango-point-to-point/point_to_point_application.h"

#include <cmath>
#include <sstream>

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/quaternion.hpp>
#include <tango-gl/camera.h>
#include <tango-gl/conversions.h>
#include <tango-gl/util.h>

namespace tango_point_to_point {

namespace {
// The minimum Tango Core version required from this application.
constexpr int kTangoCoreMinimumVersion = 9377;

/**
 * This function will route callbacks to our application object via the context
 * parameter.
 *
 * @param context Will be a pointer to a PointToPointApplication instance on
 * which to call callbacks.
 * @param point_cloud The point cloud to pass on.
 */
void OnPointCloudAvailableRouter(void* context,
                                 const TangoPointCloud* point_cloud) {
  PointToPointApplication* app = static_cast<PointToPointApplication*>(context);
  app->OnPointCloudAvailable(point_cloud);
}

/**
 * This function will route callbacks to our application object via the context
 * parameter.
 *
 * @param context Will be a pointer to a PointToPointApplication instance on
 * which to call callbacks.
 * @param buffer The image buffer to pass on.
 */
void OnFrameAvailableRouter(void* context, TangoCameraId,
                            const TangoImageBuffer* buffer) {
  PointToPointApplication* app = static_cast<PointToPointApplication*>(context);
  app->OnFrameAvailable(buffer);
}

/**
 * Create an OpenGL perspective matrix from window size, camera intrinsics,
 * and clip settings.
 */
glm::mat4 ProjectionMatrixForCameraIntrinsics(
    const TangoCameraIntrinsics& intrinsics, float near, float far,
    TangoSupportDisplayRotation rotation) {
  // Adjust camera intrinsics according to rotation
  double cx = intrinsics.cx;
  double cy = intrinsics.cy;
  double width = intrinsics.width;
  double height = intrinsics.height;
  double fx = intrinsics.fx;
  double fy = intrinsics.fy;

  switch (rotation) {
    case TangoSupportDisplayRotation::ROTATION_90:
      cx = intrinsics.cy;
      cy = intrinsics.width - intrinsics.cx;
      width = intrinsics.height;
      height = intrinsics.width;
      fx = intrinsics.fy;
      fy = intrinsics.fx;
      break;
    case TangoSupportDisplayRotation::ROTATION_180:
      cx = intrinsics.width - cx;
      cy = intrinsics.height - cy;
      break;
    case TangoSupportDisplayRotation::ROTATION_270:
      cx = intrinsics.height - intrinsics.cy;
      cy = intrinsics.cx;
      width = intrinsics.height;
      height = intrinsics.width;
      fx = intrinsics.fy;
      fy = intrinsics.fx;
    default:
      break;
  }

  return tango_gl::Camera::ProjectionMatrixForCameraIntrinsics(
      width, height, fx, fy, cx, cy, near, far);
}

}  // namespace

void PointToPointApplication::OnPointCloudAvailable(
    const TangoPointCloud* point_cloud) {
  TangoSupport_updatePointCloud(point_cloud_manager_, point_cloud);
}

void PointToPointApplication::OnFrameAvailable(const TangoImageBuffer* buffer) {
  TangoSupport_updateImageBuffer(image_buffer_manager_, buffer);
}

PointToPointApplication::PointToPointApplication()
    : screen_width_(0.0f),
      screen_height_(0.0f),
      last_gpu_timestamp_(0.0),
      tap_number_(0),
      point_modifier_flag_(true),
      point1_(glm::vec3(0.0, 0.0, 0.0)),
      point2_(glm::vec3(0.0, 0.0, 0.0)),
      segment_is_drawable_(false),
      is_service_connected_(false),
      is_gl_initialized_(false),
      display_rotation_(TangoSupportDisplayRotation::ROTATION_0),
      color_camera_to_display_rotation_(
          TangoSupportDisplayRotation::ROTATION_0) {}

PointToPointApplication::~PointToPointApplication() {
  TangoConfig_free(tango_config_);
  TangoSupport_freePointCloudManager(point_cloud_manager_);
  point_cloud_manager_ = nullptr;
  TangoSupport_freeDepthInterpolator(interpolator_);
  interpolator_ = nullptr;
  TangoSupport_freeImageBufferManager(image_buffer_manager_);
  image_buffer_manager_ = nullptr;
}

void PointToPointApplication::OnCreate(JNIEnv* env, jobject activity) {
  // Check the installed version of the TangoCore.  If it is too old, then
  // it will not support the most up to date features.
  int version;
  TangoErrorType err = TangoSupport_GetTangoVersion(env, activity, &version);

  if (err != TANGO_SUCCESS || version < kTangoCoreMinimumVersion) {
    LOGE(
        "PointToPointApplication::OnCreate, Tango Core version is out of "
        "date.");
    std::exit(EXIT_SUCCESS);
  }
}

void PointToPointApplication::OnTangoServiceConnected(JNIEnv* env,
                                                      jobject binder) {
  TangoErrorType ret = TangoService_setBinder(env, binder);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointToPointApplication: Failed to initialize Tango service with"
        "error code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  TangoSetupConfig();
  TangoConnectCallbacks();
  TangoConnect();
  is_service_connected_ = true;
}

void PointToPointApplication::TangoSetupConfig() {
  // Here, we will configure the service to run in the way we would want. For
  // this application, we will start from the default configuration
  // (TANGO_CONFIG_DEFAULT). This enables basic motion tracking capabilities.
  // In addition to motion tracking, however, we want to run with depth so that
  // we can measure things. As such, we are going to set an additional flag
  // "config_enable_depth" to true.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE(
        "PointToPointApplication::TangoSetupConfig, "
        "Unable to get tango config");
    std::exit(EXIT_SUCCESS);
  }

  TangoErrorType ret;
  ret = TangoConfig_setBool(tango_config_, "config_enable_depth", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointToPointApplication::TangoSetupConfig, "
        "Failed to enable depth.");
    std::exit(EXIT_SUCCESS);
  }

  // Need to specify the depth_mode as XYZC.
  ret = TangoConfig_setInt32(tango_config_, "config_depth_mode",
                             TANGO_POINTCLOUD_XYZC);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "Failed to set 'depth_mode' configuration flag with error"
        " code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  ret = TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointToPointApplication::TangoSetupConfig, "
        "Failed to enable color camera.");
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
        "PointToPointApplication::TangoSetupConfig, "
        "Fail to enable drift correction mode");
    std::exit(EXIT_SUCCESS);
  }

  // Note that it is super important for AR applications that we enable low
  // latency IMU integration so that we have pose information available as
  // quickly as possible. Without setting this flag, you will often receive
  // invalid poses when calling getPoseAtTime() for an image.
  ret = TangoConfig_setBool(tango_config_,
                            "config_enable_low_latency_imu_integration", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointToPointApplication::TangoSetupConfig, "
        "Failed to enable low latency imu integration.");
    std::exit(EXIT_SUCCESS);
  }
}

void PointToPointApplication::TangoConnectCallbacks() {
  if (point_cloud_manager_ == nullptr) {
    int32_t max_point_cloud_elements;
    TangoErrorType ret = TangoConfig_getInt32(
        tango_config_, "max_point_cloud_elements", &max_point_cloud_elements);
    if (ret != TANGO_SUCCESS) {
      LOGE(
          "PointToPointApplication::TangoConnectCallbacks, "
          "Failed to query maximum number of point cloud elements.");
      std::exit(EXIT_SUCCESS);
    }

    ret = TangoSupport_createPointCloudManager(max_point_cloud_elements,
                                               &point_cloud_manager_);
    if (ret != TANGO_SUCCESS) {
      std::exit(EXIT_SUCCESS);
    }
  }

  // Register for depth notification.
  TangoErrorType ret =
      TangoService_connectOnPointCloudAvailable(OnPointCloudAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to connected to depth callback.");
    std::exit(EXIT_SUCCESS);
  }

  // Connect to color camera.
  ret =
      TangoService_connectOnTextureAvailable(TANGO_CAMERA_COLOR, this, nullptr);
  if (ret != TANGO_SUCCESS) {
    LOGE("PointToPointApplication: Failed to initialize the video overlay");
    std::exit(EXIT_SUCCESS);
  }
}

void PointToPointApplication::TangoConnect() {
  // Here, we will connect to the TangoService and set up to run. Note that
  // we are passing in a pointer to ourselves as the context which will be
  // passed back in our callbacks.
  TangoErrorType ret = TangoService_connect(this, tango_config_);
  if (ret != TANGO_SUCCESS) {
    LOGE("PointToPointApplication: Failed to connect to the Tango service.");
    std::exit(EXIT_SUCCESS);
  }

  // Get the intrinsics for the color camera and pass them on to the depth
  // image. We need these to know how to project the point cloud into the color
  // camera frame.
  ret = TangoService_getCameraIntrinsics(TANGO_CAMERA_COLOR,
                                         &color_camera_intrinsics_);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointToPointApplication: Failed to get the intrinsics for the color"
        "camera.");
    std::exit(EXIT_SUCCESS);
  }

  // Register for image notification.
  ret = TangoService_connectOnFrameAvailable(TANGO_CAMERA_COLOR, this,
                                             OnFrameAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE("PointToPointApplication: Error connecting color frame %d", ret);
    std::exit(EXIT_SUCCESS);
  }

  // Initialize TangoSupport context.
  TangoSupport_initializeLibrary();

  // The image_buffer_manager_ contains the image data and allows reading and
  // writing of data in a thread safe way.
  if (!image_buffer_manager_) {
    ret = TangoSupport_createImageBufferManager(
        TANGO_HAL_PIXEL_FORMAT_YCrCb_420_SP, color_camera_intrinsics_.width,
        color_camera_intrinsics_.height, &image_buffer_manager_);

    if (ret != TANGO_SUCCESS) {
      LOGE("PointToPointApplication: Failed to create image buffer manager");
      std::exit(EXIT_SUCCESS);
    }
  }

  // The interpolator_ contains camera intrinsics and references to data buffers
  // allowing for effective upsampling of the depth data to camera image
  // resolution.
  ret = TangoSupport_createDepthInterpolator(&interpolator_);
  if (ret != TANGO_SUCCESS) {
    LOGE("PointToPointApplication: Failed to set up interpolator.");
    std::exit(EXIT_SUCCESS);
  }
}

void PointToPointApplication::OnPause() {
  is_service_connected_ = false;
  is_gl_initialized_ = false;
  TangoDisconnect();
  DeleteResources();
}

void PointToPointApplication::TangoDisconnect() { TangoService_disconnect(); }

void PointToPointApplication::OnSurfaceCreated() {
  video_overlay_ = new tango_gl::VideoOverlay();
  video_overlay_->SetColorToDisplayRotation(color_camera_to_display_rotation_);

  segment_ = new tango_gl::SegmentDrawable();
  segment_->SetLineWidth(4.0);
  segment_->SetColor(1.0, 1.0, 1.0);
  tap_number_ = 0;
  segment_is_drawable_ = false;

  is_gl_initialized_ = true;
}

void PointToPointApplication::SetUpsampleViaBilateralFiltering(bool on) {
  if (on) {
    algorithm_ = UpsampleAlgorithm::kBilateral;
    return;
  }
  algorithm_ = UpsampleAlgorithm::kNearest;
}

void PointToPointApplication::OnSurfaceChanged(int width, int height) {
  screen_width_ = static_cast<float>(width);
  screen_height_ = static_cast<float>(height);

  SetViewportAndProjection();
}

void PointToPointApplication::SetViewportAndProjection() {
  if (!is_gl_initialized_ || !is_service_connected_) {
    return;
  }

  video_overlay_->SetColorToDisplayRotation(color_camera_to_display_rotation_);
  video_overlay_->SetTextureOffset(
      screen_width_, screen_height_,
      static_cast<float>(color_camera_intrinsics_.width),
      static_cast<float>(color_camera_intrinsics_.height));

  glViewport(0, 0, screen_width_, screen_height_);

  constexpr float kNearPlane = 0.1f;
  constexpr float kFarPlane = 100.0f;
  projection_matrix_ar_ = ProjectionMatrixForCameraIntrinsics(
      color_camera_intrinsics_, kNearPlane, kFarPlane,
      color_camera_to_display_rotation_);
}

void PointToPointApplication::OnDrawFrame() {
  // If tracking is lost, further down in this method Scene::Render
  // will not be called. Prevent flickering that would otherwise
  // happen by rendering solid black as a fallback.
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  if (!is_gl_initialized_ || !is_service_connected_) {
    return;
  }

  // Update the texture associated with the color camera.
  if (TangoService_updateTextureExternalOes(
          TANGO_CAMERA_COLOR, video_overlay_->GetTextureId(),
          &last_gpu_timestamp_) != TANGO_SUCCESS) {
    LOGE("PointToPointApplication: Failed to get a color image.");
    return;
  }

  // Querying the GPU color image's frame transformation based its timestamp.
  //
  // When drift correction mode is enabled in config file, we need to query
  // the device with respect to Area Description pose in order to use the
  // drift corrected pose.
  //
  // Note that if you don't want to use the drift corrected pose, the
  // normal device with respect to start of service pose is still available.
  TangoMatrixTransformData matrix_transform;
  TangoSupport_getMatrixTransformAtTime(
      last_gpu_timestamp_, TANGO_COORDINATE_FRAME_AREA_DESCRIPTION,
      TANGO_COORDINATE_FRAME_CAMERA_COLOR, TANGO_SUPPORT_ENGINE_OPENGL,
      TANGO_SUPPORT_ENGINE_OPENGL, static_cast<TangoSupportDisplayRotation>(
                                       color_camera_to_display_rotation_),
      &matrix_transform);
  if (matrix_transform.status_code != TANGO_POSE_VALID) {
    // When the pose status is not valid, it indicates the tracking has
    // been lost. In this case, we simply stop rendering.
    //
    // This is also the place to display UI to suggest the user walk
    // to recover tracking.
    LOGE(
        "PointToPointApplication: Could not find a valid matrix transform at "
        "time %lf for the color camera.",
        last_gpu_timestamp_);
    return;
  } else {
    const glm::mat4 start_service_T_color_camera =
        glm::make_mat4(matrix_transform.matrix);
    GLRender(start_service_T_color_camera);
  }
}

void PointToPointApplication::GLRender(
    const glm::mat4& start_service_T_color_camera) {
  glEnable(GL_CULL_FACE);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  video_overlay_->Render(glm::mat4(1.0), glm::mat4(1.0));
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  const glm::mat4 color_camera_T_start_service =
      glm::inverse(start_service_T_color_camera);

  if (segment_is_drawable_) {
    segment_->Render(projection_matrix_ar_, color_camera_T_start_service);
  }
}

void PointToPointApplication::DeleteResources() {
  delete video_overlay_;
  delete segment_;
  video_overlay_ = nullptr;
  segment_ = nullptr;
}

// We assume the Java layer ensures this function is called on the GL thread.
void PointToPointApplication::OnTouchEvent(float x, float y) {
  if (!is_gl_initialized_ || !is_service_connected_) {
    return;
  }

  // Get the latest point cloud.
  TangoPointCloud* point_cloud = nullptr;
  TangoSupport_getLatestPointCloud(point_cloud_manager_, &point_cloud);
  if (point_cloud == nullptr) {
    return;
  }

  // Get the latest color image since we need it for the bilateral upsample.
  TangoImageBuffer* image = nullptr;
  TangoSupport_getLatestImageBuffer(image_buffer_manager_, &image);

  // We want to use either CPU or GPU image timestamps based on our upsampling
  // method.
  double color_image_timestamp = 0.0;
  if (algorithm_ == UpsampleAlgorithm::kNearest) {
    color_image_timestamp = last_gpu_timestamp_;  // GPU
  } else {
    color_image_timestamp = image->timestamp;  // CPU
  }

  // Calculate the relative pose between the depth camera and color camera.
  TangoPoseData pose_color_camera_T_depth_camera;
  int ret = TangoSupport_calculateRelativePose(
      color_image_timestamp, TANGO_COORDINATE_FRAME_CAMERA_COLOR,
      point_cloud->timestamp, TANGO_COORDINATE_FRAME_CAMERA_DEPTH,
      &pose_color_camera_T_depth_camera);
  if (ret != TANGO_SUCCESS) {
    LOGE("PointToPointApplication::%s: could not calculate relative pose",
         __func__);
    return;
  }

  // Get the point near the user's click.
  glm::vec2 uv = glm::vec2(x / screen_width_, y / screen_height_);
  glm::vec2 rotated_uv = tango_gl::util::GetColorCameraUVFromDisplay(
      uv, color_camera_to_display_rotation_);
  float color_position[3] = {0.0f, 0.0f, 0.0f};
  double identity_translation[3] = {0.0, 0.0, 0.0};
  double identity_orientation[4] = {0.0, 0.0, 0.0, 1.0};
  TangoErrorType depth_at_point_return;
  if (algorithm_ == UpsampleAlgorithm::kNearest) {
    depth_at_point_return = TangoSupport_getDepthAtPointNearestNeighbor(
        point_cloud, pose_color_camera_T_depth_camera.translation,
        pose_color_camera_T_depth_camera.orientation,
        glm::value_ptr(rotated_uv), identity_translation, identity_orientation,
        color_position);
  } else {
    depth_at_point_return = TangoSupport_getDepthAtPointBilateral(
        interpolator_, point_cloud,
        pose_color_camera_T_depth_camera.translation,
        pose_color_camera_T_depth_camera.orientation, image,
        glm::value_ptr(rotated_uv), identity_translation, identity_orientation,
        color_position);
  }

  // If we found a point, let's transform it to the world and draw it.
  if (depth_at_point_return == TANGO_SUCCESS) {
    const glm::vec3 color_position_vec =
        glm::vec3(color_position[0], color_position[1], color_position[2]);
    const glm::mat4 opengl_world_T_color =
        GetStartServiceTColorPose(color_image_timestamp);
    // Transform to world coordinates
    const glm::vec4 world_position =
        opengl_world_T_color * glm::vec4(color_position_vec, 1.0f);
    UpdateSegment(world_position);
  } else {
    LOGE("No depth for this point");
  }
}

void PointToPointApplication::OnDisplayChanged(int display_rotation,
                                               int color_camera_rotation) {
  display_rotation_ =
      static_cast<TangoSupportDisplayRotation>(display_rotation);
  color_camera_to_display_rotation_ =
      tango_gl::util::GetAndroidRotationFromColorCameraToDisplay(
          display_rotation_, color_camera_rotation);

  SetViewportAndProjection();
}

glm::mat4 PointToPointApplication::GetStartServiceTColorPose(
    const double& image_time) {
  glm::mat4 start_service_opengl_T_color_tango;
  TangoMatrixTransformData matrix_transform;
  TangoSupport_getMatrixTransformAtTime(
      image_time, TANGO_COORDINATE_FRAME_AREA_DESCRIPTION,
      TANGO_COORDINATE_FRAME_CAMERA_COLOR, TANGO_SUPPORT_ENGINE_OPENGL,
      TANGO_SUPPORT_ENGINE_TANGO, static_cast<TangoSupportDisplayRotation>(0),
      &matrix_transform);
  if (matrix_transform.status_code != TANGO_POSE_VALID) {
    LOGE(
        "PointToPointApplication: Could not find a valid matrix transform at "
        "time %lf for the color camera.",
        image_time);
  } else {
    start_service_opengl_T_color_tango =
        glm::make_mat4(matrix_transform.matrix);
  }
  return start_service_opengl_T_color_tango;
}

void PointToPointApplication::UpdateSegment(glm::vec4 world_position) {
  // Update the points with world_position.
  std::lock_guard<std::mutex> lock(tango_points_mutex_);
  if (point_modifier_flag_) {
    point1_ = glm::vec3(world_position);
  } else {
    point2_ = glm::vec3(world_position);
  }
  point_modifier_flag_ = !point_modifier_flag_;

  // Only draw on second or later tap.
  if (tap_number_ != 2) {
    if (tap_number_ == 0) {
      ++tap_number_;
      return;
    }
    ++tap_number_;
    segment_is_drawable_ = true;
  }
  segment_->UpdateSegment(tango_gl::Segment(point1_, point2_));
}

std::string PointToPointApplication::GetPointSeparation() {
  std::lock_guard<std::mutex> lock(tango_points_mutex_);
  if (segment_is_drawable_) {
    float dist = glm::distance(point1_, point2_);
    std::ostringstream strs;
    strs << dist << " meters";
    return strs.str();
  }
  return "Undefined";
}

}  // namespace tango_point_to_point
