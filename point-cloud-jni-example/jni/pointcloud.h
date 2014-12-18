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

#ifndef POINT_CLOUD_JNI_EXAMPLE_POINTCLOUD_H_
#define POINT_CLOUD_JNI_EXAMPLE_POINTCLOUD_H_

#include "tango_data.h"
#include "tango-gl-renderer/gl_util.h"

class Pointcloud {
 public:
  Pointcloud();

  /// Render function take in the depth buffer and the buffer size, and render
  /// the points base on current transformation.
  ///
  /// depth_buffer_size: Number of vertices in of the data. Example: 60 floats
  /// in the buffer, the size should be 60/3 = 20;
  ///
  /// depth_data_buffer: Pointer to float array contains float triplet of each
  /// vertices in the point cloud.
  void Render(glm::mat4 projection_mat, glm::mat4 view_mat, glm::mat4 model_mat,
              int depth_buffer_size, float* depth_data_buffer);
 private:
  GLuint vertex_buffers_;

  GLuint shader_program_;
  GLuint attrib_vertices_;
  GLuint uniform_mvp_mat_;
};

#endif  // POINT_CLOUD_JNI_EXAMPLE_POINTCLOUD_H_
