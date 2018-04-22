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

#ifndef TANGO_GL_DRAWABLE_OBJECT_H_
#define TANGO_GL_DRAWABLE_OBJECT_H_

#include <vector>

#include "tango-gl/color.h"
#include "tango-gl/transform.h"
#include "tango-gl/util.h"

namespace tango_gl {
class DrawableObject : public Transform {
 public:
  DrawableObject() : red_(0), green_(0), blue_(0), alpha_(1.0f) {}
  DrawableObject(const DrawableObject& other) = delete;
  const DrawableObject& operator=(const DrawableObject&) = delete;

  void DeleteGlResources();
  void SetShader();
  void SetColor(const Color& color);
  void SetColor(const float red, const float green, const float blue);
  void SetAlpha(const float alpha);
  void SetVertices(const std::vector<GLfloat>& vertices);
  void SetVertices(const std::vector<GLfloat>& vertices,
                   const std::vector<GLushort>& indices);
  void SetVertices(const std::vector<GLfloat>& vertices,
                   const std::vector<GLfloat>& normals);
  void SetVertices(const std::vector<GLfloat>& vertices,
                   const std::vector<GLfloat>& normals,
                   const std::vector<GLushort>& indices);
  virtual void Render(const glm::mat4& projection_mat,
                      const glm::mat4& view_mat) const = 0;

 protected:
  float red_;
  float green_;
  float blue_;
  float alpha_;
  std::vector<GLushort> indices_;
  std::vector<GLfloat> vertices_;
  std::vector<GLfloat> normals_;

  GLenum render_mode_;
  GLuint shader_program_;
  GLuint uniform_color_;
  GLuint uniform_mvp_mat_;
  GLuint attrib_vertices_;
  GLuint attrib_normals_;
};
}  // namespace tango_gl
#endif  // TANGO_GL_DRAWABLE_OBJECT_H_
