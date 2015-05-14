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

#include "tango-gl/band.h"
#include "tango-gl/util.h"

namespace tango_gl {

// Set band resolution to 0.01m(1cm) when using UpdateVertexArray()
static const float kMinDistanceSquared = 0.0001f;

Band::Band(const unsigned int max_length)
    : band_width_(0.2), max_length_(max_length) {
  SetShader();
  vertices_v_.reserve(max_length);
}

void Band::SetWidth(const float width) {
  band_width_ = width;
}

void Band::UpdateVertexArray(const glm::mat4 m) {
  bool need_to_initialize = (vertices_v_.size() < 2);

  bool sufficient_delta = false;
  if (!need_to_initialize) {
    glm::vec3 band_front = 0.5f * (vertices_v_[vertices_v_.size() - 1] +
                                   vertices_v_[vertices_v_.size() - 2]);
    sufficient_delta = kMinDistanceSquared <
        util::DistanceSquared(band_front, util::GetTranslationFromMatrix(m));
  }

  if (need_to_initialize || sufficient_delta) {
    glm::mat4 left  = glm::mat4(1.0f);
    glm::mat4 right = glm::mat4(1.0f);
    left[3][0]  = -band_width_ / 2.0f;
    right[3][0] =  band_width_ / 2.0f;
    left  = m * left;
    right = m * right;

    vertices_v_.push_back(util::GetTranslationFromMatrix(left));
    vertices_v_.push_back(util::GetTranslationFromMatrix(right));
    if (vertices_v_.size() > max_length_) {
      vertices_v_.erase(vertices_v_.begin(), vertices_v_.begin() + 2);
    }
  }
}

void Band::SetVertexArray(const std::vector<glm::vec3>& v,
                          const glm::vec3& up) {
  vertices_v_.clear();
  vertices_v_.reserve(2 * v.size());
  if (v.size() < 2)
    return;

  for (size_t i = 0; i < v.size() - 1; ++i) {
    glm::vec3 gl_p_world_a = v[i];
    glm::vec3 gl_p_world_b = v[i + 1];
    glm::vec3 dir = glm::normalize(gl_p_world_b - gl_p_world_a);
    glm::vec3 left = glm::cross(up, dir);
    glm::normalize(left);

    vertices_v_.push_back(gl_p_world_a + (band_width_ / 2.0f * left));
    vertices_v_.push_back(gl_p_world_a - (band_width_ / 2.0f * left));

    // Cap the end of the path.
    if (i == v.size() - 2) {
      vertices_v_.push_back(gl_p_world_b + (band_width_ / 2.0f * left));
      vertices_v_.push_back(gl_p_world_b - (band_width_ / 2.0f * left));
    }
  }

}

void Band::ClearVertexArray() { vertices_v_.clear(); }

void Band::Render(const glm::mat4& projection_mat,
                  const glm::mat4& view_mat) const {
  glUseProgram(shader_program_);
  glm::mat4 model_mat = GetTransformationMatrix();
  glm::mat4 mvp_mat = projection_mat * view_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  glUniform4f(uniform_color_, red_, green_, blue_, alpha_);

  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE,
                        sizeof(glm::vec3), &vertices_v_[0]);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices_v_.size());
  glDisableVertexAttribArray(attrib_vertices_);
  glUseProgram(0);
}

}  // namespace tango_gl
