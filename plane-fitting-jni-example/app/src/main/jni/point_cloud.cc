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

#include "tango-plane-fitting/point_cloud.h"

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

PointCloud::PointCloud(int32_t max_point_cloud_size)
    : plane_distance_(0.05f),
      debug_colors_(false),
      plane_model_(glm::vec4(0.0, 0.0, 1.0, 0.0)),
      points_back_(max_point_cloud_size),
      points_swap_(max_point_cloud_size),
      points_front_(max_point_cloud_size) {
  opengl_world_T_start_service_ =
      tango_gl::conversions::opengl_world_T_tango_world();

  shader_program_ = tango_gl::util::CreateProgram(
      kPointCloudVertexShader.c_str(), kPointCloudFragmentShader.c_str());

  glGenBuffers(1, &vertex_buffer_);

  mvp_handle_ = glGetUniformLocation(shader_program_, "mvp");
  vertices_handle_ = glGetAttribLocation(shader_program_, "vertex");
  plane_handle_ = glGetUniformLocation(shader_program_, "plane");
  plane_distance_handle_ =
      glGetUniformLocation(shader_program_, "plane_distance");

  tango_gl::util::CheckGlError("Pointcloud::Construction");
}

PointCloud::~PointCloud() {
  glDeleteProgram(shader_program_);
  glDeleteBuffers(0, &vertex_buffer_);
}

void PointCloud::UpdateVertices(const TangoXYZij* cloud) {
  // Get the transform.
  TangoCoordinateFramePair frame_pair;
  frame_pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  frame_pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  TangoPoseData pose_start_service_T_device_t1;

  if (TangoService_getPoseAtTime(cloud->timestamp, frame_pair,
                                 &pose_start_service_T_device_t1) !=
      TANGO_SUCCESS) {
    LOGE("PointCloud: Could not localize point cloud data");
    return;
  }

  const glm::mat4 start_service_T_device =
      tango_gl::conversions::TransformFromArrays(
          pose_start_service_T_device_t1.translation,
          pose_start_service_T_device_t1.orientation);

  // Copy into back buffer.
  TangoSupport_copyXYZij(cloud, &points_back_.cloud);
  points_back_.start_service_T_device_t1 = start_service_T_device;

  {
    std::lock_guard<std::mutex> lock(buffer_lock_);
    SwapPointCloudData(points_back_, points_swap_);
  }
}

PointCloud::PointData::PointData(int32_t max_point_cloud_size)
    : start_service_T_device_t1(1.0) {
  TangoSupport_createXYZij(max_point_cloud_size, &cloud);
}

PointCloud::PointData::~PointData() { TangoSupport_freeXYZij(&cloud); }

void PointCloud::SwapPointCloudData(PointCloud::PointData& a,
                                    PointCloud::PointData& b) const {
  std::swap(a.cloud.xyz, b.cloud.xyz);
  std::swap(a.cloud.xyz_count, b.cloud.xyz_count);
  std::swap(a.cloud.timestamp, b.cloud.timestamp);
  std::swap(a.start_service_T_device_t1, b.start_service_T_device_t1);
}

bool PointCloud::UpdateRenderPoints() {
  bool updated = false;
  std::lock_guard<std::mutex> lock(buffer_lock_);
  if (points_swap_.cloud.timestamp > points_front_.cloud.timestamp) {
    updated = true;
    SwapPointCloudData(points_swap_, points_front_);
  }
  return updated;
}

void PointCloud::Render(const glm::mat4& projection,
                        const glm::mat4& opengl_camera_T_start_service,
                        const glm::mat4& device_T_depth) {
  // Update point data.
  const bool points_need_buffering = this->UpdateRenderPoints();
  if (!debug_colors_) {
    return;
  }

  glUseProgram(shader_program_);

  const size_t number_of_vertices = points_front_.cloud.xyz_count;

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  if (points_need_buffering) {
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * number_of_vertices,
                 points_front_.cloud.xyz[0], GL_STATIC_DRAW);
  }
  const glm::mat4 start_service_T_device_t1 =
      points_front_.start_service_T_device_t1;
  const glm::mat4 mvp_mat = projection * opengl_camera_T_start_service *
                            start_service_T_device_t1 * device_T_depth;

  const glm::mat4 depth_T_opengl =
      glm::inverse(opengl_world_T_start_service_ * start_service_T_device_t1 *
                   device_T_depth);

  // Transform plane into depth camera coordinates.
  glm::vec4 camera_plane;
  PlaneTransform(plane_model_, depth_T_opengl, &camera_plane);

  glUniformMatrix4fv(mvp_handle_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  glUniform4fv(plane_handle_, 1, glm::value_ptr(camera_plane));

  // It looks better to have more points colored by the plane than the number
  // needed to be a good inlier support for fitting. Scale the distance here.
  constexpr float kDistanceScale = 5.0f;
  glUniform1f(plane_distance_handle_, kDistanceScale * plane_distance_);

  glEnableVertexAttribArray(vertices_handle_);
  glVertexAttribPointer(vertices_handle_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  glDrawArrays(GL_POINTS, 0, number_of_vertices);

  glDisableVertexAttribArray(vertices_handle_);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glUseProgram(0);
  tango_gl::util::CheckGlError("Pointcloud::Render");
}

}  // namespace tango_plane_fitting
