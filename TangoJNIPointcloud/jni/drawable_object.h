#ifndef DRAWABLE_OBJECT_H
#define DRAWABLE_OBJECT_H

#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "gl_util.h"

#define GLM_FORCE_RADIANS
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "gtc/type_ptr.hpp"

#define  LOG_TAG    "tango_pointcloud"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

class DrawableObject{
public:
  DrawableObject();
  virtual void Render(glm::mat4 view_projection_mat) = 0;
  void SetPosition(glm::vec3 pos);
  void SetRotation(glm::quat rot);
  void SetScale(glm::vec3 s);
  void Rotate(glm::quat rot);
  glm::mat4 GetCurrentModelMatrix();
private:
  GLuint shader_program;
  glm::quat rotation;
  glm::vec3 position;
  glm::vec3 scale;
};

#endif  // DRAWABLE_OBJECT_H