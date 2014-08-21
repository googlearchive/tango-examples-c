#ifndef GL_UTIL_H
#define GL_UTIL_H
#define GLM_FORCE_RADIANS

#define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642

#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "gtc/type_ptr.hpp"

#define LOG_TAG "tango_area_description"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

class GlUtil {
 public:
  static void CheckGlError(const char* operation);
  static GLuint CreateProgram(const char* vertex_source,
                              const char* fragment_source);
  static glm::quat ConvertRotationToOpenGL(glm::quat rotation);
  static glm::vec3 ConvertPositionToOpenGL(glm::vec3 position);
 private:
  static GLuint LoadShader(GLenum shader_type, const char* shader_source);
};

#endif  // GL_UTIL
