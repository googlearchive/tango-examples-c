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
  glm::quat rotation_;
  glm::vec3 position_;
  glm::vec3 scale_;
};
#endif  // DRAWABLE_OBJECT_H
