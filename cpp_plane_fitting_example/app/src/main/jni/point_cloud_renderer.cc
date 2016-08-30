/*
 * Copyright 2015 Google Inc. All Rights Reserved.
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

#include "tango-plane-fitting/point_cloud_renderer.h"

#include <tango-gl/conversions.h>
#include <tango_support_api.h>

#include "tango-plane-fitting/plane_fitting.h"

namespace tango_plane_fitting {

namespace {

const std::string kPointCloudVertexShader =
    "precision mediump float;\n"
    "attribute vec4 vertex;\n"
    "uniform mat4 mvp;\n"
    "uniform vec4 plane;\n"
    "uniform float plane_distance;\n"
    "varying vec3 v_color;\n"
    ""
    "void main() {\n"
    "  gl_PointSize = 7.0;\n"
    "  gl_Position =  mvp*vertex;\n"
    "  "
    "  float d = dot(plane, vertex);\n"
    "  if (abs(d) < (plane_distance * plane_distance)) {\n"
    "    v_color = vec3(0.0, 1.0, 0.0);\n"
    "  } else if (d < 0.0) {\n"
    "    v_color = vec3(1.0, 0.0, 0.0);\n"
    "  } else {\n"
    "    v_color = vec3(0.0, 0.0, 1.0);\n"
    "  }\n"
    "}\n";
const std::string kPointCloudFragmentShader =
    "precision mediump float;\n"
    "varying vec3 v_color;\n"
    "void main() {\n"
    "  gl_FragColor = vec4(v_color, 1.0);\n"
    "}\n";

}  // namespace

PointCloudRenderer::PointCloudRenderer()
    : plane_distance_(0.05f),
      debug_colors_(false),
      plane_model_(glm::vec4(0.0, 0.0, 1.0, 0.0)) {

  shader_program_ = tango_gl::util::CreateProgram(
      kPointCloudVertexShader.c_str(), kPointCloudFragmentShader.c_str());

  glGenBuffers(1, &vertex_buffer_);

  mvp_handle_ = glGetUniformLocation(shader_program_, "mvp");
  vertices_handle_ = glGetAttribLocation(shader_program_, "vertex");
  plane_handle_ = glGetUniformLocation(shader_program_, "plane");
  plane_distance_handle_ =
      glGetUniformLocation(shader_program_, "plane_distance");

  tango_gl::util::CheckGlError("PointCloudRenderer::Construction");
}

PointCloudRenderer::~PointCloudRenderer() {}

void PointCloudRenderer::DeleteGLResources() {
  glDeleteProgram(shader_program_);
  glDeleteBuffers(0, &vertex_buffer_);
}

void PointCloudRenderer::Render(const glm::mat4& projection_T_depth,
                                const glm::mat4& opengl_T_depth,
                                const TangoPointCloud* point_cloud) {
  if (!debug_colors_) {
    return;
  }

  glUseProgram(shader_program_);

  const size_t number_of_vertices = point_cloud->num_points;

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * number_of_vertices,
               point_cloud->points[0], GL_STATIC_DRAW);

  const glm::mat4 depth_T_opengl = glm::inverse(opengl_T_depth);

  // Transform plane into depth camera coordinates.
  glm::vec4 camera_plane;
  PlaneTransform(plane_model_, depth_T_opengl, &camera_plane);

  glUniformMatrix4fv(mvp_handle_, 1, GL_FALSE,
                     glm::value_ptr(projection_T_depth));

  glUniform4fv(plane_handle_, 1, glm::value_ptr(camera_plane));

  // It looks better to have more points colored by the plane than the number
  // needed to be a good inlier support for fitting. Scale the distance here.
  constexpr float kDistanceScale = 5.0f;
  glUniform1f(plane_distance_handle_, kDistanceScale * plane_distance_);

  glEnableVertexAttribArray(vertices_handle_);
  glVertexAttribPointer(vertices_handle_, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

  glDrawArrays(GL_POINTS, 0, number_of_vertices);

  glDisableVertexAttribArray(vertices_handle_);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glUseProgram(0);
  tango_gl::util::CheckGlError("PointCloudRenderer::Render");
}

}  // namespace tango_plane_fitting
