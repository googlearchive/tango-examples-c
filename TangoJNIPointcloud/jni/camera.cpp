#include "camera.h"

static const glm::mat4 inverse_all_mat =
glm::mat4(-1.0, 0.0, 0.0, 0.0,
          0.0, -1.0, 0.0, 0.0,
          0.0, 0.0, -1.0, 0.0,
          0.0, 0.0, 0.0, 1.0);

Camera::Camera(){
  field_of_view = 45.0f;
  aspect_ratio = 4.0f/3.0f;
  near_clip_plane = 0.1f;
  far_clip_plane = 100.0f;
  
  rotation_mat = glm::mat4(1.0f);
}

glm::mat4 Camera::GetCurrentProjectionViewMatrix(){
  glm::mat4 projection_mat = glm::perspective(field_of_view, aspect_ratio, near_clip_plane, far_clip_plane);

  glm::mat4 translate_mat = glm::translate(glm::mat4(1.0f), -position);
  
  return projection_mat * rotation_mat * translate_mat;
}

void Camera::SetAspectRatio(float _aspect_ratio){
  aspect_ratio = _aspect_ratio;
}

void Camera::SetPosition(glm::vec3 pos){
  position = pos;
}

void Camera::SetRotation(glm::quat rot){
  rotation_mat = glm::inverse(glm::mat4_cast(rot));
}

void Camera::LookAt(glm::vec3 up_vec, glm::vec3 look_at_pos){
  rotation_mat = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                   look_at_pos,
                                   up_vec);
}

Camera::~Camera()
{
  
}
