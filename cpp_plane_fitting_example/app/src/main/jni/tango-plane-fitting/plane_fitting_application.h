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

#ifndef TANGO_PLANE_FITTING_PLANE_FITTING_APPLICATION_H_
#define TANGO_PLANE_FITTING_PLANE_FITTING_APPLICATION_H_

#include <jni.h>

#include <atomic>
#include <tango_client_api.h>
#include <tango-gl/cube.h>
#include <tango-gl/util.h>
#include <tango-gl/video_overlay.h>

#include "tango-plane-fitting/point_cloud_renderer.h"

namespace tango_plane_fitting {

/**
 * This class is the main application for PlaneFitting. It can be instantiated
 * in the JNI layer and use to pass information back and forth between Java. The
 * class also manages the application's lifecycle and interaction with the Tango
 * service. Primarily, this involves registering for callbacks and passing on
 * the necessary information to stored objects.
 */
class PlaneFittingApplication {
 public:
  PlaneFittingApplication();
  ~PlaneFittingApplication();

  // OnCreate() callback is called when this Android application's
  // OnCreate function is called from UI thread. In the OnCreate
  // function, we are only checking the Tango Core's version.
  //
  // @param env, java environment parameter OnCreate is being called.
  // @param caller_activity, caller of this function.
  // @param activity_orientation, orienation param for the activity.
  // @param sensor_orientation, orientation param for the color camera sensor.
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

  // Create OpenGL state and connect to the color camera texture.
  void OnSurfaceCreated();

  // Configure whether to display depth data for debugging.
  void SetRenderDebugPointCloud(bool on);

  // Configure the viewport of the GL view.
  void OnSurfaceChanged(int width, int height);

  // Get current camera position and render.
  void OnDrawFrame();

  //
  // Callback for point clouds that come in from the Tango service.
  //
  // @param point_cloud The point cloud returned by the service.
  //
  void OnPointCloudAvailable(const TangoPointCloud* point_cloud);

  //
  // Callback for touch events to fit a plane and place an object.  The Java
  // layer should ensure this is only called from the GL thread.
  //
  // @param x The requested x coordinate in screen space of the window.
  // @param y The requested y coordinate in screen space of the window.
  void OnTouchEvent(float x, float y);

  // Callback for display change event, we use this function to detect display
  // orientation change.
  //
  // @param display_rotation, the rotation index of the display. Same as the
  // Android display enum value, see here:
  // https://developer.android.com/reference/android/view/Display.html#getRotation()
  // Same as the Android sensor rotation enum value, see here:
  // https://developer.android.com/reference/android/hardware/Camera.CameraInfo.html#orientation
  void OnDisplayChanged(int display_rotation);

 private:
  // Details of rendering to OpenGL after determining transforms.
  void GLRender(const glm::mat4& w_T_cc);

  // Update the current point data.
  void UpdateCurrentPointData();

  // Setup the configuration file for the Tango Service. We'll also see whether
  // we'd like auto-recover enabled.
  void TangoSetupConfig();

  // Connect the OnXYZijAvailable and OnTextureAvailable callbacks.
  void TangoConnectCallbacks();

  // Connect to Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start Motion Tracking.
  void TangoConnect();

  // Disconnect from the Project Tango service.
  void TangoDisconnect();

  // Delete the GL resources.
  void DeleteResources();

  // Return transform for depth camera in Tango coordinate convention with
  // respect to
  // Area Description in OpenGL coordinate convention. The reason to switch from
  // one convention to
  // the other is an optimization that allow us to avoid transforming the depth
  // points into OpenGL
  // coordinate frame.
  glm::mat4 GetAreaDescriptionTDepthTransform(double timestamp);

  // Set view port and projection matrix. This must be called in the GL thread.
  void SetViewportAndProjectionGLThread();

  TangoConfig tango_config_;
  TangoCameraIntrinsics color_camera_intrinsics_;

  // Render objects
  tango_gl::VideoOverlay* video_overlay_;
  PointCloudRenderer* point_cloud_renderer_;
  tango_gl::Cube* cube_;

  // The dimensions of the render window.
  float screen_width_;
  float screen_height_;

  bool point_cloud_debug_render_;

  double last_gpu_timestamp_;

  // Cached transforms
  // OpenGL projection matrix.
  glm::mat4 projection_matrix_ar_;

  std::atomic<bool> is_service_connected_;
  std::atomic<bool> is_gl_initialized_;
  std::atomic<bool> is_scene_camera_configured_;

  // Point data manager.
  TangoSupportPointCloudManager* point_cloud_manager_;

  // Both of these orientation is used for handling display rotation in portrait
  // or landscape.
  TangoSupportRotation display_rotation_;
};

}  // namespace tango_plane_fitting

#endif  // TANGO_PLANE_FITTING_PLANE_FITTING_APPLICATION_H_
