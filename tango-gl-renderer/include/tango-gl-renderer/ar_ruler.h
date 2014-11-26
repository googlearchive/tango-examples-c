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

#ifndef TANGO_GL_RENDERER_AR_RULER_H
#define TANGO_GL_RENDERER_AR_RULER_H

#include "tango-gl-renderer/drawable_object.h"
#include "tango-gl-renderer/gl_util.h"

class ArRuler : public DrawableObject {
 public:
  ArRuler();
  ArRuler(const ArRuler& other) = delete;
  ArRuler& operator=(const ArRuler&) = delete;
  ~ArRuler();
  void SetColor(const float color[4]);
  void SetAlpha(const float alpha);
  void Render(const glm::mat4& projection_mat, const glm::mat4& view_mat) const;

 private:
  float color_[4];
  float alpha_;
  GLuint vertex_buffer_;
  GLuint color_buffer_;

  GLuint shader_program_;
  GLuint attrib_vertices_;
  GLuint uniform_color_;
  GLuint uniform_mvp_mat_;
};

#endif  // TANGO_GL_RENDERER_AR_RULER_H
