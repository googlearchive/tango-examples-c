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

#ifndef CPP_POINT_TO_POINT_EXAMPLE_TANGO_POINT_TO_POINT_POINT_TO_POINT_APPLICATION_H_
#define CPP_POINT_TO_POINT_EXAMPLE_TANGO_POINT_TO_POINT_POINT_TO_POINT_APPLICATION_H_

#include <atomic>
#include <jni.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <tango_client_api.h>
#include <tango_support.h>
#include <tango_depth_interpolation.h>
#include <tango-gl/line.h>
#include <tango-gl/segment_drawable.h>
#include <tango-gl/util.h>
#include <tango-gl/video_overlay.h>

namespace tango_point_to_point {

class MeasuredPoint {
 public:
  MeasuredPoint(const glm::vec3& point, double timestamp) {
    this->point_depth = point;
    this->timestamp = timestamp;
  }
  glm::vec3 point_depth;
  double timestamp;
};

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

  // Create OpenGL state and connect to the color camera texture.
  void OnSurfaceCreated();

  // Configure which method to use for upsampling.
  void SetUpsampleViaBilateralFiltering(bool on);

  // Configure the viewport of the GL view.
  void OnSurfaceChanged(int width, int height);

  // Get current camera position and render.
  void OnDrawFrame();

  // Return the distance between the two selected points.
  std::string GetPointSeparation();

  //
  // Callback for point clouds that come in from the Tango service.
  //
  // @param point_cloud The point cloud returned by the service.
  //
  void OnPointCloudAvailable(const TangoPointCloud* point_cloud);

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

  //
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
  // Update the segment based on a new touch position.
  void UpdateMeasuredPoints(glm::vec3 world_position, double timestamp);

  // Setup the configuration file for the Tango Service. We'll also see whether
  // we'd like auto-recover enabled.
  void TangoSetupConfig();

  // Connect the OnTextureAvailable and OnTangoEvent callbacks.
  void TangoConnectCallbacks();

  // Connect to Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start Motion Tracking.
  void TangoConnect();

  // Disconnect from the Project Tango service.
  void TangoDisconnect();

  // Delete the allocate resources.
  void DeleteResources();

  // Set view port and projection matrix. This must be called in the GL thread.
  void SetViewportAndProjectionGLThread();

  TangoConfig tango_config_;
  TangoCameraIntrinsics color_camera_intrinsics_;

  // Render objects
  tango_gl::VideoOverlay* video_overlay_;

  // The dimensions of the render window.
  float screen_width_;
  float screen_height_;

  double last_gpu_timestamp_;

  // Cached transforms
  // OpenGL projection matrix.
  glm::mat4 projection_matrix_ar_;

  // Point data manager.
  TangoSupport_PointCloudManager* point_cloud_manager_;

  // Image data manager.
  TangoSupport_ImageBufferManager* image_buffer_manager_;

  // The depth interpolator class.
  TangoDepthInterpolation_Interpolator* interpolator_;

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

  MeasuredPoint measured_point0_;
  MeasuredPoint measured_point1_;
  float measured_distance_;

  // Are both points defined?
  bool segment_is_drawable_;

  std::atomic<bool> is_service_connected_;
  std::atomic<bool> is_gl_initialized_;
  std::atomic<bool> is_scene_camera_configured_;

  // Orientation is used for handling display rotation in portrait
  // or landscape.
  TangoSupport_Rotation display_rotation_;
};

}  // namespace tango_point_to_point

#endif  // CPP_POINT_TO_POINT_EXAMPLE_TANGO_POINT_TO_POINT_POINT_TO_POINT_APPLICATION_H_
