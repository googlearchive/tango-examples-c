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

#define  LOG_TAG    "tango_pointcloud"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

class Camera{
public:
  Camera();
  ~Camera();
  glm::mat4 get_projection_view_matrix();
  void set_aspect_ratio(float _aspect_ratio);
  
  void Rotate(float w, float x, float y, float z);
  void Translate(float x, float y, float z);
private:
  glm::mat4 rotation_mat;
  glm::mat4 translate_mat;
  float position_x, position_y, position_z;
  float field_of_view;
  float aspect_ratio;
  float near_clip_plane, far_clip_plane;
};

#endif  // CAMERA_H