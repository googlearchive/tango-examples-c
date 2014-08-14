#ifndef CAMERA_H
#define CAMERA_H

#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define GLM_FORCE_RADIANS
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "gtc/type_ptr.hpp"

#define  LOG_TAG    "tango_motion_tracking"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

class Camera{
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
//  glm::quat rotation;
  glm::vec3 position;
  glm::vec3 scale;
};

#endif  // CAMERA_H
