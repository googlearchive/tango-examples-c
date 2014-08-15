#ifndef DRAWABLE_OBJECT_H
#define DRAWABLE_OBJECT_H

#include "gl_util.h"

class DrawableObject {
 public:
  DrawableObject();
  virtual void Render(glm::mat4 view_projection_mat) = 0;
  void SetPosition(glm::vec3 pos);
  void SetRotation(glm::quat rot);
  void SetScale(glm::vec3 s);
  void Rotate(glm::quat rot);
  glm::mat4 GetCurrentModelMatrix();
 private:
  GLuint shader_program;
  glm::quat rotation;
  glm::vec3 position;
  glm::vec3 scale;
};
#endif  // DRAWABLE_OBJECT_H
