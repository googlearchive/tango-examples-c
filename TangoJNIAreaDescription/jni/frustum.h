#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "drawable_object.h"
#include "gl_util.h"

class Frustum : public DrawableObject {
 public:
  Frustum();
  void Render(glm::mat4 view_projection_mat);
 private:
  GLuint vertex_buffer_;
  GLuint shader_program_;
  GLuint attrib_vertices_;
  GLuint uniform_mvp_mat_;
};

#endif  // FRUSTUM_H
