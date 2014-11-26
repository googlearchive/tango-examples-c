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

#include "tango-gl-renderer/band.h"

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

static const float kMinDistanceSquared = 0.0025f;

Band::Band(const unsigned int max_length) : band_width_(0.2) {
  shader_program_ = GlUtil::CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program_) {
    LOGE("Could not create program.");
  }
  max_length_ = max_length;
  uniform_mvp_mat_ = glGetUniformLocation(shader_program_, "mvp_matrix");
  attrib_vertices_ = glGetAttribLocation(shader_program_, "vertex");
  uniform_color_ = glGetUniformLocation(shader_program_, "color");
  vertices_.reserve(max_length);
  alpha_ = 1.0f;
}

Band::~Band() { glDeleteShader(shader_program_); }

void Band::SetColor(const float color[4]) {
  memcpy(band_color_, color, 4 * sizeof(float));
}

void Band::SetAlpha(const float alpha) { alpha_ = alpha; }

void Band::SetWidth(const float width) {
  band_width_ = width;
}

void Band::UpdateVertexArray(const glm::mat4 m) {
  if (vertices_.size() == 0 ||
      GlUtil::DistanceSquared(
          (vertices_[vertices_.size() - 1] + vertices_[vertices_.size() - 2]) *
              0.5f,
          glm::vec3(m[3][0], m[3][1], m[3][2])) < kMinDistanceSquared) {
    glm::mat4 left = glm::mat4(1.0f);
    left[3][0] = -band_width_ / 2.0f;
    glm::mat4 right = glm::mat4(1.0f);
    right[3][0] = band_width_ / 2.0f;
    left = m * left;
    right = m * right;

    vertices_.push_back(glm::vec3(left[3][0], left[3][1], left[3][2]));
    vertices_.push_back(glm::vec3(right[3][0], right[3][1], right[3][2]));
    if (vertices_.size() > max_length_) {
      vertices_.erase(vertices_.begin());
      vertices_.erase(vertices_.begin());
    }
  }
}

void Band::SetVertexArray(const std::vector<glm::vec3>& v,
                          const glm::vec3& up) {
  vertices_.clear();
  vertices_.reserve(2 * v.size());
  if (v.size() < 2)
    return;

  for (size_t i = 0; i < v.size() - 1; ++i) {
    glm::vec3 gl_p_world_a = v[i];
    glm::vec3 gl_p_world_b = v[i + 1];
    glm::vec3 dir = glm::normalize(gl_p_world_b - gl_p_world_a);
    glm::vec3 left = glm::cross(up, dir);
    glm::normalize(left);

    vertices_.push_back(gl_p_world_a + (band_width_ / 2.0f * left));
    vertices_.push_back(gl_p_world_a - (band_width_ / 2.0f * left));

    // Cap the end of the path.
    if (i == v.size() - 2) {
      vertices_.push_back(gl_p_world_b + (band_width_ / 2.0f * left));
      vertices_.push_back(gl_p_world_b - (band_width_ / 2.0f * left));
    }
  }

}

void Band::ClearVertexArray() { vertices_.clear(); }

void Band::Render(const glm::mat4& projection_mat,
                  const glm::mat4& view_mat) const {
  glUseProgram(shader_program_);
  glm::mat4 model_mat = GetTransformationMatrix();
  glm::mat4 mvp_mat = projection_mat * view_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  glUniform4f(uniform_color_, band_color_[0], band_color_[1], band_color_[2],
              alpha_);

  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE,
                        sizeof(glm::vec3), &vertices_[0]);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices_.size());
  glDisableVertexAttribArray(attrib_vertices_);
  glUseProgram(0);
}
