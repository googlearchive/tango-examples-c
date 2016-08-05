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

#ifndef TANGO_POINT_TO_POINT_POINT_TO_POINT_APPLICATION_H_
#define TANGO_POINT_TO_POINT_POINT_TO_POINT_APPLICATION_H_

#include <jni.h>
#include <memory>
#include <mutex>
#include <string>

#include <tango_client_api.h>
#include <tango_support_api.h>
#include <tango-gl/line.h>
#include <tango-gl/segment_drawable.h>
#include <tango-gl/util.h>
#include <tango-gl/video_overlay.h>

namespace tango_point_to_point {

/**
 * This class is the main application for PointToPoint. It can be instantiated
 * in the JNI layer and use to pass information back and forth between Java. The
 * class also manages the application's lifecycle and interaction with the Tango
 * service. Primarily, this involves registering for callbacks and passing on
 * the necessary information to stored objects.
 */
class PointToPointApplication {
 public:
  PointToPointApplication();
  ~PointToPointApplication();

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

  // Setup configuration options for Project Tango service, register
  // for callbacks, and connect to the Project Tango service.
  int TangoSetupAndConnect();

  // Disconnect from the Project Tango service.
  void TangoDisconnect();

  // Create OpenGL state and connect to the color camera texture.
  int InitializeGLContent();

  // Configure which method to use for upsampling.
  void SetUpsampleViaBilateralFiltering(bool on);

  // Configure the viewport of the GL view.
  void SetViewPort(int width, int height);

  // Get current camera position and render.
  void Render();

  // Delete the allocate resources.
  void DeleteResources();

  // Return the distance between the two selected points.
  std::string GetPointSeparation();

  //
  // Callback for point clouds that come in from the Tango service.
  //
  // @param xyz_ij The point cloud returned by the service.
  //
  void OnXYZijAvailable(const TangoXYZij* xyz_ij);

  //
  // Callback for image buffers that come in from the Tango service.
  //
  // @param buffer The image buffer returned by the service.
  //
  void OnFrameAvailable(const TangoImageBuffer* buffer);

  //
  // Callback for touch events to place apoint and update the line segment. The
  // Java
  // layer should ensure this is only called from the GL thread.
  //
  // @param x The requested x coordinate in screen space of the window.
  // @param y The requested y coordinate in screen space of the window.
  void OnTouchEvent(float x, float y);

 private:
  // Details of rendering to OpenGL after determining transforms.
  void GLRender(const glm::mat4& opengl_ss_T_color_opengl);

  // Get the x,y,z point of the touch location.
  bool GetDepthAtPoint(const float uv[2], float xyz[3],
                       const TangoPoseData& color_camera_T_point_cloud);

  // Update the segment based on a new touch position.
  void UpdateSegment(glm::vec4 world_position);

  // Return transform for depth camera in Tango coordinate convention with
  // respect to
  // Start of Service in OpenGL coordinate convention. The reason to switch from
  // one convention to
  // the other is an optimization that allow us to avoid transforming the depth
  // points into OpenGL
  // coordinate frame.
  glm::mat4 GetStartServiceTDepthPose();

  TangoConfig tango_config_;
  TangoCameraIntrinsics color_camera_intrinsics_;

  // Render objects
  tango_gl::VideoOverlay* video_overlay_;

  // The dimensions of the render window.
  GLsizei screen_width_;
  GLsizei screen_height_;

  double last_gpu_timestamp_;

  // Cached transforms
  // OpenGL projection matrix.
  glm::mat4 projection_matrix_ar_;

  // Point data manager.
  TangoSupportPointCloudManager* point_cloud_manager_;
  TangoXYZij* front_cloud_;

  // Image data manager.
  TangoSupportImageBufferManager* image_buffer_manager_;
  TangoImageBuffer* image_buffer_;

  // The depth interpolator class.
  TangoSupportDepthInterpolator* interpolator_;

  // To keep track of when segment can be rendered.
  int tap_number_;

  enum UpsampleAlgorithm {
    kNearest,
    kBilateral
  };
  UpsampleAlgorithm algorithm_;

  tango_gl::SegmentDrawable* segment_;

  // point data is shared between the UI thread we start for updating
  // texts and the TangoService event callback thread. We keep points_mutex_ to
  // protect point1_, point2_, and both flags.
  std::mutex tango_points_mutex_;

  // Toggles which point will be altered
  bool point_modifier_flag_;
  glm::vec3 point1_;
  glm::vec3 point2_;

  // Are both points defined?
  bool segment_is_drawable_;
};

}  // namespace tango_point_to_point

#endif  // TANGO_POINT_TO_POINT_POINT_TO_POINT_APPLICATION_H_
