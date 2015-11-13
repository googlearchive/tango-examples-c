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

SynchronizationApplication::SynchronizationApplication()
    : color_image_(),
      depth_image_(),
      main_scene_(),
      // We'll store the fixed transform between the opengl frame convention.
      // (Y-up, X-right) and tango frame convention. (Z-up, X-right).
      OW_T_SS_(tango_gl::conversions::opengl_world_T_tango_world()),
      swap_signal(false),
      gpu_upsample_(false) {}

SynchronizationApplication::~SynchronizationApplication() {
  if (tango_config_) {
    TangoConfig_free(tango_config_);
  }
}

int SynchronizationApplication::TangoInitialize(JNIEnv* env,
                                                jobject caller_activity) {
  SetDepthAlphaValue(0.0);
  SetGPUUpsample(false);

  // The first thing we need to do for any Tango enabled application is to
  // initialize the service. We'll do that here, passing on the JNI environment
  // and jobject corresponding to the Android activity that is calling us.
  return TangoService_initialize(env, caller_activity);
}

int SynchronizationApplication::TangoSetupConfig() {
  if (tango_config_ != nullptr) {
    return TANGO_SUCCESS;
  }

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
      TANGO_CAMERA_COLOR, color_image_.GetTextureId(), this, nullptr);
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
  depth_image_.SetCameraIntrinsics(color_camera_intrinsics);
  main_scene_.SetCameraIntrinsics(color_camera_intrinsics);

  // Pose of device frame wrt imu.
  TangoPoseData pose_imu_T_device;
  // Pose of color frame wrt imu.
  TangoPoseData pose_imu_T_color;
  // Pose of depth camera wrt imu.
  TangoPoseData pose_imu_T_depth;
  TangoCoordinateFramePair frame_pair;
  glm::vec3 translation;
  glm::quat rotation;

  // We need to get the extrinsic transform between the color camera and the
  // imu coordinate frames. This matrix is then used to compute the extrinsic
  // transform between color camera and device:
  // color_T_device = color_T_imu * imu_T_device;
  // Note that the matrix color_T_device is a constant transformation since the
  // hardware will not change, we use the getPoseAtTime() function to query it
  // once right after the Tango Service connected and store it for efficiency.
  frame_pair.base = TANGO_COORDINATE_FRAME_IMU;
  frame_pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  ret = TangoService_getPoseAtTime(0.0, frame_pair, &pose_imu_T_device);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "SynchronizationApplication: Failed to get transform between the IMU "
        "and"
        "device frames. Something is wrong with device extrinsics.");
    return ret;
  }
  // Converting pose to transformation matrix.
  glm::mat4 imu_T_device = util::GetMatrixFromPose(&pose_imu_T_device);

  // Get color camera with respect to imu transformation matrix.
  // This matrix is used to compute the extrinsics between color camera and
  // device: color_T_device = color_T_imu * imu_T_device;
  frame_pair.base = TANGO_COORDINATE_FRAME_IMU;
  frame_pair.target = TANGO_COORDINATE_FRAME_CAMERA_COLOR;
  ret = TangoService_getPoseAtTime(0.0, frame_pair, &pose_imu_T_color);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "SynchronizationApplication: Failed to get transform between the IMU "
        "and"
        "color camera frames. Something is wrong with device extrinsics.");
    return ret;
  }
  // Converting pose to transformation matrix.
  glm::mat4 imu_T_color = util::GetMatrixFromPose(&pose_imu_T_color);

  frame_pair.base = TANGO_COORDINATE_FRAME_IMU;
  frame_pair.target = TANGO_COORDINATE_FRAME_CAMERA_DEPTH;
  ret = TangoService_getPoseAtTime(0.0, frame_pair, &pose_imu_T_depth);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "SynchronizationApplication: Failed to get transform between the IMU "
        "and depth camera frames. Something is wrong with device extrinsics.");
    return ret;
  }
  // Converting pose to transformation matrix.
  glm::mat4 imu_T_depth = util::GetMatrixFromPose(&pose_imu_T_depth);

  device_T_color_ = glm::inverse(imu_T_device) * imu_T_color;
  device_T_depth_ = glm::inverse(imu_T_device) * imu_T_depth;

  return ret;
}

void SynchronizationApplication::TangoDisconnect() {
  TangoService_disconnect();
}

void SynchronizationApplication::InitializeGLContent() {
  depth_image_.InitializeGL();
  color_image_.InitializeGL();
  main_scene_.InitializeGL();
}

