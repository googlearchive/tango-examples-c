#include "drawable_object.h"

DrawableObject::DrawableObject() {
  position = glm::vec3(0.0f, 0.0f, 0.0f);
  rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  scale = glm::vec3(1.0f, 1.0f, 1.0f);
}

void DrawableObject::SetPosition(glm::vec3 pos) {
  position = pos;
}

void DrawableObject::SetRotation(glm::quat rot) {
  rotation = rot;
}

void DrawableObject::SetScale(glm::vec3 s) {
  scale = s;
}

void DrawableObject::Rotate(glm::quat rot) {
  rotation = rot;
}

glm::mat4 DrawableObject::GetCurrentModelMatrix() {
  glm::mat4 t = glm::translate(glm::mat4(1.0f), position);
  glm::mat4 r = glm::mat4_cast(rotation);
  glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
  return t * r * s;
}
