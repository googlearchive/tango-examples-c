#ifndef POINTCLOUD_H
#define POINTCLOUD_H

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

class Pointcloud{
public:
  Pointcloud();
  void Render(glm::mat4 model_view_mat, float depth_buffer_size, float *depth_data_buffer);
private:
  GLuint vertex_buffers;
  
  GLuint shader_program;
  GLuint attrib_vertices;
  GLuint uniform_mvp_mat;
};

#endif  // POINTCLOUD_H