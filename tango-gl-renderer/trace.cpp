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

#include "trace.h"

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

static const int kMaxTraceLength = 5000;
static const float kDistanceCheck = 0.05f;

Trace::Trace() {
  shader_program_ = GlUtil::CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program_) {
    LOGE("Could not create program.");
  }

  uniform_mvp_mat_ = glGetUniformLocation(shader_program_, "mvp_matrix");
  attrib_vertices_ = glGetAttribLocation(shader_program_, "vertex");
  uniform_color_ = glGetUniformLocation(shader_program_, "color");
  vertices_.reserve(kMaxTraceLength);
  vertices_count_ = 0;
}

void Trace::SetTraceColor(const float color[4]) {
  memcpy(trace_color_, color, 4 * sizeof(float));
}

void Trace::UpdateVertexArray(const glm::vec3 v) {
  float dist = glm::distance(vertices_[vertices_count_-1], v);
  if (dist < kDistanceCheck)
    return;
  vertices_.push_back(v);
  ++vertices_count_;
}

void Trace::ClearVertexArray() { vertices_.clear(); }

void Trace::Render(const glm::mat4 projection_mat, const glm::mat4 view_mat) {
  glUseProgram(shader_program_);
  glLineWidth(3.0f);
  glm::mat4 model_mat = GetTransformationMatrix();
  glm::mat4 mvp_mat = projection_mat * view_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  glUniform4f(uniform_color_, trace_color_[0], trace_color_[1], trace_color_[2],
              trace_color_[3]);

  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE,
                        sizeof(glm::vec3), &vertices_[0]);

  glDrawArrays(GL_LINE_STRIP, 0, vertices_.size());
  glLineWidth(1.0f);
  glUseProgram(0);
}

Trace::~Trace() { glDeleteShader(shader_program_); }
