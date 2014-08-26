#include "gl_util.h"

void GlUtil::CheckGlError(const char* operation) {
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("after %s() glError (0x%x)\n", operation, error);
  }
}

GLuint GlUtil::LoadShader(GLenum shader_type, const char* shader_source) {
  GLuint shader = glCreateShader(shader_type);
  if (shader) {
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
      GLint info_len = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
      if (info_len) {
        char* buf = (char*) malloc(info_len);
        if (buf) {
          glGetShaderInfoLog(shader, info_len, NULL, buf);
          LOGE("Could not compile shader %d:\n%s\n", shader_type, buf);
          free(buf);
        }
        glDeleteShader(shader);
        shader = 0;
      }
    }
  }
  return shader;
}

GLuint GlUtil::CreateProgram(const char* vertex_source,
                             const char* fragment_source) {
  GLuint vertexShader = LoadShader(GL_VERTEX_SHADER, vertex_source);
  if (!vertexShader) {
    return 0;
  }

  GLuint fragment_shader = LoadShader(GL_FRAGMENT_SHADER, fragment_source);
  if (!fragment_shader) {
    return 0;
  }

  GLuint program = glCreateProgram();
  if (program) {
    glAttachShader(program, vertexShader);
    CheckGlError("glAttachShader");
    glAttachShader(program, fragment_shader);
    CheckGlError("glAttachShader");
    glLinkProgram(program);
    GLint link_status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE) {
      GLint buf_length = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &buf_length);
      if (buf_length) {
        char* buf = (char*) malloc(buf_length);
        if (buf) {
          glGetProgramInfoLog(program, buf_length, NULL, buf);
          LOGE("Could not link program:\n%s\n", buf);
          free(buf);
        }
      }
      glDeleteProgram(program);
      program = 0;
    }
  }
  return program;
}

glm::quat GlUtil::ConvertRotationToOpenGL(glm::quat rotation) {
  glm::quat rotation_offsetX = glm::rotate(rotation, 1.57079f,
                                           glm::vec3(1.0f, 0.0f, 0.0f));
  glm::quat rotation_offsetZ = glm::rotate(rotation_offsetX, 1.57079f,
                                           glm::vec3(0.0f, 0.0f, -1.0f));
  glm::quat rotation_inversed = glm::quat(glm::inverse(rotation_offsetZ));
  return glm::quat(rotation_inversed.w, rotation_inversed.y,
                   rotation_inversed.x * -1.0f, rotation_inversed.z);
}

glm::vec3 GlUtil::ConvertPositionToOpenGL(glm::vec3 position) {
  return glm::vec3(position.x, position.z, position.y * -1.0f);
}
