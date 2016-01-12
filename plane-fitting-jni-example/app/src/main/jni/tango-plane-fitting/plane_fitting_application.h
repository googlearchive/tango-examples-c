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

  // Initialize the Project Tango service.
  int TangoInitialize(JNIEnv* env, jobject caller_activity);
  // Setup configuration options for Project Tango service, register
  // for callbacks, and connect to the Project Tango service.
  int TangoSetupAndConnect();

  // Disconnect from the Project Tango service.
  void TangoDisconnect();

  // Create OpenGL state and connect to the color camera texture.
  int InitializeGLContent();

  // Configure whether to display depth data for debugging.
  void SetRenderDebugPointCloud(bool on);

  // Configure the viewport of the GL view.
  void SetViewPort(int width, int height);

  // Get current camera position and render.
  void Render();

  // Delete the allocate resources.
  void DeleteResources();

  //
  // Callback for point clouds that come in from the Tango service.
  //
  // @param xyz_ij The point cloud returned by the service.
  //
  void OnXYZijAvailable(const TangoXYZij* xyz_ij);

  //
  // Callback for touch events to fit a plane and place an object.  The Java
  // layer should ensure this is only called from the GL thread.
  //
  // @param x The requested x coordinate in screen space of the window.
  // @param y The requested y coordinate in screen space of the window.
  void OnTouchEvent(float x, float y);

 private:
  // Details of rendering to OpenGL after determining transforms.
  void GLRender(const glm::mat4& w_T_cc);

  // Update the current point data.
  void UpdateCurrentPointData();

  // return pose for device position with respect to
  // start of service.
  glm::mat4 GetStartServiceTDeviceTransform();

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
  // Pose of color camera with respect to device.
  glm::mat4 device_T_color_;
  // Pose of depth camera with respect to device.
  glm::mat4 device_T_depth_;
  // Start of service with respect to OpenGL world.
  glm::mat4 opengl_world_T_start_service_;
  // OpenGL camera with respect to color camera.
  glm::mat4 color_camera_T_opengl_camera_;
  // OpenGL projection matrix.
  glm::mat4 projection_matrix_ar_;

  // Point data manager.
  TangoSupportPointCloudManager* point_cloud_manager_;
  TangoXYZij* front_cloud_;
};

}  // namespace tango_plane_fitting

#endif  // TANGO_PLANE_FITTING_PLANE_FITTING_APPLICATION_H_
