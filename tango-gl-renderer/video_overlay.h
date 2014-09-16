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

#ifndef VIDEO_OVERLAY_H
#define VIDEO_OVERLAY_H

#include "drawable_object.h"
#include "gl_util.h"

class VideoOverlay : public DrawableObject {
 public:
  VideoOverlay();
  void Render(glm::mat4 projection_mat, glm::mat4 view_mat);
  GLuint texture_id;
  void SetupIntrinsics(float k1, float k2, float k3, float cx, float cy, float w, float h);

 private:
  GLuint vertex_buffers_;

  GLuint attrib_vertices_;
  GLuint attrib_textureCoords;
  GLuint uniform_texture;
  GLuint vertex_buffers[3];
  GLuint shader_program_;

  GLuint uniform_mvp_mat_;
};
#endif  // VIDEO_OVERLAY_H
