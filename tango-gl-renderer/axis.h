#ifndef AXIS_H
#define AXIS_H

#include "drawable_object.h"
#include "gl_util.h"

class Axis : public DrawableObject {
 public:
  Axis();
  void Render(glm::mat4 view_projection_mat);
 private:
  GLuint vertex_buffer_;
  GLuint color_buffer_;

  GLuint shader_program_;
  GLuint attrib_vertices_;
  GLuint attrib_colors_;
  GLuint uniform_mvp_mat_;
};

#endif  // AXIS_H
