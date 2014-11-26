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

#ifndef TANGO_GL_RENDERER_BAND_H
#define TANGO_GL_RENDERER_BAND_H

#include <stdlib.h>
#include <vector>

#include "tango-gl-renderer/drawable_object.h"
#include "tango-gl-renderer/gl_util.h"

class Band : public DrawableObject {
 public:
  Band(const unsigned int max_legnth);
  Band(const Band& other) = delete;
  Band& operator=(const Band&) = delete;
  ~Band();

  void SetColor(const float color[4]);
  void SetAlpha(const float alpha);
  void SetWidth(const float width);
  void UpdateVertexArray(const glm::mat4 m);
  void SetVertexArray(const std::vector<glm::vec3>& v, const glm::vec3& up);
  void ClearVertexArray();
  void Render(const glm::mat4& projection_mat, const glm::mat4& view_mat) const;
  std::vector<glm::vec3> vertices_;

 private:
  float band_color_[4];
  float band_width_;
  float alpha_;
  unsigned int max_length_;

  GLuint shader_program_;
  GLuint attrib_vertices_;
  GLuint uniform_mvp_mat_;
  GLuint uniform_color_;
};

#endif  // TANGO_GL_RENDERER_BAND_H
