/*
 * Copyright 2014 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "tango-gl-renderer/gl_util.h"

const glm::mat4 GlUtil::ss_to_ow_mat =
    glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
              0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

const glm::mat4 GlUtil::oc_to_c_mat =
    glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
              -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

const glm::mat4 GlUtil::oc_to_d_mat =
    glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0, cos(13 * 3.14f / 180.0f),
              sin(13 * 3.14f / 180.0f), 0, 0, -sin(13 * 3.14f / 180.0f),
              cos(13 * 3.14f / 180.0f), 0, 0.0f, 0.0f, 0.0f, 1.0f);

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

glm::quat GlUtil::ConvertRotationToOpenGL(const glm::quat& rotation) {
  // The conversion quaternion is equivalent to this conversion matrix below
  // NOTE: the following matrix is a column major matrix, both openGL and glm
  // take column major matrix.
  //
  // float conversionArray[16] = {
  //     1.0f, 0.0f, 0.0f, 0.0f,
  //     0.0f, 0.0f, -1.0f, 0.0f,
  //     0.0f, 1.0f, 0.0f, 0.0f,
  //     0.0f, 0.0f, 0.0f, 1.0f
  // };
  // glm::mat4 conversionMatrix;
  // memcpy(glm::value_ptr(conversionMatrix), conversionArray, sizeof(conversionArray));
  //
  // glm::mat4 resultMatrix= conversionMatrix*glm::mat4_cast(rotation);
  // return glm::quat_cast(resultMatrix);

  const float M_SQRT_2_OVER_2 = sqrt(2) / 2.0f;
  glm::quat conversionQuaternion = glm::quat(M_SQRT_2_OVER_2, -M_SQRT_2_OVER_2,
                                             0.0f, 0.0f);

  // Quaternion cumulate in the reverse way (like matrix multiplication).
  // The following line is applying the conversionQuaternion to the rotation.
  return conversionQuaternion * rotation;
}

glm::vec3 GlUtil::ConvertPositionToOpenGL(const glm::vec3& position) {
  return glm::vec3(position.x, position.z, position.y * -1.0f);
}

void GlUtil::DecomposeMatrix (const glm::mat4& transform_mat,
                              glm::vec3& translation,
                              glm::quat& rotation,
                              glm::vec3& scale) {
  float scale_x = glm::length( glm::vec3( transform_mat[0][0], transform_mat[1][0], transform_mat[2][0] ) );
  float scale_y = glm::length( glm::vec3( transform_mat[0][1], transform_mat[1][1], transform_mat[2][1] ) );
  float scale_z = glm::length( glm::vec3( transform_mat[0][2], transform_mat[1][2], transform_mat[2][2] ) );


  float determinant = glm::determinant( transform_mat );
  if( determinant < 0.0 )
    scale_x = -scale_x;

  translation.x = transform_mat[3][0];
  translation.y = transform_mat[3][1];
  translation.z = transform_mat[3][2];

  float inverse_scale_x = 1.0 / scale_x;
  float inverse_scale_y = 1.0 / scale_y;
  float inverse_scale_z = 1.0 / scale_z;

  glm::mat4 transform_unscaled = transform_mat;

  transform_unscaled[0][0] *= inverse_scale_x;
  transform_unscaled[1][0] *= inverse_scale_x;
  transform_unscaled[2][0] *= inverse_scale_x;

  transform_unscaled[0][1] *= inverse_scale_y;
  transform_unscaled[1][1] *= inverse_scale_y;
  transform_unscaled[2][1] *= inverse_scale_y;

  transform_unscaled[0][2] *= inverse_scale_z;
  transform_unscaled[1][2] *= inverse_scale_z;
  transform_unscaled[2][2] *= inverse_scale_z;

  rotation = glm::quat_cast( transform_mat );

  scale.x = scale_x;
  scale.y = scale_y;
  scale.z = scale_z;
}

glm::vec3 GlUtil::GetTranslationFromMatrix(const glm::mat4& transform_mat) {
  glm::vec3 translation;
  translation.x = transform_mat[3][0];
  translation.y = transform_mat[3][1];
  translation.z = transform_mat[3][2];
  return translation;
}

float GlUtil::Clamp(float value, float min, float max) {
  return value < min ? min : (value > max ? max : value);
}

// Print out a column major matrix.
void GlUtil::PrintMatrix(const glm::mat4& matrix) {
  int i;
  for (i = 0; i < 4; i++) {
    LOGI("[ %f, %f, %f, %f ]",
         matrix[0][i], matrix[1][i], matrix[2][i], matrix[3][i]);
  }
  LOGI(" ");
}

void GlUtil::PrintVector(const glm::vec3& vector) {
  LOGI("[ %f, %f, %f ]", vector[0], vector[1], vector[2]);
  LOGI(" ");
}

glm::vec3 GlUtil::LerpVector(const glm::vec3& x, const glm::vec3& y, float a) {
  return x * (1.0f - a) + y * a;
}

float GlUtil::DistanceSquared(const glm::vec3& v1, const glm::vec3& v2) {
  glm::vec3 delta = v2 - v1;
  return glm::dot(delta, delta);
}
