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

#ifndef CAMERA_H
#define CAMERA_H

#include "gl_util.h"

class Camera {
 public:
  Camera();
  ~Camera();

  void SetAspectRatio(float aspect_ratio);
  void SetPosition(glm::vec3 pos);
  void SetRotation(glm::quat rot);
  void SetScale(glm::vec3 s);
  void LookAt(glm::vec3 cam_pos, glm::vec3 up_vec, glm::vec3 look_at_pos);

  glm::mat4 GetCurrentProjectionViewMatrix();
 private:
  float field_of_view_;
  float aspect_ratio_;
  float near_clip_plane_, far_clip_plane_;

  glm::mat4 rotation_mat_;
  glm::vec3 position_;
  glm::vec3 scale_;
};

#endif  // CAMERA_H
