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
