/*
 * Copyright 2017 Google Inc. All Rights Reserved.
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

#ifndef CPP_MARKER_DETECTION_EXAMPLE_TANGO_MARKER_DETECTION_SCENE_H_
#define CPP_MARKER_DETECTION_EXAMPLE_TANGO_MARKER_DETECTION_SCENE_H_

#include <jni.h>
#include <atomic>
#include <memory>
#include <vector>
#include <math.h>
#include <mutex>

#include <android/asset_manager.h>

#include <tango_client_api.h>
#include <tango-gl/camera.h>
#include <tango-gl/video_overlay.h>

#include <tango-marker-detection/marker_object.h>

namespace tango_marker_detection {

// Scene provides OpenGL drawable objects and renders them for visualization.
class Scene {
 public:
  // Constructor and destructor.
  Scene();
  ~Scene();

  // Allocate OpenGL resources for rendering.
  void InitGLContent(AAssetManager* aasset_manager);

  // Release non-OpenGL resources.
  void DeleteResources();

  // Setup GL view port.
  // @param: w, width of the screen.
  // @param: h, height of the screen.
  void SetupViewport(int w, int h);

  // Setup the projection matrix and the transformation matrix of the virtual
  // camera of the AR view (first person view)
  // @param: projection_matrix, the projection matrix.
  // @param: transformation_matrix, the transformation matrix.
  void SetupCamera(const glm::mat4& projection_matrix,
                   const glm::mat4& transformation_matrix);

  // Clear the screen to a solid color.
  void Clear();

  // Render loop.
  void Render();

  // Get video overlay texture id.
  // @return: texture id of video overlay's texture.
  GLuint GetVideoOverlayTextureId() { return video_overlay_->GetTextureId(); }

  // Set video overlay's orientation based on current device orientation.
  void SetVideoOverlayRotation(int display_rotation);

  // Detect markers within the input image buffer.
  // @param image_buffer pointer to the input image buffer.
  // @param world_T_camera the transformation from camera to world frame.
  void DetectMarkers(const TangoImageBuffer& image_buffer,
                     const glm::mat4& world_T_camera);

 private:
  // Video overlay drawable object to display the camera image.
  std::unique_ptr<tango_gl::VideoOverlay> video_overlay_;

  // Camera object that allows user to use touch input to interact with.
  std::unique_ptr<tango_gl::Camera> camera_;

  // Marker objects.
  std::vector<std::unique_ptr<MarkerObject>> objects_;

  // Mutex for protecting mesh objects. Mesh objects are shared between the
  // rendering thread and TangoService callback thread.
  std::mutex objects_mutex_;

  // Check if resources is allocated.
  std::atomic<bool> is_content_initialized_;

  // Width and height of the viewport.
  int viewport_width_;
  int viewport_height_;
};
}  // namespace tango_marker_detection

#endif  // CPP_MARKER_DETECTION_EXAMPLE_TANGO_MARKER_DETECTION_SCENE_H_
