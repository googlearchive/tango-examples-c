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

#include "tango-gl-renderer/ar_ruler.h"

static const char kVertexShader[] =
    "attribute vec4 vertex;\n"
    "uniform vec4 color;\n"
    "uniform mat4 mvp;\n"
    "varying vec4 v_color;\n"
    "void main() {\n"
    "  gl_Position = mvp*vertex;\n"
    "  v_color = color;\n"
    "}\n";

static const char kFragmentShader[] = "varying vec4 v_color;\n"
    "void main() {\n"
    "  gl_FragColor = v_color;\n"
    "}\n";

static const float vertices[] = { -0.05f, 0.0f, 0.0f,
    0.05f, 0.0f, 0.0f,
    0.05f, 0.0f, -50.0f,
    0.05f, 0.0f, -50.0f,
    -0.05f,0.0f,-50.0f,
    -0.05f, 0.0f, 0.0f
};

ArRuler::ArRuler() : alpha_(1.0f) {
  shader_program_ = GlUtil::CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program_) {
    LOGE("Could not create program.");
  }
  uniform_mvp_mat_ = glGetUniformLocation(shader_program_, "mvp");
  uniform_color_ = glGetUniformLocation(shader_program_, "color");
  attrib_vertices_ = glGetAttribLocation(shader_program_, "vertex");

  glGenBuffers(1, &vertex_buffer_);
  glGenBuffers(1, &color_buffer_);
}

ArRuler::~ArRuler() { glDeleteShader(shader_program_); }

void ArRuler::SetColor(const float color[4]) {
  memcpy(color_, color, 4 * sizeof(float));
}

void ArRuler::SetAlpha(const float alpha) {
  alpha_ = alpha;
}

void ArRuler::Render(const glm::mat4& projection_mat,
                     const glm::mat4& view_mat) const {
  glUseProgram(shader_program_);

  // Calculate model view projection matrix of this object.
  glm::mat4 model_mat = GetTransformationMatrix();
  glm::mat4 mvp_mat = projection_mat * view_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  glUniform4f(uniform_color_, color_[0], color_[1], color_[2], alpha_);

  // Binding vertex buffer.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3*6, vertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDrawArrays(GL_TRIANGLES, 0, 3*6);
  glUseProgram(0);
}
