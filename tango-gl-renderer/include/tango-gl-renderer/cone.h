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

#ifndef TANGO_GL_RENDERER_CONE_H
#define TANGO_GL_RENDERER_CONE_H

#include <stdlib.h>
#include <vector>
#include <math.h>

#include "tango-gl-renderer/drawable_object.h"
#include "tango-gl-renderer/gl_util.h"

class Cone : public DrawableObject {
 public:
  Cone();
  Cone(const Cone& other) = delete;
  Cone& operator=(const Cone&) = delete;
  ~Cone();

  void SetAlpha(const float alpha);
  void SetVertexArray();
  void Render(const glm::mat4& projection_mat, const glm::mat4& view_mat) const;
  std::vector<glm::vec3> vertices_;
  std::vector<glm::vec4> colors_;

 private:
  float alpha_;
  GLuint shader_program_;
  GLuint attrib_vertices_;
  GLuint attrib_colors_;
  GLuint uniform_mvp_mat_;
};

#endif  // TANGO_GL_RENDERER_CONE_H
