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

#include "tango-video-overlay/yuv_drawable.h"

namespace {
const GLfloat kVertices[] = {-1.0, 1.0, 0.0, -1.0, -1.0, 0.0,
                             1.0,  1.0, 0.0, 1.0,  -1.0, 0.0};

const GLushort kIndices[] = {0, 1, 2, 2, 1, 3};

const GLfloat kTextureCoords[] = {0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0};

const std::string kVertexShader =
    "precision highp float;\n"
    "precision highp int;\n"
    "attribute vec4 vertex;\n"
    "attribute vec2 textureCoords;\n"
    "varying vec2 f_textureCoords;\n"
    "uniform mat4 mvp;\n"
    "void main() {\n"
    "  f_textureCoords = textureCoords;\n"
    "  gl_Position = mvp * vertex;\n"
    "}\n";

const std::string kFragmetnShader =
    "precision highp float;\n"
    "precision highp int;\n"
    "uniform sampler2D texture;\n"
    "varying vec2 f_textureCoords;\n"
    "void main() {\n"
    "  gl_FragColor = texture2D(texture, f_textureCoords);\n"
    "}\n";
}  // namespace

namespace tango_video_overlay {

YUVDrawable::YUVDrawable() {
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  shader_program_ = tango_gl::util::CreateProgram(kVertexShader.c_str(),
                                                  kFragmetnShader.c_str());
  if (!shader_program_) {
    LOGE("Could not create program.");
  }

  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  uniform_texture_ = glGetUniformLocation(shader_program_, "texture");

  glGenBuffers(3, vertex_buffers_);
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

  // Allocate texture coordinates buufer.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers_[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * 4, kTextureCoords,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Assign the vertices attribute data.
  attrib_vertices_ = glGetAttribLocation(shader_program_, "vertex");
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers_[0]);
  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Assign the texture coordinates attribute data.
  attrib_texture_coords_ =
      glGetAttribLocation(shader_program_, "textureCoords");
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers_[2]);
  glEnableVertexAttribArray(attrib_texture_coords_);
  glVertexAttribPointer(attrib_texture_coords_, 2, GL_FLOAT, GL_FALSE, 0,
                        nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  uniform_mvp_mat_ = glGetUniformLocation(shader_program_, "mvp");
}

void YUVDrawable::Render(const glm::mat4& projection_mat,
                         const glm::mat4& view_mat) const {
  glUseProgram(shader_program_);

  glUniform1i(uniform_texture_, 2);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, texture_id_);

  glm::mat4 model_mat = GetTransformationMatrix();
  glm::mat4 mvp_mat = projection_mat * view_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  // Bind vertices buffer.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers_[0]);
  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Bind texture coordinates buffer.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers_[2]);
  glEnableVertexAttribArray(attrib_texture_coords_);
  glVertexAttribPointer(attrib_texture_coords_, 2, GL_FLOAT, GL_FALSE, 0,
                        nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Bind element array buffer.
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffers_[1]);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
  tango_gl::util::CheckGlError("glDrawElements");
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glUseProgram(0);
  tango_gl::util::CheckGlError("glUseProgram()");
}

}  // namespace tango_video_overlay
