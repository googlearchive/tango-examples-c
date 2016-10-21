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

#ifndef RGB_DEPTH_SYNC_CAMERA_TEXTURE_DRAWABLE_H_
#define RGB_DEPTH_SYNC_CAMERA_TEXTURE_DRAWABLE_H_

#include <tango_support_api.h>
#include <tango-gl/util.h>
#include "rgb-depth-sync/shader.h"

namespace rgb_depth_sync {
// The drawable class to render color camera texture in the render loop.
// Please note that the color camera texture is in GL_TEXTURE_EXTERNAL_OES
// format.
class CameraTextureDrawable {
 public:
  CameraTextureDrawable();
  ~CameraTextureDrawable();
  // Render the color texture on screen.
  void RenderImage(TangoSupportDisplayRotation camera_to_display_rotation);

  // Call this function when the GL context has been reinitialized
  void InitializeGL();

  // Returns current color texture id.
  // @return: color texture id
  GLuint GetColorTextureId() const { return color_texture_id_; }

  // Set the color texture id that is used for rendering.
  // @param texture_id: texture id which we set the color_texture_id_
  void SetColorTextureId(GLuint texture_id) { color_texture_id_ = texture_id; }

  // Set the depth texture id that is used for rendering.
  // @param texture_id: texture id which we set the depth_texture_id_
  void SetDepthTextureId(GLuint texture_id) { depth_texture_id_ = texture_id; }

  // Alpha blend value for depth texture and color camera texture.
  // The value range is [0.0f, 1.0f].
  // @param blend_alpha: Blending value between rgb and depth texture.
  void SetBlendAlpha(float blend_alpha) { blend_alpha_ = blend_alpha; }

 private:
  float blend_alpha_;

  GLuint color_texture_id_;
  GLuint depth_texture_id_;

  GLuint color_texture_handle_;
  GLuint depth_texture_handle_;
  GLuint blend_alpha_handle_;

  GLuint attrib_texture_coords_;
  GLuint attrib_vertices_;

  GLuint shader_program_;

  // Vertex and indices buffers.
  GLuint render_buffers_[2];
};
}  // namespace rgb_depth_sync

#endif  // RGB_DEPTH_SYNC_CAMERA_TEXTURE_DRAWABLE_H_
