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

#include "tango-gl-renderer/cone.h"

static const char kVertexShader[] =
    "attribute vec4 vertex;\n"
    "uniform mat4 mvp_matrix;\n"
    "attribute vec4 color;\n"
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

Cone::Cone() {
  shader_program_ = GlUtil::CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program_) {
    LOGE("Could not create program.");
  }
  uniform_mvp_mat_ = glGetUniformLocation(shader_program_, "mvp_matrix");
  attrib_vertices_ = glGetAttribLocation(shader_program_, "vertex");
  attrib_colors_ = glGetAttribLocation(shader_program_, "color");
  SetVertexArray();
}

Cone::~Cone() { glDeleteShader(shader_program_); }

void Cone::SetAlpha(const float alpha) {
  alpha_ = alpha;
  unsigned int resolution = 360;
  colors_.clear();
  colors_.push_back(glm::vec4(1.0f, 1.0f, 0.0, alpha_));
  for (int i = resolution; i >= 0; i--) {
    colors_.push_back(glm::vec4(1.0f, 1.0f, 0.0f, alpha_*0.1f));
  }
}

void Cone::SetVertexArray() {
  unsigned int resolution = 360;
  float radius = 0.25f;
  float height = 1.0f;
  vertices_.push_back(glm::vec3(0.0, height, 0.0));
  for (int i = resolution; i >= 0; i--) {
    float theta = ((float)i / 180.0f) * 3.1415f;
    vertices_.push_back(
        glm::vec3(cos(theta) * radius, 0.0, sin(theta) * radius));
  }
}

void Cone::Render(const glm::mat4& projection_mat,
                  const glm::mat4& view_mat) const {
  glUseProgram(shader_program_);
  glm::mat4 model_mat = GetTransformationMatrix();
  glm::mat4 mvp_mat = projection_mat * view_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE,
                        sizeof(glm::vec3), &vertices_[0]);

  glEnableVertexAttribArray(attrib_colors_);
  glVertexAttribPointer(attrib_colors_, 4, GL_FLOAT, GL_FALSE,
                        sizeof(glm::vec4), &colors_[0]);

  glDrawArrays(GL_TRIANGLE_FAN, 0, vertices_.size());

  glDisableVertexAttribArray(attrib_colors_);
  glDisableVertexAttribArray(attrib_vertices_);
  glUseProgram(0);
}
