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

constexpr float kCubeScale = 0.05f;
// The minimum Tango Core version required from this application.
constexpr int kTangoCoreMinimumVersion = 9377;

/**
 * This function will route callbacks to our application object via the context
 * parameter.
 *
 * @param context Will be a pointer to a PointToPointApplication instance on
 * which to call callbacks.
 * @param xyz_ij The point cloud to pass on.
 */
void OnXYZijAvailableRouter(void* context, const TangoXYZij* xyz_ij) {
  PointToPointApplication* app = static_cast<PointToPointApplication*>(context);
  app->OnXYZijAvailable(xyz_ij);
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

}  // namespace

void PointToPointApplication::OnXYZijAvailable(const TangoXYZij* xyz_ij) {
  TangoSupport_updatePointCloud(point_cloud_manager_, xyz_ij);
  TangoSupport_getLatestPointCloud(point_cloud_manager_, &front_cloud_);
}

void PointToPointApplication::OnFrameAvailable(const TangoImageBuffer* buffer) {
  TangoSupport_updateImageBuffer(image_buffer_manager_, buffer);
  TangoSupport_getLatestImageBuffer(image_buffer_manager_, &image_buffer_);
}

PointToPointApplication::PointToPointApplication()
    : last_gpu_timestamp_(0.0),
      tap_number_(0),
      point_modifier_flag_(true),
      point1_(glm::vec3(0.0, 0.0, 0.0)),
      point2_(glm::vec3(0.0, 0.0, 0.0)),
      segment_is_drawable_(false) {}

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

  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE("PointToPointApplication::OnCreate, Unable to get tango config");
    std::exit(EXIT_SUCCESS);
  }

  if (point_cloud_manager_ == nullptr) {
    int32_t max_point_cloud_elements;
    err = TangoConfig_getInt32(tango_config_, "max_point_cloud_elements",
                               &max_point_cloud_elements);
    if (err != TANGO_SUCCESS) {
      LOGE(
          "PointToPointApplication::OnCreate, "
          "Failed to query maximum number of point cloud elements.");
      std::exit(EXIT_SUCCESS);
    }

    err = TangoSupport_createPointCloudManager(max_point_cloud_elements,
                                               &point_cloud_manager_);
    if (err != TANGO_SUCCESS) {
      std::exit(EXIT_SUCCESS);
    }
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

  TangoSetupAndConnect();
}

