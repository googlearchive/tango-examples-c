#ifndef CAMERA_H
#define CAMERA_H

#include "gl_util.h"

class Camera {
 public:
  Camera();
  ~Camera();

  void SetAspectRatio(float _aspect_ratio);
  void SetPosition(glm::vec3 pos);
  void SetRotation(glm::quat rot);
  void SetScale(glm::vec3 s);
  void LookAt(glm::vec3 up_vec, glm::vec3 look_at_pos);

  glm::mat4 GetCurrentProjectionViewMatrix();
 private:
  float field_of_view;
  float aspect_ratio;
  float near_clip_plane, far_clip_plane;

  glm::mat4 rotation_mat;
  glm::vec3 position;
  glm::vec3 scale;
};

#endif  // CAMERA_H
