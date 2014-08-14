#ifndef AXIS_H
#define AXIS_H

#include "drawable_object.h"
#include "gl_util.h"

class Axis : public DrawableObject {
 public:
  Axis();
  void Render(glm::mat4 view_projection_mat);
 private:
  GLuint vertex_buffer;
  GLuint color_buffer;

  GLuint shader_program;
  GLuint attrib_vertices;
  GLuint attrib_colors;
  GLuint uniform_mvp_mat;
};

#endif  // AXIS_H