int PointToPointApplication::TangoSetupAndConnect() {
  // Here, we will configure the service to run in the way we would want. For
  // this application, we will start from the default configuration
  // (TANGO_CONFIG_DEFAULT). This enables basic motion tracking capabilities.
  // In addition to motion tracking, however, we want to run with depth so that
  // we can measure things. As such, we are going to set an additional flag
  // "config_enable_depth" to true.
  if (tango_config_ == nullptr) {
    return false;
  }

  TangoErrorType ret;
  ret = TangoConfig_setBool(tango_config_, "config_enable_depth", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable depth.");
    std::exit(EXIT_SUCCESS);
  }

  ret = TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable color camera.");
    std::exit(EXIT_SUCCESS);
  }

  // Drift correction allows motion tracking to recover after it loses tracking.
  //
  // The drift corrected pose is is available through the frame pair with
  // base frame AREA_DESCRIPTION and target frame DEVICE.
  ret = TangoConfig_setBool(tango_config_, "config_enable_drift_correction",
                            true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Fail to enable drift correction mode");
    return ret;
  }

  // Note that it is super important for AR applications that we enable low
  // latency IMU integration so that we have pose information available as
  // quickly as possible. Without setting this flag, you will often receive
  // invalid poses when calling getPoseAtTime() for an image.
  ret = TangoConfig_setBool(tango_config_,
                            "config_enable_low_latency_imu_integration", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable low latency imu integration.");
    std::exit(EXIT_SUCCESS);
  }

  // Register for depth notification.
  ret = TangoService_connectOnXYZijAvailable(OnXYZijAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to connected to depth callback.");
    std::exit(EXIT_SUCCESS);
  }

  // Here, we will connect to the TangoService and set up to run. Note that
  // we are passing in a pointer to ourselves as the context which will be
  // passed back in our callbacks.
  ret = TangoService_connect(this, tango_config_);
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

  /**
   * The image_buffer_manager_ contains the image data and allows reading and
   * writing of data in a thread safe way.
   */
  if (!image_buffer_manager_) {
    ret = TangoSupport_createImageBufferManager(
        TANGO_HAL_PIXEL_FORMAT_YCrCb_420_SP, color_camera_intrinsics_.width,
        color_camera_intrinsics_.height, &image_buffer_manager_);

    if (ret != TANGO_SUCCESS) {
      LOGE("PointToPointApplication: Failed to create image buffer manager");
      std::exit(EXIT_SUCCESS);
    }
  }


  /**
   * The interpolator_ contains camera intrinsics and references to data buffers
   * allowing for effective upsampling of the depth data to camera image
   * resolution.
   */
  ret = TangoSupport_createDepthInterpolator(&color_camera_intrinsics_,
                                             &interpolator_);
  if (ret != TANGO_SUCCESS) {
    LOGE("PointToPointApplication: Failed to set up interpolator.");
    std::exit(EXIT_SUCCESS);
  }

  constexpr float kNearPlane = 0.1;
  constexpr float kFarPlane = 100.0;

  projection_matrix_ar_ = tango_gl::Camera::ProjectionMatrixForCameraIntrinsics(
      color_camera_intrinsics_.width, color_camera_intrinsics_.height,
      color_camera_intrinsics_.fx, color_camera_intrinsics_.fy,
      color_camera_intrinsics_.cx, color_camera_intrinsics_.cy, kNearPlane,
      kFarPlane);

  // Initialize TangoSupport context.
  TangoSupport_initialize(TangoService_getPoseAtTime);

  return ret;
}

void PointToPointApplication::OnPause() {
  TangoDisconnect();
  DeleteResources();
}

void PointToPointApplication::TangoDisconnect() { TangoService_disconnect(); }

int PointToPointApplication::InitializeGLContent() {
  video_overlay_ = new tango_gl::VideoOverlay();
  segment_ = new tango_gl::SegmentDrawable();
  segment_->SetLineWidth(4.0);
  segment_->SetColor(1.0, 1.0, 1.0);
  tap_number_ = 0;
  segment_is_drawable_ = false;
  int ret;

  // The Tango service allows you to connect an OpenGL texture directly to its
  // RGB and fisheye cameras. This is the most efficient way of receiving
  // images from the service because it avoids copies. You get access to the
  // graphic buffer directly. As we are interested in rendering the color image
  // in our render loop, we will be polling for the color image as needed.
  ret = TangoService_connectTextureId(
      TANGO_CAMERA_COLOR, video_overlay_->GetTextureId(), this, nullptr);
  if (ret != TANGO_SUCCESS) {
    LOGE("PointToPointApplication: Failed to initialize the video overlay");
    return ret;
  }
  return ret;
}

void PointToPointApplication::SetUpsampleViaBilateralFiltering(bool on) {
  if (on) {
    algorithm_ = UpsampleAlgorithm::kBilateral;
    return;
  }
  algorithm_ = UpsampleAlgorithm::kNearest;
}

void PointToPointApplication::SetViewPort(int width, int height) {
  screen_width_ = static_cast<GLsizei>(width);
  screen_height_ = static_cast<GLsizei>(height);

  glViewport(0, 0, screen_width_, screen_height_);
}

