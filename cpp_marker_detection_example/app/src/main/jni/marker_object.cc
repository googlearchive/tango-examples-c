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

#include <tango-gl/shaders.h>
#include <tango-gl/conversions.h>
#include "tango-marker-detection/marker_object.h"

namespace {
// Vertices that form three line segments for X, Y and Z axes.
static const float kAxisVertices[] = {
  0.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 1.0f};

// Colors(RGBA) for each of the vertices.
static const float kAxisColors[] = {
  1.0f, 0.0f, 0.0f, 1.0f,
  1.0f, 0.0f, 0.0f, 1.0f,
  0.0f, 1.0f, 0.0f, 1.0f,
  0.0f, 1.0f, 0.0f, 1.0f,
  0.0f, 0.0f, 1.0f, 1.0f,
  0.0f, 0.0f, 1.0f, 1.0f};

// Bounding box color.
static const glm::vec4 kBoxColor = glm::vec4(0.f, 1.f, 1.f, 1.f);
}  // namespace

namespace tango_marker_detection {

MarkerObject::MarkerObject(float marker_size) : visible_(false) {
  // Create shader program.
  box_shader_program_ = tango_gl::util::CreateProgram(
      tango_gl::shaders::GetBasicVertexShader().c_str(),
      tango_gl::shaders::GetBasicFragmentShader().c_str());
  if (!box_shader_program_) {
    LOGE("Could not create program.");
  }

  // Get shader program variables.
  box_uniform_mvp_ = glGetUniformLocation(box_shader_program_, "mvp");
  box_uniform_color_ = glGetUniformLocation(box_shader_program_, "color");
  box_attrib_vertices_ = glGetAttribLocation(box_shader_program_, "vertex");

  // Create shader program for three axes.
  axis_shader_program_ = tango_gl::util::CreateProgram(
      tango_gl::shaders::GetColorVertexShader().c_str(),
      tango_gl::shaders::GetBasicFragmentShader().c_str());
  if (!axis_shader_program_) {
    LOGE("Could not create program.");
  }

  // Get shader program variables.
  axis_uniform_mvp_ = glGetUniformLocation(axis_shader_program_, "mvp");
  axis_attrib_colors_ = glGetAttribLocation(axis_shader_program_, "color");
  axis_attrib_vertices_ = glGetAttribLocation(axis_shader_program_, "vertex");

  // Make the length of each axis 1/3 of the size of the marker.
  float axis_length = marker_size / 3;

  // Populate vertex array and color array.
  size_t size = sizeof(kAxisVertices) / (sizeof(float) * 3);
  for (size_t i = 0; i < size; i++) {
    // Scale the vertex coordinates with the length of axis.
    axis_vertices_.push_back(glm::vec3(
      kAxisVertices[i * 3] * axis_length,
      kAxisVertices[i * 3 + 1] * axis_length,
      kAxisVertices[i * 3 + 2] * axis_length));

    axis_colors_.push_back(glm::vec4(
      kAxisColors[i * 4],
      kAxisColors[i * 4 + 1],
      kAxisColors[i * 4 + 2],
      kAxisColors[i * 4 + 3]));
  }
}

MarkerObject::~MarkerObject() {}

void MarkerObject::Render(tango_gl::Camera* camera) const {
  if (!visible_) {
    return;
  }

  // Render bounding box
  DrawBoundingBox(camera);

  // Render three axes
  DrawAxes(camera);
}

void MarkerObject::DrawBoundingBox(tango_gl::Camera* camera) const {
  glUseProgram(box_shader_program_);
  glLineWidth(3.f);

  // There is no need to apply a model matrix since corners are already in
  // world frame.
  glm::mat4 mvp_mat = camera->GetProjectionMatrix() * camera->GetViewMatrix();

  glUniformMatrix4fv(box_uniform_mvp_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  glUniform4f(box_uniform_color_,
    kBoxColor.r, kBoxColor.g, kBoxColor.b, kBoxColor.a);

  glEnableVertexAttribArray(box_attrib_vertices_);
  glVertexAttribPointer(box_attrib_vertices_, 3, GL_FLOAT, GL_FALSE,
                        sizeof(glm::vec3), &box_vertices_[0]);

  glDrawArrays(GL_LINE_LOOP, 0, box_vertices_.size());

  glDisableVertexAttribArray(box_attrib_vertices_);
  glUseProgram(0);
}

void MarkerObject::DrawAxes(tango_gl::Camera* camera) const {
  glUseProgram(axis_shader_program_);
  glLineWidth(10.f);

  // Compute model-view-projection matrix
  glm::mat4 mvp_mat =
      camera->GetProjectionMatrix() * camera->GetViewMatrix() * world_T_local_;

  glUniformMatrix4fv(axis_uniform_mvp_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  glEnableVertexAttribArray(axis_attrib_vertices_);
  glVertexAttribPointer(axis_attrib_vertices_, 3, GL_FLOAT, GL_FALSE,
                        sizeof(glm::vec3), &axis_vertices_[0]);

  glEnableVertexAttribArray(axis_attrib_colors_);
  glVertexAttribPointer(axis_attrib_colors_, 4, GL_FLOAT, GL_FALSE,
                        sizeof(glm::vec4), &axis_colors_[0]);

  glDrawArrays(GL_LINES, 0, axis_vertices_.size());

  glDisableVertexAttribArray(axis_attrib_vertices_);
  glDisableVertexAttribArray(axis_attrib_colors_);
  glUseProgram(0);
}

void MarkerObject::Update(const TangoMarkers_Marker& marker) {
  // Update axes model transform with marker pose.
  world_T_local_ = tango_gl::conversions::TransformFromArrays(
      marker.translation, marker.orientation);

  // Update vertices of bounding box with marker corners.
  box_vertices_.resize(4);
  for (int i = 0; i < 4; ++i) {
    box_vertices_[i] =
        glm::vec3(
          marker.corners_3d[i][0],
          marker.corners_3d[i][1],
          marker.corners_3d[i][2]);
  }

  // Make the marker visible.
  visible_ = true;
}
}  // namespace tango_marker_detection
