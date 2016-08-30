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
#include <string>

#include <tango_client_api.h>  // NOLINT
#include <tango-gl/util.h>
#include <tango_support_api.h>

#include <tango-point-cloud/scene.h>

namespace tango_point_cloud {

// PointCloudApp handles the application lifecycle and resources.
class PointCloudApp {
 public:
  // Constructor and deconstructor.
  PointCloudApp();
  ~PointCloudApp();

  // OnCreate() callback is called when this Android application's
  // OnCreate function is called from UI thread. In the OnCreate
  // function, we are only checking the Tango Core's version.
  //
  // @param env, java environment parameter OnCreate is being called.
  // @param caller_activity, caller of this function.
  void OnCreate(JNIEnv* env, jobject caller_activity);

  // OnPause() callback is called when this Android application's
  // OnCreate function is called from UI thread. In our application,
  // we disconnect Tango Service and free the Tango configuration
  // file. It is important to disconnect Tango Service and release
  // the coresponding resources in the OnPause() callback from
  // Android, otherwise, this application will hold on to the Tango
  // resources and other application will not be able to connect to
  // Tango Service.
  void OnPause();

  // Called when Tango Service is connected successfully.
  void OnTangoServiceConnected(JNIEnv* env, jobject binder);

  // Explicitly reset motion tracking and restart the pipeline.
  // Note that this will cause motion tracking to re-initialize.
  void TangoResetMotionTracking();

  // Tango Service point cloud callback function for depth data. Called when new
  // new point cloud data is available from the Tango Service.
  //
  // @param pose: The current point cloud returned by the service,
  //              caller allocated.
  void onPointCloudAvailable(const TangoPointCloud* point_cloud);

  // Allocate OpenGL resources for rendering, mainly for initializing the Scene.
  void OnSurfaceCreated();

  // Setup the view port width and height.
  void OnSurfaceChanged(int width, int height);

  // Main render loop.
  void OnDrawFrame();

  // Return total point count in the current depth frame.
  int GetPointCloudVerticesCount();

  // Return the average depth of points in the current depth frame.
  float GetAverageZ();
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

  // Set screen rotation index.
  //
  // @param screen_roatation: the screen rotation index,
  //    the index is following Android screen rotation enum.
  //    see Android documentation for detail:
  //    http://developer.android.com/reference/android/view/Surface.html#ROTATION_0
  void SetScreenRotation(int rotation_index);

 private:
  // Update the current point data.
  void UpdateCurrentPointData();

  // Setup the configuration file for the Tango Service.
  void TangoSetupConfig();

  // Connect the onPoseAvailable callback.
  void TangoConnectCallbacks();

  // Connect to Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start Motion Tracking and Depth Sensing callbacks.
  void TangoConnect();

  // Disconnect from Tango Service, release all the resources that the app is
  // holding from the Tango Service.
  void TangoDisconnect();

  // Release all non-OpenGL allocated resources.
  void DeleteResources();

  // Point data manager.
  TangoSupportPointCloudManager* point_cloud_manager_;
  TangoPointCloud* front_cloud_;
  float point_cloud_average_depth_;

  // main_scene_ includes all drawable object for visualizing Tango device's
  // movement and point cloud.
  Scene main_scene_;

  // Tango configration file, this object is for configuring Tango Service setup
  // before connect to service. For example, we turn on the depth sensing in
  // this example.
  TangoConfig tango_config_;

  // Last valid transforms.
  glm::mat4 start_service_T_device_;
  glm::mat4 start_service_opengl_T_depth_tango_;

  // Screen rotation index.
  int screen_rotation_;

  bool is_service_connected_;
};
}  // namespace tango_point_cloud

#endif  // TANGO_POINT_CLOUD_POINT_CLOUD_APP_H_
