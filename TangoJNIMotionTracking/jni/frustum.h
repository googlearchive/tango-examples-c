#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "drawable_object.h"
#include "gl_util.h"

#define  LOG_TAG    "tango_motion_tracking"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

class Frustum : public DrawableObject {
 public:
  Frustum();
  void Render(glm::mat4 view_projection_mat);
 private:
  GLuint vertex_buffer;
  GLuint shader_program;
  GLuint attrib_vertices;
  GLuint uniform_mvp_mat;
};

#endif  // FRUSTUM_H
