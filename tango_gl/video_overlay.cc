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
int CombineSensorRotation(int activity_orientation, int sensor_orientation) {
  int sensor_orientation_n = 0;
  switch (sensor_orientation) {
    case 90:
      sensor_orientation_n = 1;
      break;
    case 180:
      sensor_orientation_n = 2;
      break;
    case 270:
      sensor_orientation_n = 3;
      break;
    default:
      sensor_orientation_n = 0;
      break;
  }

  int ret = activity_orientation - sensor_orientation_n;
  if (ret < 0) {
    ret += 4;
  }
  return (ret % 4);
}

const GLfloat kVertices[] = {-1.0, 1.0, 0.0, -1.0, -1.0, 0.0,
                             1.0,  1.0, 0.0, 1.0,  -1.0, 0.0};

const GLushort kIndices[] = {0, 1, 2, 2, 1, 3};

const GLfloat kTextureCoords0[] = {0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0};
const GLfloat kTextureCoords90[] = {1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0};
const GLfloat kTextureCoords180[] = {1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
const GLfloat kTextureCoords270[] = {0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0};
}  // annonymous namespace

namespace tango_gl {

VideoOverlay::VideoOverlay(GLuint texture_type) : texture_type_(texture_type) {
  Initialize(0, 0);
}

VideoOverlay::VideoOverlay(GLuint texture_type, int activity_orientation,
                           int sensor_orientation)
    : texture_type_(texture_type) {
  Initialize(activity_orientation, sensor_orientation);
}

VideoOverlay::VideoOverlay(int activity_orientation, int sensor_orientation)
    : texture_type_(GL_TEXTURE_EXTERNAL_OES) {
  Initialize(activity_orientation, sensor_orientation);
}

VideoOverlay::VideoOverlay() : texture_type_(GL_TEXTURE_EXTERNAL_OES) {
  Initialize(0, 0);
}

void VideoOverlay::SetOrientationFromAndroid(int activity_orientation,
                                             int sensor_orientation) {
  combined_sensor_orientation_ =
      CombineSensorRotation(activity_orientation, sensor_orientation);
}

void VideoOverlay::Initialize(int activity_orientation,
                              int sensor_orientation) {
  SetOrientationFromAndroid(activity_orientation, sensor_orientation);

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
  switch (combined_sensor_orientation_) {
    case 1:
      glVertexAttribPointer(attrib_texture_coords_, 2, GL_FLOAT, GL_FALSE, 0,
                            kTextureCoords90);
      break;
    case 2:
      glVertexAttribPointer(attrib_texture_coords_, 2, GL_FLOAT, GL_FALSE, 0,
                            kTextureCoords180);
      break;
    case 3:
      glVertexAttribPointer(attrib_texture_coords_, 2, GL_FLOAT, GL_FALSE, 0,
                            kTextureCoords270);
      break;
    default:
      glVertexAttribPointer(attrib_texture_coords_, 2, GL_FLOAT, GL_FALSE, 0,
                            kTextureCoords0);
      break;
  }

  // Bind element array buffer.
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffers_[1]);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
  util::CheckGlError("glDrawElements");
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glUseProgram(0);
  util::CheckGlError("glUseProgram()");
}

}  // namespace tango_gl
