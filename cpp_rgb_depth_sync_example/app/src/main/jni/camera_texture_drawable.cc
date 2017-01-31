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

#include <rgb-depth-sync/camera_texture_drawable.h>

namespace {
const GLfloat kVertices[] = {-1.0, 1.0, 0.0, -1.0, -1.0, 0.0,
                             1.0,  1.0, 0.0, 1.0,  -1.0, 0.0};

const GLushort kIndices[] = {0, 1, 2, 2, 1, 3};

const GLfloat kTextureCoords0[] = {0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0};
const GLfloat kTextureCoords90[] = {1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0};
const GLfloat kTextureCoords180[] = {1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
const GLfloat kTextureCoords270[] = {0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0};

}  // namespace

namespace rgb_depth_sync {

CameraTextureDrawable::CameraTextureDrawable() : shader_program_(0) {}

CameraTextureDrawable::~CameraTextureDrawable() {}

void CameraTextureDrawable::InitializeGL() {
  shader_program_ =
      tango_gl::util::CreateProgram(rgb_depth_sync::shader::kColorCameraVert,
                                    rgb_depth_sync::shader::kColorCameraFrag);
  if (!shader_program_) {
    LOGE("Could not create shader program for CameraImageDrawable.");
  }

  glGenBuffers(2, render_buffers_);
  // Allocate vertices buffer.
  glBindBuffer(GL_ARRAY_BUFFER, render_buffers_[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, kVertices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Allocate triangle indices buffer.
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_buffers_[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6, kIndices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Assign the vertices attribute data.
  attrib_vertices_ = glGetAttribLocation(shader_program_, "vertex");
  glBindBuffer(GL_ARRAY_BUFFER, render_buffers_[0]);
  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Assign the texture coordinates attribute data.
  attrib_texture_coords_ =
      glGetAttribLocation(shader_program_, "textureCoords");

  color_texture_handle_ = glGetUniformLocation(shader_program_, "colorTexture");
  depth_texture_handle_ = glGetUniformLocation(shader_program_, "depthTexture");
  blend_alpha_handle_ = glGetUniformLocation(shader_program_, "blendAlpha");
}

void CameraTextureDrawable::RenderImage(
    TangoSupportRotation camera_to_display_rotation) {
  if (shader_program_ == 0) {
    InitializeGL();
  }

  glDisable(GL_DEPTH_TEST);

  glUseProgram(shader_program_);

  glUniform1f(blend_alpha_handle_, blend_alpha_);

  // Note that the Tango C-API update texture will bind the texture directly to
  // active texture, this is currently a bug in API, and because of that, we are
  // not getting any handle from shader neither binding any texture here.
  // Once this is fix, we will need to bind the texture to the correct sampler2D
  // handle.
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, color_texture_id_);
  glUniform1i(color_texture_handle_, 0);

  // Bind depth texture to texture unit 1.
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, depth_texture_id_);
  glUniform1i(depth_texture_handle_, 1);

  // Bind vertices buffer.
  glBindBuffer(GL_ARRAY_BUFFER, render_buffers_[0]);
  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glEnableVertexAttribArray(attrib_texture_coords_);
  switch (camera_to_display_rotation) {
    case TangoSupportRotation::ROTATION_90:
      glVertexAttribPointer(attrib_texture_coords_, 2, GL_FLOAT, GL_FALSE, 0,
                            kTextureCoords90);
      break;
    case TangoSupportRotation::ROTATION_180:
      glVertexAttribPointer(attrib_texture_coords_, 2, GL_FLOAT, GL_FALSE, 0,
                            kTextureCoords180);
      break;
    case TangoSupportRotation::ROTATION_270:
      glVertexAttribPointer(attrib_texture_coords_, 2, GL_FLOAT, GL_FALSE, 0,
                            kTextureCoords270);
      break;
    default:
      glVertexAttribPointer(attrib_texture_coords_, 2, GL_FLOAT, GL_FALSE, 0,
                            kTextureCoords0);
      break;
  }

  // Bind element array buffer.
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_buffers_[1]);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
  tango_gl::util::CheckGlError("ColorCameraDrawable glDrawElements");
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glUseProgram(0);
  glActiveTexture(GL_TEXTURE0);
  tango_gl::util::CheckGlError("CameraTextureDrawable::render");
}

}  // namespace rgb_depth_sync
