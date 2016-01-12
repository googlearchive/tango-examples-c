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

#ifndef TANGO_POINT_CLOUD_POINT_CLOUD_APP_H_
#define TANGO_POINT_CLOUD_POINT_CLOUD_APP_H_

#include <jni.h>
#include <memory>

#include <tango_client_api.h>  // NOLINT
#include <tango-gl/util.h>

#include <tango-point-cloud/point_cloud_data.h>
#include <tango-point-cloud/pose_data.h>
#include <tango-point-cloud/scene.h>
#include <tango-point-cloud/tango_event_data.h>


namespace tango_point_cloud {

// PointCloudApp handles the application lifecycle and resources.
class PointCloudApp {
 public:
  // Constructor and deconstructor.
  PointCloudApp();
  ~PointCloudApp();

  // Initialize the Tango Service, this function starts the communication
  // between the application and the Tango Service.
  // The activity object is used for checking if the API version is outdated.
  int TangoInitialize(JNIEnv* env, jobject caller_activity);

  // Setup the configuration file for the Tango Service. We'll also set whether
  // we'd like auto-recover enabled.
  //
  // @param is_auto_reset: set auto reset flag.
  int TangoSetupConfig(bool is_atuo_recovery);

  // Connect the onPoseAvailable callback.
  int TangoConnectCallbacks();

  // Connect to Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start Motion Tracking and Depth Sensing callbacks.
  int TangoConnect();

  // Disconnect from Tango Service, release all the resources that the app is
  // holding from the Tango Service.
  void TangoDisconnect();

  // Explicitly reset motion tracking and restart the pipeline.
  // Note that this will cause motion tracking to re-initialize.
  void TangoResetMotionTracking();

  // Tango Service point cloud callback function for depth data. Called when new
  // new point cloud data is available from the Tango Service.
  //
  // @param pose: The current point cloud returned by the service,
  //              caller allocated.
  void onPointCloudAvailable(const TangoXYZij* xyz_ij);

  // Tango service pose callback function for pose data. Called when new
  // information about device pose is available from the Tango Service.
  //
  // @param pose: The current pose returned by the service, caller allocated.
  void onPoseAvailable(const TangoPoseData* pose);

  // Tango service event callback function for event data. Called when new events
  // are available from the Tango Service.
  //
  // @param event: Tango event, caller allocated.
  void onTangoEventAvailable(const TangoEvent* event);

  // Allocate OpenGL resources for rendering, mainly for initializing the Scene.
  void InitializeGLContent();

  // Setup the view port width and height.
  void SetViewPort(int width, int height);

  // Main render loop.
  void Render();

  // Release all non-OpenGL allocated resources.
  void DeleteResources();

  // Return pose debug string.
  std::string GetPoseString();

  // Retrun Tango event debug string.
  std::string GetEventString();

  // Retrun Tango Service version string.
  std::string GetVersionString();

  // Return total point count in the current depth frame.
  int GetPointCloudVerticesCount();

  // Return the average depth of points in the current depth frame.
  float GetAverageZ();

  // Return the delta time between current and previous depth frames.
  float GetDepthFrameDeltaTime();

  // Set render camera's viewing angle, first person, third person or top down.
  //
  // @param: camera_type, camera type includes first person, third person and
  //         top down
  void SetCameraType(tango_gl::GestureCamera::CameraType camera_type);

  // Touch event passed from android activity. This function only supports two
  // touches.
  //
  // @param: touch_count, total count for touches.
  // @param: event, touch event of current touch.
  // @param: x0, normalized touch location for touch 0 on x axis.
  // @param: y0, normalized touch location for touch 0 on y axis.
  // @param: x1, normalized touch location for touch 1 on x axis.
  // @param: y1, normalized touch location for touch 1 on y axis.
  void OnTouchEvent(int touch_count, tango_gl::GestureCamera::TouchEvent event,
                    float x0, float y0, float x1, float y1);

 private:
  // Get a pose in matrix format with extrinsics in OpenGl space.
  //
  // @param: timstamp, timestamp of the target pose.
  //
  // @return: pose in matrix format.
  glm::mat4 GetPoseMatrixAtTimestamp(double timstamp);

  // Query sensor/camera extrinsic from the Tango Service, the extrinsic is only
  // available after the service is connected.
  //
  // @return: error code.
  TangoErrorType UpdateExtrinsics();

  // point_cloud_ contains the data of current depth frame, it also
  // has the render function to render the points. This instance will be passed
  // to main_scene_ for rendering.
  //
  // point_cloud_ is a thread safe object, the data protection is handled
  // internally inside the PointCloud class.
  PointCloudData point_cloud_data_;

  // Mutex for protecting the point cloud data. The point cloud data is shared
  // between render thread and TangoService callback thread.
  std::mutex point_cloud_mutex_;

  // pose_data_ handles all pose onPoseAvailable callbacks, onPoseAvailable()
  // in this object will be routed to pose_data_ to handle.
  PoseData pose_data_;

  // Mutex for protecting the pose data. The pose data is shared between render
  // thread and TangoService callback thread.
  std::mutex pose_mutex_;

  // tango_event_data_ handles all Tango event callbacks,
  // onTangoEventAvailable() in this object will be routed to tango_event_data_
  // to handle.
  TangoEventData tango_event_data_;

  // tango_event_data_ is share between the UI thread we start for updating
  // debug
  // texts and the TangoService event callback thread. We keep event_mutex_ to
  // protect tango_event_data_.
  std::mutex tango_event_mutex_;

  // main_scene_ includes all drawable object for visualizing Tango device's
  // movement and point cloud.
  Scene main_scene_;

  // Tango configration file, this object is for configuring Tango Service setup
  // before connect to service. For example, we turn on the depth sensing in
  // this example.
  TangoConfig tango_config_;

  // Tango service version string.
  std::string tango_core_version_string_;
};
}  // namespace tango_point_cloud

#endif  // TANGO_POINT_CLOUD_POINT_CLOUD_APP_H_
