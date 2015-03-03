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

#ifndef TANGO_GL_POLYGON_H_
#define TANGO_GL_POLYGON_H_

#include <stdlib.h>
#include <vector>
#include <math.h>

#include "tango-gl/drawable_object.h"

namespace tango_gl {
class Polygon : public DrawableObject {
 public:
  Polygon();
  Polygon(const Polygon& other) = delete;
  Polygon& operator=(const Polygon&) = delete;
  ~Polygon();

  void SetAlpha(const float alpha);
  void Render(const glm::mat4& projection_mat, const glm::mat4& view_mat) const;
  void SetMeshData(const std::vector<GLfloat>& vertices,
                   const std::vector<GLushort>& indices);

 private:
  float alpha_;
  std::vector<GLushort> indices_;
  std::vector<GLfloat> vertices_;
  GLuint shader_program_;
  GLuint attrib_vertices_;
  GLuint uniform_color_;
  GLuint uniform_mvp_mat_;
};
}  // namespace tango_gl
#endif  // TANGO_GL_POLYGON_H_