void SynchronizationApplication::SetViewPort(int width, int height) {
  screen_width_ = static_cast<float>(width);
  screen_height_ = static_cast<float>(height);
  main_scene_.SetupViewPort(width, height);
}

void SynchronizationApplication::Render() {
  double color_timestamp = 0.0;
  double depth_timestamp = 0.0;
  bool new_points = false;
  {
    std::lock_guard<std::mutex> lock(point_cloud_mutex_);
    depth_timestamp = depth_timestamp_;
    if (swap_signal) {
      shared_point_cloud_buffer_.swap(render_point_cloud_buffer_);
      swap_signal = false;
      new_points = true;
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
  TangoPoseData pose_start_service_T_device_t0;
  TangoCoordinateFramePair depth_frame_pair;
  depth_frame_pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  depth_frame_pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  if (TangoService_getPoseAtTime(depth_timestamp, depth_frame_pair,
                                 &pose_start_service_T_device_t0) !=
      TANGO_SUCCESS) {
    LOGE(
        "SynchronizationApplication: Could not find a valid pose at time %lf"
        " for the depth camera.",
        depth_timestamp);
  }

  // Querying the color image's frame transformation based on the depth image's
  // timestamp.
  TangoPoseData pose_start_service_T_device_t1;
  TangoCoordinateFramePair color_frame_pair;
  color_frame_pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  color_frame_pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  if (TangoService_getPoseAtTime(color_timestamp, color_frame_pair,
                                 &pose_start_service_T_device_t1) !=
      TANGO_SUCCESS) {
    LOGE(
        "SynchronizationApplication: Could not find a valid pose at time %lf"
        " for the color camera.",
        color_timestamp);
  }

  // In the following code, we define t0 as the depth timestamp and t1 as the
  // color camera timestamp.
  //
  // Device frame at timestamp t0 (depth timestamp) with respect to start of
  // service.
  glm::mat4 start_service_T_device_t0 =
      util::GetMatrixFromPose(&pose_start_service_T_device_t0);
  // Device frame at timestamp t1 (color timestamp) with respect to start of
  // service.
  glm::mat4 start_service_T_device_t1 =
      util::GetMatrixFromPose(&pose_start_service_T_device_t1);

  // Transformation of depth frame wrt Device at time stamp t0.
  // Transformation of depth frame with respect to the device frame at
  // time stamp t0. This transformation remains constant over time. Here we
  // assign to a local variable to maintain naming consistency when calculating
  // the transform: color_image_t1_T_depth_image_t0.
  glm::mat4 device_t0_T_depth_t0 = device_T_depth_;

  // Transformation of Device Frame wrt Color Image frame at time stamp t1.
  // Transformation of device frame with respect to the color camera frame at
  // time stamp t1. This transformation remains constant over time. Here we
  // assign to a local variable to maintain naming consistency when calculating
  // the transform: color_image_t1_T_depth_image_t0.
  glm::mat4 color_t1_T_device_t1 = glm::inverse(device_T_color_);

  if (pose_start_service_T_device_t1.status_code == TANGO_POSE_VALID) {
    if (pose_start_service_T_device_t0.status_code == TANGO_POSE_VALID) {
      // Note that we are discarding all invalid poses at the moment, another
      // option could be to use the latest pose when the queried pose is
      // invalid.

      // The Color Camera frame at timestamp t0 with respect to Depth
      // Camera frame at timestamp t1.
      glm::mat4 color_image_t1_T_depth_image_t0 =
          color_t1_T_device_t1 * glm::inverse(start_service_T_device_t1) *
          start_service_T_device_t0 * device_t0_T_depth_t0;

      if(gpu_upsample_) {
        depth_image_.RenderDepthToTexture(color_image_t1_T_depth_image_t0,
                                          render_point_cloud_buffer_,
                                          new_points);
      } else {
        depth_image_.UpdateAndUpsampleDepth(color_image_t1_T_depth_image_t0,
                                            render_point_cloud_buffer_);

      }
      main_scene_.Render(color_image_.GetTextureId(),
                         depth_image_.GetTextureId());
    } else {
      LOGE("Invalid pose for ss_t_depth at time: %lf", depth_timestamp);
    }
  } else {
    LOGE("Invalid pose for ss_t_color at time: %lf", color_timestamp);
  }
}

void SynchronizationApplication::SetDepthAlphaValue(float alpha) {
  main_scene_.SetDepthAlphaValue(alpha);
}

void SynchronizationApplication::SetGPUUpsample(bool on) {
  gpu_upsample_ = on;
}

}  // namespace rgb_depth_sync
