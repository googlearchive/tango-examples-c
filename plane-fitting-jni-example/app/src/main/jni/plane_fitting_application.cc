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

constexpr float kCubeScale = 0.05f;

/**
 * This function will route callbacks to our application object via the context
 * parameter.
 *
 * @param context Will be a pointer to a PlaneFittingApplication instance on
 * which to call callbacks.
 * @param xyz_ij The point cloud to pass on.
 */
void OnXYZijAvailableRouter(void* context, const TangoXYZij* xyz_ij) {
  PlaneFittingApplication* app = static_cast<PlaneFittingApplication*>(context);
  app->OnXYZijAvailable(xyz_ij);
}

}  // end namespace

void PlaneFittingApplication::OnXYZijAvailable(const TangoXYZij* xyz_ij) {
  TangoSupport_updatePointCloud(point_cloud_manager_, xyz_ij);
}

PlaneFittingApplication::PlaneFittingApplication()
    : point_cloud_debug_render_(false),
      last_gpu_timestamp_(0.0),
      opengl_world_T_start_service_(
          tango_gl::conversions::opengl_world_T_tango_world()),
      color_camera_T_opengl_camera_(
          tango_gl::conversions::color_camera_T_opengl_camera()) {}

PlaneFittingApplication::~PlaneFittingApplication() {
  TangoConfig_free(tango_config_);
  TangoSupport_freePointCloudManager(point_cloud_manager_);
  point_cloud_manager_ = nullptr;
}

int PlaneFittingApplication::TangoInitialize(JNIEnv* env,
                                             jobject caller_activity) {
  // The first thing we need to do for any Tango enabled application is to
  // initialize the service. We will do that here, passing on the JNI
  // environment and jobject corresponding to the Android activity that is
  // calling us.
  int ret = TangoService_initialize(env, caller_activity);
  if(ret != TANGO_SUCCESS) {
    return ret;
  }

  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if(tango_config_ == nullptr) {
    LOGE("Unable to get tango config");
    return TANGO_ERROR;
  }

  if (point_cloud_manager_ == nullptr) {
    int32_t max_point_cloud_elements;
    ret = TangoConfig_getInt32(tango_config_, "max_point_cloud_elements",
                                         &max_point_cloud_elements);
    if(ret != TANGO_SUCCESS) {
      LOGE("Failed to query maximum number of point cloud elements.");
      return ret;
    }

    ret = TangoSupport_createPointCloudManager(max_point_cloud_elements,
                                          &point_cloud_manager_);
    if(ret != TANGO_SUCCESS) {
      return ret;
    }
  }

  return TANGO_SUCCESS;
}

int PlaneFittingApplication::TangoSetupAndConnect() {
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
    LOGE("PlaneFittingApplication: Failed to connect to the Tango service.");
    return ret;
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
  }

  constexpr float kNearPlane = 0.1;
  constexpr float kFarPlane = 100.0;

  projection_matrix_ar_ = tango_gl::Camera::ProjectionMatrixForCameraIntrinsics(
      color_camera_intrinsics_.width, color_camera_intrinsics_.height,
      color_camera_intrinsics_.fx, color_camera_intrinsics_.fy,
      color_camera_intrinsics_.cx, color_camera_intrinsics_.cy, kNearPlane,
      kFarPlane);

  // Setup fixed pose information
  TangoPoseData pose_imu_T_color_t0;
  TangoPoseData pose_imu_T_depth_t0;
  TangoPoseData pose_imu_T_device_t0;

  TangoCoordinateFramePair frame_pair;
  glm::vec3 translation;
  glm::quat rotation;

  // We need to get the extrinsic transform between the color camera and the
  // IMU coordinate frames. This matrix is then used to compute the extrinsic
  // transform between color camera and device: C_T_D = C_T_IMU * IMU_T_D.
  // Note that the matrix C_T_D is a constant transformation since the hardware
  // will not change, we use the getPoseAtTime() function to query it once right
  // after the Tango Service connected and store it for efficiency.
  frame_pair.base = TANGO_COORDINATE_FRAME_IMU;
  frame_pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  ret = TangoService_getPoseAtTime(0.0, frame_pair, &pose_imu_T_device_t0);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PlaneFittingApplication: Failed to get transform between the IMU and"
        "device frames. Something is wrong with device extrinsics.");
    return ret;
  }
  const glm::mat4 IMU_T_device = tango_gl::conversions::TransformFromArrays(
      pose_imu_T_device_t0.translation, pose_imu_T_device_t0.orientation);

  // Get color camera with respect to IMU transformation matrix. This matrix is
  // used to compute the extrinsics between color camera and device:
  // C_T_D = C_T_IMU * IMU_T_D.
  frame_pair.base = TANGO_COORDINATE_FRAME_IMU;
  frame_pair.target = TANGO_COORDINATE_FRAME_CAMERA_COLOR;
  ret = TangoService_getPoseAtTime(0.0, frame_pair, &pose_imu_T_color_t0);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PlaneFittingApplication: Failed to get transform between the IMU and"
        "camera frames. Something is wrong with device extrinsics.");
    return ret;
  }
  const glm::mat4 IMU_T_color = tango_gl::conversions::TransformFromArrays(
      pose_imu_T_color_t0.translation, pose_imu_T_color_t0.orientation);

  // Get depth camera with respect to imu transformation matrix. This matrix is
  // used to compute the extrinsics between depth camera and device:
  // C_T_D = C_T_IMU * IMU_T_D.
  frame_pair.base = TANGO_COORDINATE_FRAME_IMU;
  frame_pair.target = TANGO_COORDINATE_FRAME_CAMERA_DEPTH;
  ret = TangoService_getPoseAtTime(0.0, frame_pair, &pose_imu_T_depth_t0);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PlaneFittingApplication: Failed to get transform between the IMU and"
        "camera frames. Something is wrong with device extrinsics.");
    return ret;
  }
  const glm::mat4 IMU_T_depth = tango_gl::conversions::TransformFromArrays(
      pose_imu_T_depth_t0.translation, pose_imu_T_depth_t0.orientation);

  device_T_depth_ = glm::inverse(IMU_T_device) * IMU_T_depth;
  device_T_color_ = glm::inverse(IMU_T_device) * IMU_T_color;

  return ret;
}

