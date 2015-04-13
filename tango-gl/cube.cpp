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

#include "tango-gl/cube.h"

namespace tango_gl {

static const GLfloat const_vertices[] = {
    -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,  -1.0f};

static const GLushort const_indices[] = {0, 1, 2, 2, 3, 0, 3, 2, 6, 6, 7, 3,
                                         7, 6, 5, 5, 4, 7, 4, 0, 3, 3, 7, 4,
                                         0, 4, 5, 5, 1, 0, 1, 5, 6, 6, 2, 1};

Cube::Cube() {
  SetShader();
  std::vector<GLfloat> vertices(
      const_vertices,
      const_vertices + sizeof(const_vertices) / sizeof(GLfloat));
  std::vector<GLushort> indices(
      const_indices, const_indices + sizeof(const_indices) / sizeof(GLushort));
  SetVertices(vertices, indices);
}
}  // namespace tango_gl
