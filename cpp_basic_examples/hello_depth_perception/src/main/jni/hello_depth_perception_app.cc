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

#include <cstdlib>

#include <tango_support.h>

#include "hello_depth_perception/hello_depth_perception_app.h"

namespace {
// The minimum Tango Core version required from this application.
constexpr int kTangoCoreMinimumVersion = 9377;

// This function logs point cloud data from OnPointCloudAvailable callbacks.
//
// @param context, this will be a pointer to a HelloDepthPerceptionApp
//        instance on which to call callbacks. This parameter is hidden
//        since it is not used.
// @param *point_cloud, point cloud data to log.
void OnPointCloudAvailable(void* /*context*/,
                           const TangoPointCloud* point_cloud) {
  // Number of points in the point cloud.
  float average_depth;

  // Calculate the average depth.
  average_depth = 0;
  // Each xyzc point has 4 coordinates.
  for (size_t i = 0; i < point_cloud->num_points; ++i) {
    average_depth += point_cloud->points[i][2];
  }
  if (point_cloud->num_points) {
    average_depth /= point_cloud->num_points;
  }

  // Log the number of points and average depth.
  LOGI("HelloDepthPerceptionApp: Point count: %d. Average depth (m): %.3f",
       point_cloud->num_points, average_depth);
}
}  // anonymous namespace

namespace hello_depth_perception {

void HelloDepthPerceptionApp::OnCreate(JNIEnv* env, jobject caller_activity) {
  // Check the installed version of the TangoCore. If it is too old, then
  // it will not support the most up to date features.
  int version = 0;
  TangoErrorType err =
      TangoSupport_getTangoVersion(env, caller_activity, &version);
  if (err != TANGO_SUCCESS || version < kTangoCoreMinimumVersion) {
    LOGE(
        "HelloDepthPerceptionApp::OnCreate, Tango Core version is out"
        " of date.");
    std::exit(EXIT_SUCCESS);
  }
}

void HelloDepthPerceptionApp::OnTangoServiceConnected(JNIEnv* env,
                                                      jobject binder) {
  // Associate the service binder to the Tango service.
  if (TangoService_setBinder(env, binder) != TANGO_SUCCESS) {
    LOGE(
        "HelloDepthPerceptionApp::OnTangoServiceConnected,"
        "TangoService_setBinder error");
    std::exit(EXIT_SUCCESS);
  }

  // Here, we'll configure the service to run in the way we want. For this
  // application, we'll start from the default configuration.
  // TANGO_CONFIG_DEFAULT is disabling Depth Perception.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE(
        "HelloDepthPerceptionApp::OnTangoServiceConnected,"
        "TangoService_getConfig error.");
    std::exit(EXIT_SUCCESS);
  }

  // Enable Depth Perception.
  TangoErrorType err =
      TangoConfig_setBool(tango_config_, "config_enable_depth", true);
  if (err != TANGO_SUCCESS) {
    LOGE(
        "HelloDepthPerceptionApp::OnTangoServiceConnected,"
        "config_enable_depth() failed with error code: %d.",
        err);
    std::exit(EXIT_SUCCESS);
  }

  // Need to specify the depth_mode as XYZC.
  err = TangoConfig_setInt32(tango_config_, "config_depth_mode",
                             TANGO_POINTCLOUD_XYZC);
  if (err != TANGO_SUCCESS) {
    LOGE(
        "Failed to set 'depth_mode' configuration flag with error"
        " code: %d",
        err);
    std::exit(EXIT_SUCCESS);
  }

  // Attach the OnPointCloudAvailable callback to the OnPointCloudAvailable
  // function defined above. The callback will be called every time a new
  // point cloud is acquired, after the service is connected.
  err = TangoService_connectOnPointCloudAvailable(OnPointCloudAvailable);
  if (err != TANGO_SUCCESS) {
    LOGE(
        "HelloDepthPerceptionApp::OnTangoServiceConnected,"
        "Failed to connect to point cloud callback with error code: %d",
        err);
    std::exit(EXIT_SUCCESS);
  }

  // Connect to the Tango Service, the service will start running:
  // point clouds can be queried and callbacks will be called.
  err = TangoService_connect(this, tango_config_);
  if (err != TANGO_SUCCESS) {
    LOGE(
        "HelloDepthPerceptionApp::OnTangoServiceConnected,"
        "Failed to connect to the Tango service with error code: %d",
        err);
    std::exit(EXIT_SUCCESS);
  }
}

void HelloDepthPerceptionApp::OnPause() {
  // When disconnecting from the Tango Service, it is important to make sure to
  // free your configuration object. Note that disconnecting from the service,
  // resets all configuration, and disconnects all callbacks. If an application
  // resumes after disconnecting, it must re-register configuration and
  // callbacks with the service.
  TangoConfig_free(tango_config_);
  tango_config_ = nullptr;
  TangoService_disconnect();
}
}  // namespace hello_depth_perception
