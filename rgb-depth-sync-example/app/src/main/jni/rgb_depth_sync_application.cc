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

#include <rgb-depth-sync/rgb_depth_sync_application.h>

namespace rgb_depth_sync {

// This function will route callbacks to our application object via the context
// parameter.
// @param context Will be a pointer to a SynchronizationApplication instance  on
// which to call callbacks.
// @param xyz_ij The point cloud to pass on.
void OnXYZijAvailableRouter(void* context, const TangoXYZij* xyz_ij) {
  SynchronizationApplication* app =
      static_cast<SynchronizationApplication*>(context);
  app->OnXYZijAvailable(xyz_ij);
}

void SynchronizationApplication::OnXYZijAvailable(const TangoXYZij* xyz_ij) {
  // We'll just update the point cloud associated with our depth image.
  size_t point_cloud_size = xyz_ij->xyz_count * 3;
  callback_point_cloud_buffer_.resize(point_cloud_size);
  std::copy(xyz_ij->xyz[0], xyz_ij->xyz[0] + point_cloud_size,
            callback_point_cloud_buffer_.begin());
  {
    std::lock_guard<std::mutex> lock(point_cloud_mutex_);
    depth_timestamp_ = xyz_ij->timestamp;
    callback_point_cloud_buffer_.swap(shared_point_cloud_buffer_);
    swap_signal = true;
  }
}

SynchronizationApplication::SynchronizationApplication() {
  // We'll store the fixed transform between the opengl frame convention.
  // (Y-up, X-right) and tango frame convention. (Z-up, X-right).
  OW_T_SS_ = tango_gl::conversions::opengl_world_T_tango_world();
}

SynchronizationApplication::~SynchronizationApplication() {}

int SynchronizationApplication::TangoInitialize(JNIEnv* env,
                                                jobject caller_activity) {
  // The first thing we need to do for any Tango enabled application is to
  // initialize the service. We'll do that here, passing on the JNI environment
  // and jobject corresponding to the Android activity that is calling us.
  return TangoService_initialize(env, caller_activity);
}

int SynchronizationApplication::TangoSetupConfig() {
  // Here, we'll configure the service to run in the way we'd want. For this
  // application, we'll start from the default configuration
  // (TANGO_CONFIG_DEFAULT). This enables basic motion tracking capabilities.
  // In addition to motion tracking, however, we want to run with depth so that
  // we can sync Image data with Depth Data. As such, we're going to set an
  // additional flag "config_enable_depth" to true.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    return TANGO_ERROR;
  }

  TangoErrorType ret =
      TangoConfig_setBool(tango_config_, "config_enable_depth", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable depth.");
    return ret;
  }

  // Note that it's super important for AR applications that we enable low
  // latency imu integration so that we have pose information available as
  // quickly as possible. Without setting this flag, you'll often receive
  // invalid poses when calling GetPoseAtTime for an image.
  ret = TangoConfig_setBool(tango_config_,
                            "config_enable_low_latency_imu_integration", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable low latency imu integration.");
    return ret;
  }
  return ret;
}

int SynchronizationApplication::TangoConnectTexture() {
  // The Tango service allows you to connect an OpenGL texture directly to its
  // RGB and fisheye cameras. This is the most efficient way of receiving
  // images from the service because it avoids copies. You get access to the
  // graphic buffer directly. As we're interested in rendering the color image
  // in our render loop, we'll be polling for the color image as needed.
  return TangoService_connectTextureId(
      TANGO_CAMERA_COLOR, color_image_->GetTextureId(), this, nullptr);
}

int SynchronizationApplication::TangoConnectCallbacks() {
  // We're interested in only one callback for this application. We need to be
  // notified when we receive depth information in order to support measuring
  // 3D points. For both pose and color camera information, we'll be polling.
  // The render loop will drive the rate at which we need color images and all
  // our poses will be driven by timestamps. As such, we'll use GetPoseAtTime.
  TangoErrorType depth_ret =
      TangoService_connectOnXYZijAvailable(OnXYZijAvailableRouter);
  return depth_ret;
}

