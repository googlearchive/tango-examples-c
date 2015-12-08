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

#ifndef TANGO_PLANE_FITTING_POINT_CLOUD_H_
#define TANGO_PLANE_FITTING_POINT_CLOUD_H_

#include <mutex>
#include <utility>
#include <vector>

#include <tango_client_api.h>
#include <tango-gl/util.h>
#include <tango_support_api.h>

namespace tango_plane_fitting {

// PointCloud contains the logic to maintain an updated, renderable depth
// buffer.
class PointCloud {
 public:
  PointCloud(int32_t max_point_cloud_size);
  ~PointCloud();

  // Update the point cloud data with the latest results from the
  // callback. This is intended to be called from the callback thread.
  void UpdateVertices(const TangoXYZij* cloud);

  // Render the point cloud colored by its location relative to the
  // world plane model.
  //
  // @param projection The OpenGL projection matrix.
  // @param opengl_camera_T_start_service The pose of the OpenGL camera with
  // respect to start of service.
  // @param device_T_depth Fixed extrinsics of pose of depth camera
  // with respect to device (not time-varying).
  void Render(const glm::mat4& projection,
              const glm::mat4& opengl_camera_T_start_service,
              const glm::mat4& device_T_depth);

  // Render depth points with debugging colors.
  void SetRenderDebugColors(bool on) { debug_colors_ = on; }

  // A plane equation in world coordinates for debug rendering.
  void SetPlaneEquation(const glm::vec4& plane) { plane_model_ = plane; }

  // Get a reference to the current point data.
  const TangoXYZij* GetCurrentPointData();

  // Get a copy of the current point cloud transform of device with respect to
  // start of service.
  const glm::mat4 GetPointCloudStartServiceTDeviceTransform();

 private:
  bool UpdateRenderPoints();
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

  // Cached transform from opengl world to tango world.
  // This is initialized and never updated.
  glm::mat4 opengl_world_T_start_service_;

  // Point data manager.
  TangoSupportPointCloudManager* point_cloud_manager_;
  TangoXYZij* front_cloud_;
};

}  // namespace tango_plane_fitting

#endif  // TANGO_PLANE_FITTING_POINT_CLOUD_H_
