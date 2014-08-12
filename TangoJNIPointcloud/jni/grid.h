#ifndef GRID_H
#define GRID_H

#include <stdlib.h>
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

#define  LOG_TAG    "tango_pointcloud"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

class Grid : public DrawableObject{
public:
  Grid();
  ~Grid();
  void Render(glm::mat4 view_projection_mat);
private:
  float *vertices;
  
  GLuint vertex_buffer;
  GLuint color_buffer;
  
  GLuint shader_program;
  GLuint attrib_vertices;
  GLuint attrib_colors;
  GLuint uniform_mvp_mat;
};

#endif  // GRID_H