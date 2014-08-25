#ifndef GRID_H
#define GRID_H

#include "drawable_object.h"
#include "gl_util.h"

class Grid : public DrawableObject {
 public:
  Grid(float density = 0.2f, int quantity = 100);
  ~Grid();
  void Render(glm::mat4 view_projection_mat);
 private:
  float* vertices_;
  float density_;
  int quantity_;
  int traverse_len_;

  GLuint vertex_buffer_;

  GLuint shader_program_;
  GLuint attrib_vertices_;
  GLuint uniform_mvp_mat_;
};

#endif  // GRID_H
