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
#include "tango-gl-renderer/frustum.h"
#include "tango-gl-renderer/gl_util.h"

static const char kVertexShader[] =
    "attribute vec4 vertex;\n"
    "uniform mat4 mvp;\n"
    "void main() {\n"
    "  gl_Position = mvp*vertex;\n"
    "}\n";

static const char kFragmentShader[] =
    "void main() {\n"
    "  gl_FragColor = vec4(0,0,0,1);\n"
    "}\n";

static const float vertices[] = {
    0.0f,  0.0f,  0.0f,  -1.0f, 1.0f,  -1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
    1.0f,  -1.0f, 0.0f,  0.0f,  0.0f,  -1.0f, -1.0f, -1.0f, 0.0f,  0.0f,
    0.0f,  1.0f,  -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f,
    1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f};

Frustum::Frustum() {
  shader_program_ = GlUtil::CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program_) {
    LOGE("Could not create program.");
  }
  uniform_mvp_mat_ = glGetUniformLocation(shader_program_, "mvp");
  attrib_vertices_ = glGetAttribLocation(shader_program_, "vertex");

  glGenBuffers(1, &vertex_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 8, vertices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Frustum::~Frustum() { glDeleteShader(shader_program_); }

void Frustum::Render(const glm::mat4& projection_mat,
                     const glm::mat4& view_mat) const {
  glUseProgram (shader_program_);
  glLineWidth(3.0f);

  // Calculate MVP matrix and pass it to shader.
  glm::mat4 model_mat = GetTransformationMatrix();
  glm::mat4 mvp_mat = projection_mat * view_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  // Vertice binding
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glEnableVertexAttribArray (attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDrawArrays(GL_LINES, 0, 6 * 8);
  glLineWidth(1.0f);
  glUseProgram(0);
}