void PointToPointApplication::Render() {
  // Update the texture associated with the color image.
  if (TangoService_updateTexture(TANGO_CAMERA_COLOR, &last_gpu_timestamp_) !=
      TANGO_SUCCESS) {
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
      TANGO_SUPPORT_ENGINE_OPENGL, ROTATION_0, &matrix_transform);
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
  /// Calculate the conversion from the latest depth camera position to the
  /// position of the most recent color camera image. This corrects for screen
  /// lag between the two systems.
  TangoPoseData pose_color_camera_t0_T_depth_camera_t1;
  int ret = TangoSupport_calculateRelativePose(
      last_gpu_timestamp_, TANGO_COORDINATE_FRAME_CAMERA_COLOR,
      front_cloud_->timestamp, TANGO_COORDINATE_FRAME_CAMERA_DEPTH,
      &pose_color_camera_t0_T_depth_camera_t1);
  if (ret != TANGO_SUCCESS) {
    LOGE("PointToPointApplication::%s: could not calculate relative pose",
         __func__);
    return;
  }
  float uv[2] = {x / screen_width_, y / screen_height_};
  // use this to calculate position relative to depth camera
  float depth_position[3] = {0.0f, 0.0f, 0.0f};
  // This sets the position relative to the depth camera.
  // Returns true if it is a valid point.
  if (GetDepthAtPoint(uv, depth_position,
                      pose_color_camera_t0_T_depth_camera_t1)) {
    const glm::vec3 depth_position_vec =
        glm::vec3(depth_position[0], depth_position[1], depth_position[2]);

    const glm::mat4 opengl_world_T_depth = GetStartServiceTDepthPose();

    // Transform to world coordinates
    const glm::vec4 world_position =
        opengl_world_T_depth * glm::vec4(depth_position_vec, 1.0f);

    UpdateSegment(world_position);
  }
}

glm::mat4 PointToPointApplication::GetStartServiceTDepthPose() {
  glm::mat4 start_service_opengl_T_depth_tango;
  TangoMatrixTransformData matrix_transform;
  TangoSupport_getMatrixTransformAtTime(
      front_cloud_->timestamp, TANGO_COORDINATE_FRAME_AREA_DESCRIPTION,
      TANGO_COORDINATE_FRAME_CAMERA_DEPTH, TANGO_SUPPORT_ENGINE_OPENGL,
      TANGO_SUPPORT_ENGINE_TANGO, ROTATION_0, &matrix_transform);
  if (matrix_transform.status_code != TANGO_POSE_VALID) {
    LOGE(
        "PointToPointApplication: Could not find a valid matrix transform at "
        "time %lf for the color camera.",
        last_gpu_timestamp_);
  } else {
    start_service_opengl_T_depth_tango =
        glm::make_mat4(matrix_transform.matrix);
  }
  return start_service_opengl_T_depth_tango;
}

bool PointToPointApplication::GetDepthAtPoint(
    const float uv[2], float xyz[3],
    const TangoPoseData& color_camera_T_point_cloud) {
  int is_valid_point = 0;
  if (algorithm_ == UpsampleAlgorithm::kNearest) {
    TangoSupport_getDepthAtPointNearestNeighbor(
        front_cloud_, &color_camera_intrinsics_, &color_camera_T_point_cloud,
        uv, xyz, &is_valid_point);
    return is_valid_point;
  }
  TangoSupport_getDepthAtPointBilateral(
      interpolator_, front_cloud_, image_buffer_, &color_camera_T_point_cloud,
      uv, xyz, &is_valid_point);
  return is_valid_point;
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

double DistanceSquared(glm::vec3 pt1, glm::vec3 pt2) {
  double v1((pt1[0] - pt2[0]) * (pt1[0] - pt2[0]));
  double v2((pt1[1] - pt2[1]) * (pt1[1] - pt2[1]));
  double v3((pt1[2] - pt2[2]) * (pt1[2] - pt2[2]));
  return v1 + v2 + v3;
}

std::string PointToPointApplication::GetPointSeparation() {
  std::lock_guard<std::mutex> lock(tango_points_mutex_);
  if (segment_is_drawable_) {
    double value(sqrt(DistanceSquared(point1_, point2_)));
    std::ostringstream strs;
    strs << value << " meters";
    return strs.str();
  }
  return "Undefined";
}

}  // namespace tango_point_to_point
