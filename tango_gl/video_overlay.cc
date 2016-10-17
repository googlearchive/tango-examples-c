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
#include <cstdlib>

#include "tango-gl/video_overlay.h"
#include "tango-gl/shaders.h"

namespace {
const GLfloat kVertices[] = {-1.0, 1.0, 0.0, -1.0, -1.0, 0.0,
                             1.0,  1.0, 0.0, 1.0,  -1.0, 0.0};

const GLushort kIndices[] = {0, 1, 2, 2, 1, 3};
}  // annonymous namespace

namespace tango_gl {

VideoOverlay::VideoOverlay()
    : texture_type_(GL_TEXTURE_EXTERNAL_OES),
      camera_to_display_rotation_(TangoSupportDisplayRotation::ROTATION_0),
      u_offset_(0.0f),
      v_offset_(0.0f) {
  Initialize();
}

VideoOverlay::VideoOverlay(GLuint texture_type)
    : texture_type_(texture_type),
      camera_to_display_rotation_(TangoSupportDisplayRotation::ROTATION_0),
      u_offset_(0.0f),
      v_offset_(0.0f) {
  Initialize();
}

VideoOverlay::VideoOverlay(
    TangoSupportDisplayRotation camera_to_display_rotation)
    : texture_type_(GL_TEXTURE_EXTERNAL_OES),
      camera_to_display_rotation_(camera_to_display_rotation),
      u_offset_(0.0f),
      v_offset_(0.0f) {
  Initialize();
}

VideoOverlay::VideoOverlay(
    GLuint texture_type, TangoSupportDisplayRotation camera_to_display_rotation)
    : texture_type_(texture_type),
      camera_to_display_rotation_(camera_to_display_rotation),
      u_offset_(0.0f),
      v_offset_(0.0f) {
  Initialize();
}

void VideoOverlay::Initialize() {
  SetColorToDisplayRotation(camera_to_display_rotation_);

  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  if (texture_type_ == GL_TEXTURE_EXTERNAL_OES) {
    shader_program_ =
        util::CreateProgram(shaders::GetVideoOverlayVertexShader().c_str(),
                            shaders::GetVideoOverlayFragmentShader().c_str());
  } else {
    shader_program_ = util::CreateProgram(
        shaders::GetVideoOverlayVertexShader().c_str(),
        shaders::GetVideoOverlayTexture2DFragmentShader().c_str());
  }

  if (!shader_program_) {
    LOGE("Could not create program.");
  }

  glGenTextures(1, &texture_id_);
  glBindTexture(texture_type_, texture_id_);
  glTexParameteri(texture_type_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(texture_type_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  uniform_texture_ = glGetUniformLocation(shader_program_, "texture");

  glGenBuffers(2, vertex_buffers_);
  // Allocate vertices buffer.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers_[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, kVertices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Allocate triangle indices buffer.
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffers_[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6, kIndices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Assign the vertices attribute data.
  attrib_vertices_ = glGetAttribLocation(shader_program_, "vertex");
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers_[0]);
  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Assign the texture coordinates attribute data.
  attrib_texture_coords_ =
      glGetAttribLocation(shader_program_, "textureCoords");

  uniform_mvp_mat_ = glGetUniformLocation(shader_program_, "mvp");
}

void VideoOverlay::SetColorToDisplayRotation(
    TangoSupportDisplayRotation rotation) {
  camera_to_display_rotation_ = rotation;
  switch (camera_to_display_rotation_) {
    case TangoSupportDisplayRotation::ROTATION_90:
      texture_coords_ = {1.0f - u_offset_, 0.0f + v_offset_,
                         0.0f + u_offset_, 0.0f + v_offset_,
                         1.0f - u_offset_, 1.0f - v_offset_,
                         0.0f + u_offset_, 1.0f - v_offset_};
      break;
    case TangoSupportDisplayRotation::ROTATION_180:
      texture_coords_ = {1.0f - u_offset_, 1.0f - v_offset_,
                         1.0f - u_offset_, 0.0f + v_offset_,
                         0.0f + u_offset_, 1.0f - v_offset_,
                         0.0f + u_offset_, 0.0f + v_offset_};
      break;
    case TangoSupportDisplayRotation::ROTATION_270:
      texture_coords_ = {0.0f + u_offset_, 1.0f - v_offset_,
                         1.0f - u_offset_, 1.0f - v_offset_,
                         0.0f + u_offset_, 0.0f + v_offset_,
                         1.0f - u_offset_, 0.0f + v_offset_};
      break;
    default:
      texture_coords_ = {0.0f + u_offset_, 0.0f + v_offset_,
                         0.0f + u_offset_, 1.0f - v_offset_,
                         1.0f - u_offset_, 0.0f + v_offset_,
                         1.0f - u_offset_, 1.0f - v_offset_};
      break;
  }
}

void VideoOverlay::SetTextureOffset(float screen_width, float screen_height,
                                    float image_width, float image_height) {
  if ((screen_width / screen_height > 1.0f) !=
      (image_width / image_height > 1.0f)) {
    // if image ratio and screen ratio don't comply to each other, we always
    // aligned things to screen ratio.
    float tmp = image_width;
    image_width = image_height;
    image_height = tmp;
  }

  float screen_ratio = screen_width / screen_height;
  float image_ratio = image_width / image_height;
  float zoom_factor = 1.0f;

  if (image_ratio > screen_ratio) {
    zoom_factor = screen_height / image_height;
  } else {
    zoom_factor = screen_width / image_width;
  }

  float render_width = image_width * zoom_factor;
  float render_height = image_height * zoom_factor;

  u_offset_ = ((render_width - screen_width) / 2.0f) / render_width;
  v_offset_ = ((render_height - screen_height) / 2.0f) / render_height;

  SetColorToDisplayRotation(camera_to_display_rotation_);
}

void VideoOverlay::Render(const glm::mat4& projection_mat,
                          const glm::mat4& view_mat) const {
  glUseProgram(shader_program_);

  glUniform1i(uniform_texture_, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(texture_type_, texture_id_);

  glm::mat4 model_mat = GetTransformationMatrix();
  glm::mat4 mvp_mat = projection_mat * view_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  // Bind vertices buffer.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers_[0]);
  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glEnableVertexAttribArray(attrib_texture_coords_);
  glVertexAttribPointer(attrib_texture_coords_, 2, GL_FLOAT, GL_FALSE, 0,
                        texture_coords_.data());

  // Bind element array buffer.
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffers_[1]);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
  util::CheckGlError("glDrawElements");
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glUseProgram(0);
  util::CheckGlError("glUseProgram()");
}

}  // namespace tango_gl
