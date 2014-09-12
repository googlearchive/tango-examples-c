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
}

glm::mat4 Camera::GetViewMatrix() {
  glm::mat4 translate_mat = glm::translate(glm::mat4(1.0f), position_);
  glm::mat4 rotation_mat = glm::mat4_cast(rotation_);
  glm::mat4 view_mat = glm::inverse(translate_mat*rotation_mat);
  return view_mat;
}

glm::mat4 Camera::GetProjectionMatrix() {
  return glm::perspective(field_of_view_, aspect_ratio_,
                          near_clip_plane_, far_clip_plane_);
}

void Camera::SetAspectRatio(float aspect_ratio) {
  aspect_ratio_ = aspect_ratio;
}

void Camera::SetPosition(glm::vec3 pos) {
  position_ = pos;
}

glm::vec3 Camera::GetPosition() {
  return position_;
}

void Camera::SetRotation(glm::quat rot) {
  rotation_ = rot;
}

glm::quat Camera::GetRotation() {
  return rotation_;
}

Camera::~Camera() {

}
