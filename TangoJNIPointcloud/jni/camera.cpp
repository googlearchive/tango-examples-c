#include "camera.h"

Camera::Camera()
{
  field_of_view = 45.f;
  aspect_ratio = 4.f/3.f;
  near_clip_plane = 0.1f;
  far_clip_plane = 100.f;
  
  position_z = .1f;
  rotation_mat = glm::mat4(1.0f);
  translate_mat = glm::mat4(1.0f);
}

glm::mat4 Camera::get_projection_view_matrix()
{
  glm::mat4 projection_mat = glm::perspective(field_of_view, aspect_ratio, near_clip_plane, far_clip_plane);
  glm::mat4 view_mat = glm::lookAt(glm::vec3(0, 3, 3),
                                   glm::vec3(0, 0, -1),
                                   glm::vec3(0, 1, 0));
  
  return projection_mat * view_mat * rotation_mat;
}

void Camera::Rotate(float w, float x, float y, float z)
{
  rotation_mat = glm::rotate(rotation_mat, 0.01f, glm::vec3(0,1,0));
}

void Camera::set_aspect_ratio(float _aspect_ratio)
{
  aspect_ratio = _aspect_ratio;
}

void Camera::Translate(float x, float y, float z)
{
  position_x += x;
  position_y += y;
  position_z += z;
}

Camera::~Camera()
{
  
}