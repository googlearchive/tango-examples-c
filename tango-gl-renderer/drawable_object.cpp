/*
 * Copyright 2014 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
