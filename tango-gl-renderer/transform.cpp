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

#include "transform.h"

Transform::Transform() {
  position_ = glm::vec3(0.0f, 0.0f, 0.0f);
  scale_ = glm::vec3(1.0f, 1.0f, 1.0f);
  rotation_ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  parent_ = NULL;
}

void Transform::SetPosition(glm::vec3 position) {
  position_ = position;
}

glm::vec3 Transform::GetPosition() {
  return position_;
}

void Transform::SetRotation(glm::quat rotation) {
  rotation_ = rotation;
}

glm::quat Transform::GetRotation() {
  return rotation_;
}

void Transform::SetScale(glm::vec3 scale) {
  scale_ = scale;
}

glm::vec3 Transform::GetScale() {
  return scale_;
}

void Transform::Translate(glm::vec3 translation) {
  position_ += translation;
}

void Transform::SetTransformationMatrix(glm::mat4 transform_mat) {
  glm::vec3 skew;
  glm::vec4 persp;
  GlUtil::DecomposeMatrix(transform_mat, position_, rotation_, scale_);
//  glm::decompose(transform_mat, scale_, rotation_, position_, skew, persp);
}

glm::mat4 Transform::GetTransformationMatrix() {
  glm::mat4 trans_mat = glm::scale(glm::mat4_cast(rotation_), scale_);
  trans_mat[3][0] = position_.x;
  trans_mat[3][1] = position_.y;
  trans_mat[3][2] = position_.z;
  glm::mat4 parent_mat = glm::mat4(1.0f);
  if (parent_ != NULL) {
    parent_mat = parent_->GetTransformationMatrix();
    trans_mat = parent_mat * trans_mat;
  }
  return  trans_mat;
}

void Transform::SetParent(Transform* transform) {
  parent_ = transform;
}

Transform* Transform::GetParent() {
  return parent_;
}
