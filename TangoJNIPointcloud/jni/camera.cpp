#include "camera.h"

Camera::Camera()
{
  field_of_view = 45.f;
  aspect_ratio = 4.f/3.f;
  near_clip_plane = 0.1f;
  far_clip_plane = 100.f;
  
  position_z = .1f;
}

glm::mat4 Camera::get_projection_view_matrix()
{
  glm::mat4 projection_mat = glm::perspective(field_of_view, aspect_ratio, near_clip_plane, far_clip_plane);
  glm::mat4 view_mat = glm::lookAt(glm::vec3(position_x, position_y, position_z),
                                   glm::vec3(0,0,0),
                                   glm::vec3(0,1,0));
  
  return projection_mat * view_mat;
}

void Camera::set_aspect_ratio(float _aspect_ratio)
{
  aspect_ratio = _aspect_ratio;
}

void Camera::translate(float x, float y, float z)
{
  position_x += x;
  position_y += y;
  position_z += z;
}

Camera::~Camera()
{
  
}