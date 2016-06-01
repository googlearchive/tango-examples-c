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

#ifndef TANGO_POINT_CLOUD_POINT_CLOUD_DRAWABLE_H_
#define TANGO_POINT_CLOUD_POINT_CLOUD_DRAWABLE_H_

#include <jni.h>
#include <vector>

#include <tango-gl/util.h>

namespace tango_point_cloud {

// PointCloudDrawable is responsible for the point cloud rendering.
class PointCloudDrawable {
 public:
  PointCloudDrawable();

  // Free all GL Resources, i.e, shaders, buffers.
  void DeleteGlResources();

  // Update current point cloud data.
  //
  // @param projection_mat: projection matrix from current render camera.
  // @param view_mat: view matrix from current render camera.
  // @param vertices: all vertices in this point cloud frame.
  void Render(glm::mat4 projection_mat, glm::mat4 view_mat,
              const std::vector<float>& vertices);

 private:
  // Vertex buffer of the point cloud geometry.
  GLuint vertex_buffers_;

  // Shader to display point cloud.
  GLuint shader_program_;

  // Handle to vertex attribute value in the shader.
  GLuint vertices_handle_;

  // Handle to the model view projection matrix uniform in the shader.
  GLuint mvp_handle_;
};
}  // namespace tango_point_cloud

#endif  // TANGO_POINT_CLOUD_POINT_CLOUD_DRAWABLE_H_
