#include "drawable_object.h"

DrawableObject::DrawableObject() {
  position_ = glm::vec3(0.0f, 0.0f, 0.0f);
  rotation_ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  scale_ = glm::vec3(1.0f, 1.0f, 1.0f);
}

void DrawableObject::SetPosition(glm::vec3 pos) {
  position_ = pos;
}

void DrawableObject::SetRotation(glm::quat rot) {
  rotation_ = rot;
}

void DrawableObject::SetScale(glm::vec3 s) {
  scale_ = s;
}

void DrawableObject::Rotate(glm::quat rot) {
  rotation_ = rot;
}

glm::mat4 DrawableObject::GetCurrentModelMatrix() {
  glm::mat4 t = glm::translate(glm::mat4(1.0f), position_);
  glm::mat4 r = glm::mat4_cast(rotation_);
  glm::mat4 s = glm::scale(glm::mat4(1.0f), scale_);
  return t * r * s;
}
