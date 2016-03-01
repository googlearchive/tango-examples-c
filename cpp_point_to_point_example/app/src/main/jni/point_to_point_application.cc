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


bool PointToPointApplication::CheckTangoVersion(JNIEnv* env, jobject activity,
                                           int min_tango_version) {
  // Check the installed version of the TangoCore.  If it is too old, then
  // it will not support the most up to date features.
  int version;
  TangoErrorType err = TangoSupport_GetTangoVersion(env, activity, &version);
  return err == TANGO_SUCCESS && version >= min_tango_version;
}

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
      opengl_world_T_start_service_(
          tango_gl::conversions::opengl_world_T_tango_world()),
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

int PointToPointApplication::TangoInitialize(JNIEnv* env,
                                             jobject caller_activity) {
  // The first thing we need to do for any Tango enabled application is to
  // initialize the service. We will do that here, passing on the JNI
  // environment and jobject corresponding to the Android activity that is
  // calling us.
  int ret = TangoService_initialize(env, caller_activity);
  if (ret != TANGO_SUCCESS) {
    return ret;
  }

  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE("PointToPointApplication: Unable to get tango config");
    return TANGO_ERROR;
  }

  /**
   * The point_cloud_manager_ contains the depth buffer data and allows
   * reading and writing of data in a thread safe way.
   */
  if (point_cloud_manager_ == nullptr) {
    int32_t max_point_cloud_elements;
    ret = TangoConfig_getInt32(tango_config_, "max_point_cloud_elements",
                               &max_point_cloud_elements);
    if (ret != TANGO_SUCCESS) {
      LOGE(
          "PointToPointApplication: Failed to query maximum number of point"
          " cloud elements.");
      return ret;
    }

    ret = TangoSupport_createPointCloudManager(max_point_cloud_elements,
                                               &point_cloud_manager_);
    if (ret != TANGO_SUCCESS) {
      return ret;
    }
  }

  return TANGO_SUCCESS;
}

int PointToPointApplication::TangoSetupAndConnect() {
  // Here, we will configure the service to run in the way we would want. For
  // this application, we will start from the default configuration
  // (TANGO_CONFIG_DEFAULT). This enables basic motion tracking capabilities.
  // In addition to motion tracking, however, we want to run with depth so that
  // we can measure things. As such, we are going to set an additional flag
  // "config_enable_depth" to true.
  if (tango_config_ == nullptr) {
    return TANGO_ERROR;
  }

  TangoErrorType ret =
      TangoConfig_setBool(tango_config_, "config_enable_depth", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable depth.");
    return ret;
  }

  ret = TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable color camera.");
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
    return ret;
  }

  // Register for depth notification.
  ret = TangoService_connectOnXYZijAvailable(OnXYZijAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to connected to depth callback.");
    return ret;
  }

  // Here, we will connect to the TangoService and set up to run. Note that
  // we are passing in a pointer to ourselves as the context which will be
  // passed back in our callbacks.
  ret = TangoService_connect(this, tango_config_);
  if (ret != TANGO_SUCCESS) {
    LOGE("PointToPointApplication: Failed to connect to the Tango service.");
    return ret;
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
  }

  // Register for image notification.
  ret = TangoService_connectOnFrameAvailable(TANGO_CAMERA_COLOR, this,
                                             OnFrameAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE("PointToPointApplication: Error connecting color frame %d", ret);
    return ret;
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
      return ret;
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
  }

  constexpr float kNearPlane = 0.1;
  constexpr float kFarPlane = 100.0;

  projection_matrix_ar_ = tango_gl::Camera::ProjectionMatrixForCameraIntrinsics(
      color_camera_intrinsics_.width, color_camera_intrinsics_.height,
      color_camera_intrinsics_.fx, color_camera_intrinsics_.fy,
      color_camera_intrinsics_.cx, color_camera_intrinsics_.cy, kNearPlane,
      kFarPlane);

  return ret;
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

  // Querying the device frame transformation based on color GPU timestamp.
  TangoPoseData pose_start_service_T_device_t1;
  TangoCoordinateFramePair color_gpu_frame_pair;
  color_gpu_frame_pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  color_gpu_frame_pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  if (TangoService_getPoseAtTime(last_gpu_timestamp_, color_gpu_frame_pair,
                                 &pose_start_service_T_device_t1) !=
      TANGO_SUCCESS) {
    LOGE(
        "PointToPointApplication: Could not find a valid pose at time %lf"
        " for the color camera.",
        last_gpu_timestamp_);
  }

  if (pose_start_service_T_device_t1.status_code == TANGO_POSE_VALID) {
    GLRender(pose_start_service_T_device_t1);
  } else {
    LOGE(
        "PointToPointApplication: Invalid pose for gpu color image at time: "
        "%lf",
        last_gpu_timestamp_);
  }
}

void PointToPointApplication::GLRender(
    const TangoPoseData& pose_start_service_T_device) {
  glEnable(GL_CULL_FACE);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  video_overlay_->Render(glm::mat4(1.0), glm::mat4(1.0));
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  TangoPoseData pose_opengl_world_T_opengl_camera;
  TangoErrorType ret = TangoSupport_getPoseInEngineFrame(
      TANGO_SUPPORT_COORDINATE_CONVENTION_OPENGL,
      TANGO_COORDINATE_FRAME_CAMERA_COLOR, pose_start_service_T_device,
      &pose_opengl_world_T_opengl_camera);
  if (ret != TANGO_SUCCESS) {
    LOGE("PointToPointApplication::%s: error getting color camera pose.",
         __func__);
    return;
  }

  const glm::mat4 opengl_camera_T_opengl_world =
      glm::inverse(tango_gl::conversions::TransformFromArrays(
          pose_opengl_world_T_opengl_camera.translation,
          pose_opengl_world_T_opengl_camera.orientation));
  if (segment_is_drawable_) {
    segment_->Render(projection_matrix_ar_, opengl_camera_T_opengl_world);
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

    // Use transformation helper to calculate start_service_T_depth
    TangoPoseData pose_start_servce_T_device;
    GetStartServiceTDevicePose(&pose_start_servce_T_device);
    TangoPoseData pose_start_servce_T_depth;
    TangoErrorType ret = TangoSupport_getPoseInEngineFrame(
        TANGO_SUPPORT_COORDINATE_CONVENTION_TANGO,
        TANGO_COORDINATE_FRAME_CAMERA_DEPTH, pose_start_servce_T_device,
        &pose_start_servce_T_depth);
    if (ret != TANGO_SUCCESS) {
      LOGE("PointToPointApplication::%s: error getting depth camera pose",
           __func__);
      return;
    }

    const glm::mat4 start_service_T_depth =
        tango_gl::conversions::TransformFromArrays(
            pose_start_servce_T_depth.translation,
            pose_start_servce_T_depth.orientation);

    // Apply final transformation from start service to OpenGL world
    const glm::mat4 opengl_world_T_depth =
        opengl_world_T_start_service_ * start_service_T_depth;

    // Transform to world coordinates
    const glm::vec4 world_position =
        opengl_world_T_depth * glm::vec4(depth_position_vec, 1.0f);

    UpdateSegment(world_position);
  }
}

TangoErrorType PointToPointApplication::GetStartServiceTDevicePose(
    TangoPoseData* pose) {
  TangoCoordinateFramePair frame_pair;
  frame_pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  frame_pair.target = TANGO_COORDINATE_FRAME_DEVICE;

  return TangoService_getPoseAtTime(front_cloud_->timestamp, frame_pair, pose);
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