int SynchronizationApplication::TangoConnect() {
  // Here, we'll connect to the TangoService and set up to run. Note that we're
  // passing in a pointer to ourselves as the context which will be passed back
  // in our callbacks.
  TangoErrorType ret = TangoService_connect(this, tango_config_);
  if (ret != TANGO_SUCCESS) {
    LOGE("SynchronizationApplication: Failed to connect to the Tango service.");
  }
  return ret;
}
int SynchronizationApplication::TangoSetIntrinsicsAndExtrinsics() {
  // Get the intrinsics for the color camera and pass them on to the depth
  // image. We need these to know how to project the point cloud into the color
  // camera frame.
  TangoCameraIntrinsics color_camera_intrinsics;
  TangoErrorType ret = TangoService_getCameraIntrinsics(
      TANGO_CAMERA_COLOR, &color_camera_intrinsics);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "SynchronizationApplication: Failed to get the intrinsics for the color"
        "camera.");
    return ret;
  }
  depth_image_->SetCameraIntrinsics(color_camera_intrinsics);
  main_scene_->SetCameraIntrinsics(color_camera_intrinsics);

  float image_width = static_cast<float>(color_camera_intrinsics.width);
  float image_height = static_cast<float>(color_camera_intrinsics.height);
  float image_plane_ratio = image_height / image_width;
  float screen_ratio = screen_height_ / screen_width_;

  if (image_plane_ratio < screen_ratio) {
    glViewport(0, 0, screen_width_, screen_width_ * image_plane_ratio);
  } else {
    glViewport((screen_width_ - screen_height_ / image_plane_ratio) / 2, 0,
               screen_height_ / image_plane_ratio, screen_height_);
  }

  TangoPoseData pose_imu_T_device_t0;
  TangoPoseData pose_imu_T_color_t0;
  TangoCoordinateFramePair frame_pair;
  glm::vec3 translation;
  glm::quat rotation;

  // We need to get the extrinsic transform between the color camera and the
  // imu coordinate frames. This matrix is then used to compute the extrinsic
  // transform between color camera and device: C_T_D = C_T_IMU * IMU_T_D;
  // Note that the matrix C_T_D is a constant transformation since the hardware
  // will not change, we use the getPoseAtTime() function to query it once right
  // after the Tango Service connected and store it for efficiency.
  frame_pair.base = TANGO_COORDINATE_FRAME_IMU;
  frame_pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  ret = TangoService_getPoseAtTime(0.0, frame_pair, &pose_imu_T_device_t0);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "SynchronizationApplication: Failed to get transform between the IMU "
        "and"
        "device frames. Something is wrong with device extrinsics.");
    return ret;
  }

  glm::mat4 IMU_T_D = util::GetMatrixFromPose(&pose_imu_T_device_t0);

  // Get color camera with respect to imu transformation matrix.
  // This matrix is used to compute the extrinsics between color camera and
  // device: C_T_D = C_T_IMU * IMU_T_D;
  frame_pair.base = TANGO_COORDINATE_FRAME_IMU;
  frame_pair.target = TANGO_COORDINATE_FRAME_CAMERA_COLOR;
  ret = TangoService_getPoseAtTime(0.0, frame_pair, &pose_imu_T_color_t0);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "SynchronizationApplication: Failed to get transform between the IMU "
        "and"
        "camera frames. Something is wrong with device extrinsics.");
    return ret;
  }

  glm::mat4 IMU_T_C = util::GetMatrixFromPose(&pose_imu_T_color_t0);

  // Here we are getting matrix for extrinsics of depth camera and color camera.
  // Even though there are on different timestamp frame, but there are constant,
  // so we could simply inverse one of them to get another one.
  Di_T_Ci_ = Dj_T_Cj_ = glm::inverse(IMU_T_D) * IMU_T_C;
  Ci_T_Di_ = Cj_T_Dj_ = glm::inverse(Dj_T_Cj_);

  return ret;
}

void SynchronizationApplication::TangoDisconnect() {
  TangoConfig_free(tango_config_);
  tango_config_ = nullptr;
  TangoService_disconnect();
}

