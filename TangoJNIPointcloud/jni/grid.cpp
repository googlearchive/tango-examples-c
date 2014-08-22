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

#include "grid.h"

static const char kVertexShader[] = "attribute vec4 vertex;\n"
    "uniform mat4 mvp;\n"
    "void main() {\n"
    "  gl_Position = mvp*vertex;\n"
    "}\n";

static const char kFragmentShader[] = "void main() {\n"
    "  // Gray color."
    "  gl_FragColor = vec4(0.58f, 0.58f, 0.58f, 1.0f);\n"
    "}\n";

Grid::Grid(float density, int quantity) {
  // Distance between two lines is 0.2f.
  density_ = density;
  
  // 100 horizontal lines and 100 vertical lines.
  quantity_ = quantity;

  shader_program_ = GlUtil::CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program_) {
    LOGE("Could not create program.");
  }
  uniform_mvp_mat_ = glGetUniformLocation(shader_program_, "mvp");
  attrib_vertices_ = glGetAttribLocation(shader_program_, "vertex");

  glGenBuffers(1, &vertex_buffer_);

  int counter = 0;
  
  // 3 float in 1 vertex, 2 vertices form a line.
  // Horizontal line and vertical line forms the grid.
  traverse_len_ = ((quantity_ * 3) * 2) * 2;
  vertices_ = new float[traverse_len_];
  float width = density_ * quantity_ / 2;

  // Horizontal line.
  for (int i = 0; i < traverse_len_ / 2; i += 6) {
    vertices_[i] = -width;
    vertices_[i + 1] = 0.0f;
    vertices_[i + 2] = -width + counter * density_;

    vertices_[i + 3] = width;
    vertices_[i + 4] = 0.0f;
    vertices_[i + 5] = -width + counter * density_;

    ++counter;
  }

  // Vertical line.
  counter = 0;
  for (int i = traverse_len_ / 2; i < traverse_len_; i += 6) {
    vertices_[i] = -width + counter * density_;
    vertices_[i + 1] = 0.0f;
    vertices_[i + 2] = -width;

    vertices_[i + 3] = -width + counter * density_;
    vertices_[i + 4] = 0.0f;
    vertices_[i + 5] = width;

    ++counter;
  }
}

void Grid::Render(glm::mat4 view_projection_mat) {
  glUseProgram(shader_program_);

  // Calculate model view projection matrix.
  glm::mat4 model_mat = GetCurrentModelMatrix();
  glm::mat4 mvp_mat = view_projection_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  // Binding vertex buffer.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * traverse_len_, vertices_,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDrawArrays(GL_LINES, 0, traverse_len_);
  glUseProgram(0);
  GlUtil::CheckGlError("glUseProgram()");
}

Grid::~Grid() {
  delete[] vertices_;
}
