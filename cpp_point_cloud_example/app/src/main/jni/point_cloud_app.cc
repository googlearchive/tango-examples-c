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

// This function routes onPoseAvailable callbacks to the application object for
// handling.
//
// @param context, context will be a pointer to a PointCloudApp
//        instance on which to call callbacks.
// @param pose, pose data to route to onPoseAvailable function.
void onPoseAvailableRouter(void* context, const TangoPoseData* pose) {
  tango_point_cloud::PointCloudApp* app =
      static_cast<tango_point_cloud::PointCloudApp*>(context);
  app->onPoseAvailable(pose);
}
}  // namespace

namespace tango_point_cloud {
void PointCloudApp::onPointCloudAvailable(const TangoXYZij* xyz_ij) {
  std::lock_guard<std::mutex> lock(point_cloud_mutex_);
  point_cloud_data_.UpdatePointCloud(xyz_ij);
}

void PointCloudApp::onPoseAvailable(const TangoPoseData* pose) {
  std::lock_guard<std::mutex> lock(pose_mutex_);
  pose_data_.UpdatePose(pose);
}

PointCloudApp::PointCloudApp() {}

PointCloudApp::~PointCloudApp() {
  if (tango_config_ != nullptr) {
    TangoConfig_free(tango_config_);
  }
}

bool PointCloudApp::CheckTangoVersion(JNIEnv* env, jobject activity,
                                      int min_tango_version) {
  // Check the installed version of the TangoCore.  If it is too old, then
  // it will not support the most up to date features.
  int version;
  TangoErrorType err = TangoSupport_GetTangoVersion(env, activity, &version);
  return err == TANGO_SUCCESS && version >= min_tango_version;
}

int PointCloudApp::TangoSetupConfig() {
  // Here, we'll configure the service to run in the way we'd want. For this
  // application, we'll start from the default configuration
  // (TANGO_CONFIG_DEFAULT). This enables basic motion tracking capabilities.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE("PointCloudApp: Failed to get default config form");
    return TANGO_ERROR;
  }

  // Set auto-recovery for motion tracking as requested by the user.
  int ret =
      TangoConfig_setBool(tango_config_, "config_enable_auto_recovery", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointCloudApp: config_enable_auto_recovery() failed with error"
        "code: %d",
        ret);
    return ret;
  }

  // Enable depth.
  ret = TangoConfig_setBool(tango_config_, "config_enable_depth", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointCloudApp: config_enable_depth() failed with error"
        "code: %d",
        ret);
    return ret;
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
    return ret;
  }

  // Setting up the frame pair for the onPoseAvailable callback.
  TangoCoordinateFramePair pairs;
  pairs.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  pairs.target = TANGO_COORDINATE_FRAME_DEVICE;

  // Attach the onPoseAvailable callback.
  // The callback will be called after the service is connected.
  ret = TangoService_connectOnPoseAvailable(1, &pairs, onPoseAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointCloudApp: Failed to connect to pose callback with error"
        "code: %d",
        ret);
    return ret;
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
    return false;
  }

  err = UpdateExtrinsics();
  if (err != TANGO_SUCCESS) {
    LOGE("PointCloudApp: Failed to query sensor extrinsic with error code: %d",
         err);
    return false;
  }
  return true;
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
  // TangoService_getPoseAtTime() to query pose at timestamp.
  glm::mat4 cur_pose_transformation;
  glm::mat4 point_cloud_transformation;
  {
    std::lock_guard<std::mutex> lock(pose_mutex_);
    cur_pose_transformation = pose_data_.GetLatestPoseMatrix();
  }

  double point_cloud_timestamp;
  // We make another copy for rendering and depth computation.
  std::vector<float> vertices_cpy;
  {
    std::lock_guard<std::mutex> lock(point_cloud_mutex_);
    point_cloud_timestamp = point_cloud_data_.GetCurrentTimstamp();
    std::vector<float> vertices = point_cloud_data_.GetVerticeVector();
    vertices_cpy = std::vector<float>(vertices);
  }

  // Get the latest pose transformation in opengl frame and apply extrinsics to
  // it.
  cur_pose_transformation =
      pose_data_.GetExtrinsicsAppliedOpenGLWorldFrame(cur_pose_transformation);

  // Query pose based on point cloud frame's timestamp.
  point_cloud_transformation = GetPoseMatrixAtTimestamp(point_cloud_timestamp);
  // Get the point cloud transformation in opengl frame and apply extrinsics to
  // it.
  point_cloud_transformation = pose_data_.GetExtrinsicsAppliedOpenGLWorldFrame(
      point_cloud_transformation);

  // Compute the average depth value.
  float average_depth_ = 0.0f;
  size_t iteration_size = vertices_cpy.size();
  size_t vertices_count = 0;
  for (size_t i = 2; i < iteration_size; i += 3) {
    average_depth_ += vertices_cpy[i];
    vertices_count++;
  }
  if (vertices_count) {
    average_depth_ /= vertices_count;
  }

  {
    std::lock_guard<std::mutex> lock(point_cloud_mutex_);
    point_cloud_data_.SetAverageDepth(average_depth_);
  }

  main_scene_.Render(cur_pose_transformation, point_cloud_transformation,
                     vertices_cpy);
}

