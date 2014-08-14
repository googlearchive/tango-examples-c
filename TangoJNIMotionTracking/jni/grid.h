#ifndef GRID_H
#define GRID_H

#include "drawable_object.h"
#include "gl_util.h"

class Grid : public DrawableObject {
 public:
  Grid();
  ~Grid();
  void Render(glm::mat4 view_projection_mat);
 private:
  float *vertices;
  float density;
  int quantity;
  int traverse_len;

  GLuint vertex_buffer;
  GLuint color_buffer;

  GLuint shader_program;
  GLuint attrib_vertices;
  GLuint attrib_colors;
  GLuint uniform_mvp_mat;
};

#endif  // GRID_H