void PlaneFittingApplication::TangoDisconnect() {
  TangoService_disconnect();
}

int PlaneFittingApplication::InitializeGLContent() {
  video_overlay_ = new tango_gl::VideoOverlay();
  point_cloud_renderer_ = new PointCloudRenderer();
  cube_ = new tango_gl::Cube();
  cube_->SetScale(glm::vec3(kCubeScale, kCubeScale, kCubeScale));
  cube_->SetColor(0.7f, 0.7f, 0.7f);

  // The Tango service allows you to connect an OpenGL texture directly to its
  // RGB and fisheye cameras. This is the most efficient way of receiving
  // images from the service because it avoids copies. You get access to the
  // graphic buffer directly. As we are interested in rendering the color image
  // in our render loop, we will be polling for the color image as needed.
  return TangoService_connectTextureId(
      TANGO_CAMERA_COLOR, video_overlay_->GetTextureId(), this, nullptr);
}

void PlaneFittingApplication::SetRenderDebugPointCloud(bool on) {
  point_cloud_renderer_->SetRenderDebugColors(on);
}

void PlaneFittingApplication::SetViewPort(int width, int height) {
  screen_width_ = static_cast<float>(width);
  screen_height_ = static_cast<float>(height);

  glViewport(0, 0, screen_width_, screen_height_);
}

void PlaneFittingApplication::Render() {
  // We need to make sure that we update the texture associated with the color
  // image.
  if (TangoService_updateTexture(TANGO_CAMERA_COLOR, &last_gpu_timestamp_) !=
      TANGO_SUCCESS) {
    LOGE("PlaneFittingApplication: Failed to get a color image.");
    return;
  }

  // Querying the GPU color image's frame transformation based its timestamp.
  TangoPoseData pose_start_service_T_color_gpu;
  TangoCoordinateFramePair color_gpu_frame_pair;
  color_gpu_frame_pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  color_gpu_frame_pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  if (TangoService_getPoseAtTime(last_gpu_timestamp_, color_gpu_frame_pair,
                                 &pose_start_service_T_color_gpu) !=
      TANGO_SUCCESS) {
    LOGE(
        "PlaneFittingApplication: Could not find a valid pose at time %lf"
        " for the color camera.",
        last_gpu_timestamp_);
  }

  if (pose_start_service_T_color_gpu.status_code == TANGO_POSE_VALID) {
    const glm::mat4 start_service_T_device =
        tango_gl::conversions::TransformFromArrays(
            pose_start_service_T_color_gpu.translation,
            pose_start_service_T_color_gpu.orientation);

    const glm::mat4 start_service_T_color_camera =
        start_service_T_device * device_T_color_;

    GLRender(start_service_T_color_camera);
  } else {
    LOGE("Invalid pose for gpu color image at time: %lf", last_gpu_timestamp_);
  }
}

