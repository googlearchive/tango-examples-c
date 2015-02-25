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

#include "tango-gl/polygon.h"
#include "tango-gl/util.h"

namespace tango_gl {

static const char kVertexShader[] =
    "attribute vec4 vertex;\n"
    "uniform mat4 mvp_matrix;\n"
    "uniform vec4 color;\n"
    "varying vec4 v_color;\n"
    "void main() {\n"
    "  gl_Position = mvp_matrix*vertex;\n"
    "  v_color = color;\n"
    "}\n";

static const char kFragmentShader[] =
    "varying vec4 v_color;\n"
    "void main() {\n"
    "  gl_FragColor = v_color;\n"
    "}\n";

Polygon::Polygon() {
  shader_program_ = util::CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program_) {
    LOGE("Could not create program.");
  }
  uniform_mvp_mat_ = glGetUniformLocation(shader_program_, "mvp_matrix");
  attrib_vertices_ = glGetAttribLocation(shader_program_, "vertex");
  uniform_color_ = glGetUniformLocation(shader_program_, "color");
  alpha_ = 0.5f;
}

Polygon::~Polygon() { glDeleteShader(shader_program_); }

void Polygon::SetAlpha(const float alpha) { alpha_ = alpha; }

void Polygon::SetMeshData(const std::vector<GLfloat>& vertices,
                          const std::vector<GLushort>& indices) {
  vertices_ = vertices;
  indices_ = indices;
}

void Polygon::Render(const glm::mat4& projection_mat,
                     const glm::mat4& view_mat) const {
  if (indices_.empty())
    return;
  glUseProgram(shader_program_);
  glm::mat4 model_mat = GetTransformationMatrix();
  glm::mat4 mvp_mat = projection_mat * view_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE,
                        3 * sizeof(GLfloat), vertices_.data());
  glUniform4f(uniform_color_, 1.0f, 0.0f, 0.0, alpha_);
  glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_SHORT,
                 indices_.data());

  glDisableVertexAttribArray(attrib_vertices_);
  glUseProgram(0);
}

}  // namespace tango_gl
