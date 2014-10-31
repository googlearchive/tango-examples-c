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

static const char kFragmentShader[] =
"void main() {\n"
"  gl_FragColor = vec4(0.85f,0.85f,0.85f,1);\n"
"}\n";

// Initialize Grid with x and y grid count,
// qx, quantity in x
// qy, quantity in y.
Grid::Grid(float density, int qx, int qy) {
  density_ = density;
  quantity_x_ = qx;
  quantity_y_ = qy;

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
  traverse_len_ = (((quantity_x_ + quantity_y_ + 2) * 3) * 2);
  vertices_ = new float[traverse_len_];
  float width = density_ * quantity_x_ / 2;
  float height = density_ * quantity_y_ / 2;

  // Horizontal line.
  for (int i = 0; i < (quantity_y_ + 1) * 6; i += 6) {
    vertices_[i] = -width;
    vertices_[i + 1] = 0.0f;
    vertices_[i + 2] = -height + counter * density_;

    vertices_[i + 3] = width;
    vertices_[i + 4] = 0.0f;
    vertices_[i + 5] = -height + counter * density_;

    ++counter;
  }

  // Vertical line.
  counter = 0;
  for (int i = (quantity_y_ + 1) * 6; i < traverse_len_; i += 6) {
    vertices_[i] = -width + counter * density_;
    vertices_[i + 1] = 0.0f;
    vertices_[i + 2] = -height;

    vertices_[i + 3] = -width + counter * density_;
    vertices_[i + 4] = 0.0f;
    vertices_[i + 5] = height;

    ++counter;
  }
}

void Grid::Render(glm::mat4 projection_mat, glm::mat4 view_mat) {
  glUseProgram(shader_program_);

  // Calculate model view projection matrix.
  glm::mat4 model_mat = GetTransformationMatrix();
  glm::mat4 mvp_mat = projection_mat * view_mat * model_mat;
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
  GlUtil::CheckGlError("grid glDrawArray()");
  glUseProgram(0);
  GlUtil::CheckGlError("glUseProgram()");
}

Grid::~Grid() {
  delete[] vertices_;
}