void PlaneFittingApplication::GLRender(
    const glm::mat4& start_service_T_color_camera) {
  glEnable(GL_CULL_FACE);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // We want to render from the perspective of the device, so we will set our
  // camera based on the transform that was passed in.
  glm::mat4 opengl_camera_T_ss = glm::inverse(start_service_T_color_camera *
                                              color_camera_T_opengl_camera_);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  video_overlay_->Render(glm::mat4(1.0), glm::mat4(1.0));
  glEnable(GL_DEPTH_TEST);
  UpdateCurrentPointData();
  const glm::mat4 start_service_T_device_t1 = GetStartServiceTDeviceTransform();
  const glm::mat4 projection_T_depth = projection_matrix_ar_ * opengl_camera_T_ss *
                            start_service_T_device_t1 * device_T_depth_;
  const glm::mat4 start_service_T_depth = start_service_T_device_t1 * device_T_depth_;
  point_cloud_renderer_->Render(projection_T_depth, start_service_T_depth, front_cloud_);
  glDisable(GL_BLEND);

  glm::mat4 opengl_camera_T_opengl_world =
      opengl_camera_T_ss *
      glm::inverse(tango_gl::conversions::opengl_world_T_tango_world());
  cube_->Render(projection_matrix_ar_, opengl_camera_T_opengl_world);
}

void PlaneFittingApplication::DeleteResources() {
  delete video_overlay_;
  delete cube_;
  video_overlay_ = nullptr;
  cube_ = nullptr;
}

// We assume the Java layer ensures this function is called on the GL thread.
void PlaneFittingApplication::OnTouchEvent(float x, float y) {
  /// Calculate the conversion from the latest depth camera position to the
  /// position of the most recent color camera image. This corrects for screen
  /// lag between the two systems. 
  TangoPoseData pose_color_camera_t0_T_depth_camera_t1;
  int ret = TangoSupport_calculateRelativePose(
      last_gpu_timestamp_, TANGO_COORDINATE_FRAME_CAMERA_COLOR,
      front_cloud_->timestamp, TANGO_COORDINATE_FRAME_CAMERA_DEPTH,
      &pose_color_camera_t0_T_depth_camera_t1);
  if (ret != TANGO_SUCCESS) {
    LOGE("%s: could not calculate relative pose", __func__);
    return;
  }
  glm::vec2 uv(x / screen_width_, y / screen_height_);

  glm::dvec3 double_depth_position;
  glm::dvec4 double_depth_plane_equation;
  if (TangoSupport_fitPlaneModelNearClick(
          front_cloud_, &color_camera_intrinsics_,
          &pose_color_camera_t0_T_depth_camera_t1, glm::value_ptr(uv),
          glm::value_ptr(double_depth_position),
          glm::value_ptr(double_depth_plane_equation)) != TANGO_SUCCESS) {
    return;  // Assume error has already been reported.
  }

  const glm::vec3 depth_position =
      static_cast<glm::vec3>(double_depth_position);
  const glm::vec4 depth_plane_equation =
      static_cast<glm::vec4>(double_depth_plane_equation);

  const glm::mat4 opengl_world_T_depth = opengl_world_T_start_service_ *
                                                GetStartServiceTDeviceTransform() *
                                                device_T_depth_;

  // Transform to world coordinates
  const glm::vec4 world_position =
      opengl_world_T_depth * glm::vec4(depth_position, 1.0f);

  glm::vec4 world_plane_equation;
  PlaneTransform(depth_plane_equation, opengl_world_T_depth,
                 &world_plane_equation);

  point_cloud_renderer_->SetPlaneEquation(world_plane_equation);

  const glm::vec3 plane_normal(world_plane_equation);

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
  cube_->SetPosition(glm::vec3(world_position) + plane_normal * kCubeScale);
}

glm::mat4 PlaneFittingApplication::GetStartServiceTDeviceTransform() {
  TangoCoordinateFramePair frame_pair;
  frame_pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  frame_pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  TangoPoseData pose_start_service_T_device_t1;

  TangoService_getPoseAtTime(front_cloud_->timestamp, frame_pair,
                             &pose_start_service_T_device_t1);

  return tango_gl::conversions::TransformFromArrays(
      pose_start_service_T_device_t1.translation,
      pose_start_service_T_device_t1.orientation);
}

void PlaneFittingApplication::UpdateCurrentPointData() {
  TangoSupport_getLatestPointCloud(point_cloud_manager_,
                                            &front_cloud_);
}

}  // namespace tango_plane_fitting
