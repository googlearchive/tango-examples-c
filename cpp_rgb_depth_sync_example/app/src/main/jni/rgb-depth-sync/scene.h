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

#include <tango_support_api.h>
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
  Scene();
  ~Scene();

  // Setup GL view port.
  void SetupViewPort(int w, int h);

  // Clear the render on screen.
  void Clear();

  // Renders the scene onto the camera image using the provided depth texture.
  void Render(GLuint color_texture_id, GLuint depth_texture_id,
              TangoSupportDisplayRotation camera_to_display_rotation);

  // Recreate GL structures because of context creation.
  void InitializeGL();

  // Set the depth texture's alpha blending value. The range is [0.0, 1.0].
  void SetDepthAlphaValue(float alpha);

 private:
  GLsizei viewport_width_;
  GLsizei viewport_height_;

  CameraTextureDrawable camera_texture_drawable_;
};
}  // namespace rgb_depth_sync

#endif  // RGB_DEPTH_SYNC_SCENE_H_
