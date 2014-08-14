#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include "gl_util.h"

class Pointcloud {
 public:
  Pointcloud();
  void Render(glm::mat4 model_view_mat, float depth_buffer_size,
              float *depth_data_buffer);
 private:
  GLuint vertex_buffers;

  GLuint shader_program;
  GLuint attrib_vertices;
  GLuint uniform_mvp_mat;
};

#endif  // POINTCLOUD_H
