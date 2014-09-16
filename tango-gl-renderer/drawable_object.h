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

#ifndef DRAWABLE_OBJECT_H
#define DRAWABLE_OBJECT_H

#include "gl_util.h"

class DrawableObject {
 public:
  DrawableObject();
  virtual void Render(glm::mat4 projection_mat, glm::mat4 view_mat) = 0;
  void SetPosition(glm::vec3 pos);
  void SetRotation(glm::quat rot);
  void SetScale(glm::vec3 s);
  void Rotate(glm::quat rot);
  void SetOffset(glm::vec3 offset);
  glm::mat4 GetCurrentModelMatrix();
  glm::mat4 model_mat;
private:
  glm::quat rotation_;
  glm::vec3 position_;
  glm::vec3 scale_;
  glm::vec3 offset_;
};
#endif  // DRAWABLE_OBJECT_H
