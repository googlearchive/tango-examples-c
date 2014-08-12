#include "camera.h"

static const glm::mat4 inverse_all_mat =
glm::mat4(-1.0, 0.0, 0.0, 0.0,
          0.0, -1.0, 0.0, 0.0,
          0.0, 0.0, -1.0, 0.0,
          0.0, 0.0, 0.0, 1.0);

Camera::Camera(){
  field_of_view = 45.f;
  aspect_ratio = 4.f/3.f;
  near_clip_plane = 0.1f;
  far_clip_plane = 100.f;
  
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
//  rot = rot*glm::quat(0.99996f, 0.0f, 0.00873, 0.0f);
  rotation_mat = glm::inverse(glm::mat4_cast(rot));
//  rotation_mat = glm::inverse(glm::rotate(glm::mat4(1.0f), 1.0f, glm::vec3()));
//  rotation_mat = glm::rotate(rotation_mat, 0.01f, glm::vec3(0,1,0));
}

// this doesn't work as expected, don't use now.
void Camera::LookAt(glm::vec3 up_vec, glm::vec3 look_at_pos){
  rotation_mat = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                   look_at_pos,
                                   up_vec);
}

Camera::~Camera()
{
  
}
