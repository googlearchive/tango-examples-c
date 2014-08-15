#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include "gl_util.h"

class Pointcloud {
 public:
  Pointcloud();
  
  /// Render function take in the depth buffer and the buffer size, and render
  /// the points base on current transformation.
  
  /// model_view_mat: Current view projection matrix.
  ///
  /// depth_buffer_size: Number of vertices in of the data. Example: 60 floats in
  /// the buffer, the size should be 60/3 = 20;
  ///
  /// depth_data_buffer: Pointer to float array contains float triplet of each
  /// vertices in the point cloud.
  void Render(glm::mat4 view_projection_mat, float depth_buffer_size,
              float* depth_data_buffer);
 private:
  GLuint vertex_buffers_;

  GLuint shader_program_;
  GLuint attrib_vertices_;
  GLuint uniform_mvp_mat_;
};

#endif  // POINTCLOUD_H
