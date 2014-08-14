#ifndef TRACE_H
#define TRACE_H

#include <stdlib.h>
#include <vector>
#include <jni.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "gl_util.h"
#include "drawable_object.h"

#define GLM_FORCE_RADIANS
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "gtc/type_ptr.hpp"

#define  LOG_TAG    "tango_motion_tracking"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


class Trace : public DrawableObject{
public:
  Trace();
  void UpdateVerticesArray(glm::vec3 v);
  void Render(glm::mat4 view_projection_mat);
private:
  std::vector<glm::vec3> vertices;

  GLuint shader_program;
  GLuint attrib_vertices;
  GLuint uniform_mvp_mat;
};

#endif  // TRACE_H
