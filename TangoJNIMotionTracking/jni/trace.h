#ifndef TRACE_H
#define TRACE_H

#include <stdlib.h>
#include <vector>

#include "drawable_object.h"
#include "gl_util.h"

class Trace : public DrawableObject {
 public:
  Trace();
  void UpdateVertexArray(glm::vec3 v);
  void Render(glm::mat4 view_projection_mat);
 private:
  std::vector<glm::vec3> vertices;

  GLuint shader_program;
  GLuint attrib_vertices;
  GLuint uniform_mvp_mat;
};

#endif  // TRACE_H
