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

#ifndef RGB_DEPTH_SYNC_SCENE_H_
#define RGB_DEPTH_SYNC_SCENE_H_

#include <tango-gl/camera.h>
#include <tango-gl/grid.h>
#include <tango-gl/util.h>

#include "rgb-depth-sync/camera_texture_drawable.h"
#include "rgb-depth-sync/color_image.h"
#include "rgb-depth-sync/depth_image.h"

namespace rgb_depth_sync {
// The Scene class is responsible for managing rendering for the application.
// Internally, it holds a number of drawable objects that it uses in its Render
// function to draw.
class Scene {
 public:
  Scene(ColorImage* color_image, DepthImage* depth_image);
  ~Scene();

  // Setup GL view port.
  void SetupViewPort(int w, int h);

  // Renders the scene onto the camera image using the provided depth texture.
  void Render();

  // Set the depth texture's alpha blending value. The range is [0.0, 1.0].
  void SetDepthAlphaValue(float alpha);

  // Set the camera intrinsics to use for this scene.
  void SetCameraIntrinsics(const TangoCameraIntrinsics& cc_intrinsics);

 private:
  GLuint screen_width_;
  GLuint screen_height_;

  // The Tango world with respect to the OpenGL world matrix.
  glm::mat4 OW_T_W_;

  // The OpenGL camera with respect to the color camera matrix.
  glm::mat4 CC_T_OC_;

  // The projection matrix of the color camera.
  glm::mat4 projection_matrix_ar_;

  tango_gl::Grid grid_;
  CameraTextureDrawable camera_texture_drawable_;

  // These are non-owning pointers and should not be deleted.
  ColorImage* color_image_;
  DepthImage* depth_image_;
};
}  // namespace rgb_depth_sync

#endif  // RGB_DEPTH_SYNC_SCENE_H_
