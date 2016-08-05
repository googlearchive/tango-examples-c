/*
 * Copyright 2016 Google Inc. All Rights Reserved.
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

#ifndef TANGO_GL_TANGO_GL_H_
#define TANGO_GL_TANGO_GL_H_

#include <vector>
#include <unordered_map>
#include <GLES2/gl2.h>

#include <tango-gl/texture.h>

#include "glm/glm.hpp"

namespace tango_gl {

class Camera;
class Transform;

// A simple mesh that does not animate at runtime.
class StaticMesh {
 public:
  // The rendering style for the mesh, e.g. TRIANGLES, LINES, etc.
  GLenum render_mode;

  // Vertices for the mesh.  Required.
  std::vector<glm::vec3> vertices;

  // Per-vertex normal vectors for the mesh.  If filled in, this must
  // be the same length as vertices.
  std::vector<glm::vec3> normals;

  // Per-vertex color for the mesh, stored as ABGR.  If filled in,
  // this must be the same length as vertices.
  //
  // Red              = 0xFF0000FF
  // Green            = 0xFF00FF00
  // Blue             = 0xFFFF0000
  // White, 50% alpha = 0x80FFFFFF
  std::vector<uint32_t> colors;

  // Indices for the mesh. Required.
  std::vector<uint32_t> indices;

  // UV coords for texture
  std::vector<glm::vec2> uv;
};

// Describes how to draw a mesh.
//
// It is expected that SetShader and SetParam will be called a bunch
// of times when one of these is first created and then rarely at
// runtime.
class Material {
 public:
  // Create a new material using the fallback shader.
  Material();

  // Destroy a material, destroying the underlying shader as well.
  ~Material();

  Material(const Material&) = delete;
  void operator=(const Material&) = delete;

  // Set a new shader for this material.  Returns if the shader
  // properly compiled.
  bool SetShader(const char* vertex_shader, const char* pixel_shader);

  // Set a float parameter for this material.  Returns if the
  // parameter was found and set.
  bool SetParam(const char* uniform_name, float val);

  // Set a vector parameter for this material.  Returns if the
  // parameter was found and set.
  bool SetParam(const char* uniform_name, const glm::vec4& vals);

  // Set a texture parameter for this material.
  bool SetParam(const char* uniform_name, Texture* texture);

  // Bind all parameters for this material to GL state.
  void BindParams() const;

  // Get the underlying GL shader program for this material.
  GLuint GetShaderProgram() const { return shader_program_; }

  // Get the shader program's vertex attribute index.
  GLint GetAttribVertices() const { return attrib_vertices_; }

  // Get the shader program's normal attribute index.
  GLint GetAttribNormals() const { return attrib_normals_; }

  // Get the shader program's color attribute index.
  GLint GetAttribColors() const { return attrib_colors_; }

  // Get the UVs coords attribute of the texture
  GLint GetAttribUVs() const { return attrib_uv_; }

  // Get the shader program's Model-View-Projection matrix uniform index.
  GLint GetUniformModelViewProjMatrix() const { return uniform_mvp_mat_; }

  // Get the shader program's Model-View matrix uniform index.
  GLint GetUniformModelViewMatrix() const { return uniform_mv_mat_; }

  // Get the shader program's Model matrix uniform index.
  GLint GetUniformModelMatrix() const { return uniform_m_mat_; }

  // Get the shader program's Normal matrix uniform index.
  GLint GetUniformNormalMatrix() const { return uniform_normal_mat_; }

 private:
  // Set the shader for this material to the fallback shader.  This
  // will never fail.
  void SetFallbackShader();

  // Set the shader for this material to the specified GL shader
  // program.  This will fail if required attributes and uniforms can
  // not be found.
  bool SetShaderInternal(GLuint program);

  // The fallback shader program.  Never cleaned up.
  static GLuint fallback_shader_program_;

  // Current shader program.
  GLuint shader_program_;

  // Current shader program's vertex position attribute index.
  GLint attrib_vertices_;

  // Current shader program's vertex normal attribute index.
  GLint attrib_normals_;

  // Current shader program's vertex color attribute index.
  GLint attrib_colors_;

  // Current shader program's texture coords attribute index.
  GLint attrib_uv_;

  // Current shader program's Model-View-Projection matrix uniform
  // index.
  GLint uniform_mvp_mat_;

  // Current shader program's Model-View matrix uniform index.
  GLint uniform_mv_mat_;

  // Current shader program's Model matrix uniform index.
  GLint uniform_m_mat_;

  // Current shader program's Normal matrix uniform index.
  GLint uniform_normal_mat_;

  // A hash table of float parameters.
  std::unordered_map<GLint, float> params_float_;

  // A hash table of vec4 parameters.
  std::unordered_map<GLint, glm::vec4> params_vec4_;

  // A hash table of Texture pointers parameters.
  std::unordered_map<GLint, Texture*> params_texture_;
};

// Draw a thing to the screen.
void Render(const StaticMesh& mesh, const Material& material,
            const Transform& transform, const Camera& camera);

}  // namespace tango_gl
#endif  // TANGO_GL_TANGO_GL_H_
