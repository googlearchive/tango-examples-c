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

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "gl_util.h"

class Transform {
 public:
  Transform();
  
  void SetPosition(glm::vec3 position);
  glm::vec3 GetPosition();
  
  void SetRotation(glm::quat rotation);
  glm::quat GetRotation();

  void SetScale(glm::vec3 scale);
  glm::vec3 GetScale();

  // To be implemented.
  void SetTransformationMatrix(glm::mat4 transform_mat);
  glm::mat4 GetTransformationMatrix();
  
  void SetParent(Transform* transform);
  Transform* GetParent();
 private:
  Transform *parent_;
  
  glm::vec3 position_;
  glm::quat rotation_;
  glm::vec3 scale_;
};

#endif  // TRANSFORM_H
