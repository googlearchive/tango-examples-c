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

#ifndef TANGO_PLANE_FITTING_POINT_CLOUD_RENDERER_H_
#define TANGO_PLANE_FITTING_POINT_CLOUD_RENDERER_H_

#include <mutex>
#include <utility>
#include <vector>

#include <tango_client_api.h>
#include <tango-gl/util.h>
#include <tango_support_api.h>

namespace tango_plane_fitting {

// PointCloudRenderer contains the OpenGL logic to render depth data.
class PointCloudRenderer {
 public:
  PointCloudRenderer();
  ~PointCloudRenderer();

  // Render the point cloud colored by its location relative to the
  // world plane model.
  //
  // @param projection_T_depth The pose of the openGL projection with
  // respect to the depth camera position.
  // @param start_service_T_depth The pose of the start of service with
  // respect to depth camera position.
  // @param point_cloud Depth data gathered by a PointCloudManager.
  void Render(const glm::mat4& projection_T_depth,
              const glm::mat4& start_service_T_depth,
              const TangoPointCloud* point_cloud);

  // Render depth points with debugging colors.
  void SetRenderDebugColors(bool on) { debug_colors_ = on; }

  // A plane equation in world coordinates for debug rendering.
  void SetPlaneEquation(const glm::vec4& plane) { plane_model_ = plane; }

  // A call to manually free the OpenGL resources
  void DeleteGLResources();

 private:
  GLuint shader_program_;
  GLuint vertex_buffer_;
  GLuint mvp_handle_;
  GLuint vertices_handle_;
  GLuint plane_handle_;
  GLuint plane_distance_handle_;

  // A parameter controlling inlier distance.
  GLfloat plane_distance_;

  // Controls coloring of point data.
  GLboolean debug_colors_;

  // The updated plane model after every plane fit.
  glm::vec4 plane_model_;
};

}  // namespace tango_plane_fitting

#endif  // TANGO_PLANE_FITTING_POINT_CLOUD_RENDERER_H_
