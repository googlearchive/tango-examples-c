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

#include "camera.h"

Camera::Camera() {
  field_of_view_ = 45.0f;
  aspect_ratio_ = 4.0f / 3.0f;
  near_clip_plane_ = 0.1f;
  far_clip_plane_ = 100.0f;

  rotation_mat_ = glm::mat4(1.0f);
}

glm::mat4 Camera::GetCurrentProjectionViewMatrix() {
  glm::mat4 projection_mat = glm::perspective(field_of_view_, aspect_ratio_,
                                              near_clip_plane_, far_clip_plane_);

  glm::mat4 translate_mat = glm::translate(glm::mat4(1.0f), -position_);

  return projection_mat * rotation_mat_ * translate_mat;
}

void Camera::SetAspectRatio(float aspect_ratio) {
  aspect_ratio_ = aspect_ratio;
}

void Camera::SetPosition(glm::vec3 pos) {
  position_ = pos;
}

void Camera::SetRotation(glm::quat rot) {
  rotation_mat_ = glm::inverse(glm::mat4_cast(rot));
}

void Camera::LookAt(glm::vec3 cam_pos, glm::vec3 look_at_pos, glm::vec3 up_vec) {
  rotation_mat_ = glm::lookAt(cam_pos, look_at_pos, up_vec);
}

Camera::~Camera() {

}
