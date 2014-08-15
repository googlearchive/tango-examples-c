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