void SynchronizationApplication::InitializeGLContent() {
  depth_image_ = new rgb_depth_sync::DepthImage();
  color_image_ = new rgb_depth_sync::ColorImage();
  main_scene_ = new rgb_depth_sync::Scene(color_image_, depth_image_);
}

void SynchronizationApplication::SetViewPort(int width, int height) {
  screen_width_ = static_cast<float>(width);
  screen_height_ = static_cast<float>(height);
  main_scene_->SetupViewPort(width, height);
}

void SynchronizationApplication::Render() {

  double color_timestamp = 0.0;
  double depth_timestamp = 0.0;
  {
    std::lock_guard<std::mutex> lock(point_cloud_mutex_);
    depth_timestamp = depth_timestamp_;
    if (swap_signal) {
      shared_point_cloud_buffer_.swap(render_point_cloud_buffer_);
      swap_signal = false;
    }
  }
  // We need to make sure that we update the texture associated with the color
  // image.
  if (TangoService_updateTexture(TANGO_CAMERA_COLOR, &color_timestamp) !=
      TANGO_SUCCESS) {
    LOGE("SynchronizationApplication: Failed to get a color image.");
  }

  // Querying the depth image's frame transformation based on the depth image's
  // timestamp.
  TangoPoseData pose_ss_T_devicej;
  TangoCoordinateFramePair depth_frame_pair;
  depth_frame_pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  depth_frame_pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  if (TangoService_getPoseAtTime(depth_timestamp, depth_frame_pair,
                                 &pose_ss_T_devicej) != TANGO_SUCCESS) {
    LOGE(
        "SynchronizationApplication: Could not find a valid pose at time %lf"
        " for the depth camera.",
        depth_timestamp);
  }

  // Querying the color image's frame transformation based on the depth image's
  // timestamp.
  TangoPoseData pose_ss_T_devicei;
  TangoCoordinateFramePair color_frame_pair;
  color_frame_pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  color_frame_pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  if (TangoService_getPoseAtTime(color_timestamp, color_frame_pair,
                                 &pose_ss_T_devicei) != TANGO_SUCCESS) {
    LOGE(
        "SynchronizationApplication: Could not find a valid pose at time %lf"
        " for the color camera.",
        color_timestamp);
  }

  // In the following code, we define j as the depth timestamp and i as the
  // color camera timestamp.
  //
  // Device frame at timestamp j (depth timestamp) with respect to start of
  // service.
  glm::mat4 SS_T_Dj = util::GetMatrixFromPose(&pose_ss_T_devicej);
  // Device frame at timestamp i (color timestamp) with respect to start of
  // service.
  glm::mat4 SS_T_Di = util::GetMatrixFromPose(&pose_ss_T_devicei);
  // The Camera frame at timestamp j with respect to Camera frame at timestamp
  // i.
  glm::mat4 Ci_T_Cj;

  if (pose_ss_T_devicei.status_code == TANGO_POSE_VALID) {
    if (pose_ss_T_devicej.status_code == TANGO_POSE_VALID) {
      // Note that we are discarding all invalid poses at the moment, another
      // option could be to use the latest pose when the queried pose is
      // invalid.
      Ci_T_Cj = Ci_T_Di_ * glm::inverse(SS_T_Di) * SS_T_Dj * Dj_T_Cj_;

      depth_image_->UpdateAndUpsampleDepth(Ci_T_Cj, render_point_cloud_buffer_);
    } else {
      LOGE("Invalid pose for ss_t_depth at time: %lf", depth_timestamp);
    }
  } else {
    LOGE("Invalid pose for ss_t_color at time: %lf", color_timestamp);
  }
  main_scene_->Render();
}

void SynchronizationApplication::FreeGLContent() {
  delete color_image_;
  delete depth_image_;
  delete main_scene_;
}

void SynchronizationApplication::SetDepthAlphaValue(float alpha) {
  main_scene_->SetDepthAlphaValue(alpha);
}

}  // namespace rgb_depth_sync