void PointCloudApp::DeleteResources() { main_scene_.DeleteResources(); }

int PointCloudApp::GetPointCloudVerticesCount() {
  std::lock_guard<std::mutex> lock(point_cloud_mutex_);
  return point_cloud_data_.GetPointCloudVerticesCount();
}

float PointCloudApp::GetAverageZ() {
  std::lock_guard<std::mutex> lock(point_cloud_mutex_);
  return point_cloud_data_.GetAverageDepth();
}

void PointCloudApp::SetCameraType(
    tango_gl::GestureCamera::CameraType camera_type) {
  main_scene_.SetCameraType(camera_type);
}

void PointCloudApp::OnTouchEvent(int touch_count,
                                 tango_gl::GestureCamera::TouchEvent event,
                                 float x0, float y0, float x1, float y1) {
  main_scene_.OnTouchEvent(touch_count, event, x0, y0, x1, y1);
}

glm::mat4 PointCloudApp::GetPoseMatrixAtTimestamp(double timstamp) {
  TangoPoseData pose_start_service_T_device;
  TangoCoordinateFramePair frame_pair;
  frame_pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  frame_pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  TangoErrorType status = TangoService_getPoseAtTime(
      timstamp, frame_pair, &pose_start_service_T_device);
  if (status != TANGO_SUCCESS) {
    LOGE(
        "PoseData: Failed to get transform between the Start of service and "
        "device frames at timstamp %lf",
        timstamp);
  }
  if (pose_start_service_T_device.status_code != TANGO_POSE_VALID) {
    return glm::mat4(1.0f);
  }
  return pose_data_.GetMatrixFromPose(pose_start_service_T_device);
}

TangoErrorType PointCloudApp::UpdateExtrinsics() {
  TangoErrorType ret;
  TangoPoseData pose_data;
  TangoCoordinateFramePair frame_pair;

  // TangoService_getPoseAtTime function is used for query device extrinsics
  // as well. We use timestamp 0.0 and the target frame pair to get the
  // extrinsics from the sensors.
  //
  // Get device with respect to imu transformation matrix.
  frame_pair.base = TANGO_COORDINATE_FRAME_IMU;
  frame_pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  ret = TangoService_getPoseAtTime(0.0, frame_pair, &pose_data);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointCloudApp: Failed to get transform between the IMU frame and "
        "device frames");
    return ret;
  }
  pose_data_.SetImuTDevice(pose_data_.GetMatrixFromPose(pose_data));

  // Get color camera with respect to imu transformation matrix.
  frame_pair.base = TANGO_COORDINATE_FRAME_IMU;
  frame_pair.target = TANGO_COORDINATE_FRAME_CAMERA_DEPTH;
  ret = TangoService_getPoseAtTime(0.0, frame_pair, &pose_data);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "PointCloudApp: Failed to get transform between the color camera frame "
        "and device frames");
    return ret;
  }
  pose_data_.SetImuTDepthCamera(pose_data_.GetMatrixFromPose(pose_data));
  return ret;
}

}  // namespace tango_point_cloud
