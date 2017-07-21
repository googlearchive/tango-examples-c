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

#ifndef CPP_VIDEO_STABILIZATION_EXPERIMENT_TANGO_VIDEO_STABILIZATION_SCENE_H_
#define CPP_VIDEO_STABILIZATION_EXPERIMENT_TANGO_VIDEO_STABILIZATION_SCENE_H_

#include <jni.h>
#include <memory>
#include <math.h>
#include <mutex>
#include <vector>

#include <android/asset_manager.h>

#include <tango_support.h>
#include <tango_client_api.h>  // NOLINT
#include <tango-gl/camera.h>
#include <tango-gl/color.h>
#include <tango-gl/mesh.h>
#include <tango-gl/meshes.h>
#include <tango-gl/shaders.h>
#include <tango-gl/tango-gl.h>
#include <tango-gl/texture.h>
#include <tango-gl/transform.h>
#include <tango-gl/util.h>
#include <tango-gl/video_overlay.h>

namespace tango_video_stabilization {

// Scene provides OpenGL drawable objects and renders them for visualization.
class Scene {
 public:
  // Constructor and destructor.
  Scene();
  ~Scene();

  // Allocate OpenGL resources for rendering.
  void InitGLContent();

  // Release non-OpenGL resources.
  void DeleteResources();

  // Setup GL view port.
  // @param: w, width of the screen.
  // @param: h, height of the screen.
  void SetupViewport(int w, int h);

  // Clear the render on screen.
  void Clear();

  // Render loop.
  void Render();

  // Get video overlay texture id.
  // @return: texture id of video overlay's texture.
  GLuint GetVideoOverlayTextureId();

  // @return: AR render camera's image plane ratio.
  float GetCameraImagePlaneRatio() { return camera_image_plane_ratio_; }

  // Set AR render camera's image plane ratio.
  // @param: image plane ratio.
  void SetCameraImagePlaneRatio(float ratio) {
    camera_image_plane_ratio_ = ratio;
  }

  // @return: AR render camera's image plane distance from the view point.
  float GetImagePlaneDistance() { return image_plane_distance_; }

  // Set AR render camera's image plane distance from the view point.
  // @param: distance, AR render camera's image plane distance from the view
  //         point.
  void SetImagePlaneDistance(float distance) {
    image_plane_distance_ = distance;
  }

  // Set projection matrix of the AR view (first person view)
  // @param: projection_matrix, the projection matrix.
  void SetProjectionMatrix(const glm::mat4& projection_matrix);

  // Adds a new pose to our buffer.
  void AddNewPose(const TangoPoseData& pose);

  // Sets the control for video stabilization.
  void SetEnableVideoStabilization(bool enable_video_stabilization);

  // Locks or unlocks the camera.
  void SetCameraLocked(bool camera_locked);

  // Sets the display rotation.
  void SetDisplayRotation(TangoSupport_Rotation display_rotation);

 private:
  // Returns the smooth predicted virtual camera rotation.
  glm::quat GetSmoothedCameraRotation();

  // The number of frames to delay the video for improved video stabilization.
  const int kNumLookaheadFrames = 15;

  // The factor to zoom the video to crop the stabilized video.
  const float kVideoOverlayZoomFactor = 1.0f;

  // Camera object that allows user to use touch input to interact with.
  tango_gl::Camera camera_;

  // We use both camera_image_plane_ratio_ and image_plane_distance_ to compute
  // the first person AR camera's frustum, these value is derived from actual
  // physical camera instrinsics.
  // Aspect ratio of the color camera.
  float camera_image_plane_ratio_;

  // Image plane distance from camera's origin view point.
  float image_plane_distance_;

  // The projection matrix for the first person AR camera.
  glm::mat4 ar_camera_projection_matrix_;

  // Video rotational stabilization.
  bool enable_video_stabilization_ = false;
  bool is_camera_locked_ = false;
  std::mutex video_stabilization_mutex_;
  std::mutex camera_locked_mutex_;

  // Buffer of previous poses and index pointing to the oldest pose.
  std::vector<TangoPoseData> pose_buffer_;
  int oldest_pose_buffer_index_ = 0;

  // Buffer of previous video overlay frames and index pointing to the oldest
  // frame.
  std::vector<tango_gl::VideoOverlay*> video_overlay_buffer_;
  int oldest_video_overlay_buffer_index_ = 0;

  // Check if resources is allocated.
  bool is_content_initialized_ = false;

  int viewport_width_;
  int viewport_height_;
};
}  // namespace tango_video_stabilization

#endif  // CPP_VIDEO_STABILIZATION_EXPERIMENT_TANGO_VIDEO_STABILIZATION_SCENE_H_
