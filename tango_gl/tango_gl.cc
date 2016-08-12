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

#include "tango-gl/tango-gl.h"

#include "tango-gl/camera.h"
#include "tango-gl/transform.h"
#include "tango-gl/util.h"

namespace {

// Fallback vertex shader.  This shader will be used if a valid shader
// program is not set on a material.
const char* kFallbackVS =
    "precision mediump float;\n"
    "precision mediump int;\n"
    "attribute vec4 vertex;\n"
    "\n"
    "uniform mat4 mvp;\n"
    "\n"
    "void main() {\n"
    "  gl_Position = mvp * vertex;\n"
    "}\n";

// Fallback pixel shader.  This shader will be used if a valid shader
// program is not set on a material.
const char* kFallbackPS =
    "precision mediump float;\n"
    "\n"
    "void main() {\n"
    "  gl_FragColor = vec4(1, 0, 1, 1);\n"
    "}\n";
}  // namespace

namespace tango_gl {

GLuint Material::fallback_shader_program_ = 0;

void Render(const StaticMesh& mesh, const Material& material,
            const Transform& transform, const Camera& camera) {
  glm::mat4 model_mat = transform.GetTransformationMatrix();
  glm::mat4 view_mat = camera.GetViewMatrix();
  glm::mat4 projection_mat = camera.GetProjectionMatrix();

  glUseProgram(material.GetShaderProgram());

  // Set up shader uniforms.
  GLint uniform_mvp_mat = material.GetUniformModelViewProjMatrix();
  if (uniform_mvp_mat != -1) {
    glm::mat4 mvp_mat = projection_mat * view_mat * model_mat;
    glUniformMatrix4fv(uniform_mvp_mat, 1, GL_FALSE, glm::value_ptr(mvp_mat));
  }

  GLint uniform_mv_mat = material.GetUniformModelViewMatrix();
  if (uniform_mv_mat != -1) {
    glm::mat4 mv_mat = view_mat * model_mat;
    glUniformMatrix4fv(uniform_mv_mat, 1, GL_FALSE, glm::value_ptr(mv_mat));
  }

  GLint uniform_m_mat = material.GetUniformModelMatrix();
  if (uniform_m_mat != -1) {
    glm::mat4 m_mat = model_mat;
    glUniformMatrix4fv(uniform_m_mat, 1, GL_FALSE, glm::value_ptr(m_mat));
  }

  GLint uniform_normal_mat = material.GetUniformNormalMatrix();
  if (uniform_normal_mat != -1) {
    glm::mat4 normal_mat = glm::transpose(glm::inverse(view_mat * model_mat));
    glUniformMatrix4fv(uniform_normal_mat, 1, GL_FALSE,
                       glm::value_ptr(normal_mat));
  }

  material.BindParams();

  // Set up shader attributes.
  GLint attrib_vertices = material.GetAttribVertices();
  glEnableVertexAttribArray(attrib_vertices);
  glVertexAttribPointer(attrib_vertices, 3, GL_FLOAT, GL_FALSE, 0,
                        mesh.vertices.data());

  GLint attrib_normal = material.GetAttribNormals();
  if (attrib_normal != -1 && !mesh.normals.empty()) {
    glEnableVertexAttribArray(attrib_normal);
    glVertexAttribPointer(attrib_normal, 3, GL_FLOAT, GL_FALSE, 0,
                          mesh.normals.data());
  }

  GLint attrib_color = material.GetAttribColors();
  if (attrib_color != -1 && !mesh.colors.empty()) {
    glEnableVertexAttribArray(attrib_color);
    glVertexAttribPointer(attrib_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0,
                          mesh.colors.data());
  }

  GLint attrib_uv = material.GetAttribUVs();
  if (attrib_uv != -1 && !mesh.uv.empty()) {
    glEnableVertexAttribArray(attrib_uv);
    glVertexAttribPointer(attrib_uv, 2, GL_FLOAT, GL_FALSE, 0, mesh.uv.data());
  }

  glDrawElements(mesh.render_mode, mesh.indices.size(), GL_UNSIGNED_INT,
                 mesh.indices.data());

  // Clean up state
  glDisableVertexAttribArray(attrib_vertices);
  if (attrib_normal != -1) {
    glDisableVertexAttribArray(attrib_normal);
  }
  if (attrib_color != -1) {
    glDisableVertexAttribArray(attrib_color);
  }

  glUseProgram(0);

  util::CheckGlError("Render");
}

Material::Material() {
  SetFallbackShader();
  util::CheckGlError("Material::ctor");
}

Material::~Material() {
  shader_program_ = 0;
  fallback_shader_program_ = 0;
}

bool Material::SetShader(const char* vertex_shader, const char* pixel_shader) {
  params_float_.clear();
  params_vec4_.clear();

  GLuint program = util::CreateProgram(vertex_shader, pixel_shader);
  if (!program) {
    SetFallbackShader();
    return false;
  }

  bool result = SetShaderInternal(program);
  if (!result) {
    SetFallbackShader();
  }
  return result;
}

void Material::SetFallbackShader() {
  params_float_.clear();
  params_vec4_.clear();

  if (!fallback_shader_program_) {
    fallback_shader_program_ = util::CreateProgram(kFallbackVS, kFallbackPS);
    if (!fallback_shader_program_) {
      LOGE("%s -- Critical error shader would not load.", __func__);
      abort();
    }
  }

  bool result = SetShaderInternal(fallback_shader_program_);
  if (!result) {
    LOGE(
        "%s -- Critical error, could not get required uniforms and/or "
        "attributes.",
        __func__);
    abort();
  }
}

bool Material::SetShaderInternal(GLuint program) {
  shader_program_ = program;
  attrib_vertices_ = glGetAttribLocation(shader_program_, "vertex");
  if (attrib_vertices_ == -1) {
    LOGE("Could not get vertex attribute");
    // Positions are required.
    return false;
  }
  attrib_normals_ = glGetAttribLocation(shader_program_, "normal");
  attrib_colors_ = glGetAttribLocation(shader_program_, "color");
  attrib_uv_ = glGetAttribLocation(shader_program_, "uv");

  uniform_mvp_mat_ = glGetUniformLocation(shader_program_, "mvp");
  if (uniform_mvp_mat_ == -1) {
    LOGE("Could not get mvp uniform");
    // The Model-View-Projection matrix is required.
    return false;
  }

  uniform_mv_mat_ = glGetUniformLocation(shader_program_, "mv");
  uniform_m_mat_ = glGetUniformLocation(shader_program_, "m");
  uniform_normal_mat_ = glGetUniformLocation(shader_program_, "normal_mat");
  return true;
}

bool Material::SetParam(const char* uniform_name, float val) {
  if (shader_program_ == fallback_shader_program_) {
    // The fallback shader ignores all parameters to avoid cluttering
    // up the log.
    return true;
  }

  GLint location = glGetUniformLocation(shader_program_, uniform_name);
  if (location == -1) {
    LOGE("%s -- Unable to find parameter %s", __func__, uniform_name);
    return false;
  }

  params_float_[location] = val;
  return true;
}

bool Material::SetParam(const char* uniform_name, const glm::vec4& vals) {
  if (shader_program_ == fallback_shader_program_) {
    // The fallback shader ignores all parameters to avoid cluttering
    // up the log.
    return true;
  }

  GLint location = glGetUniformLocation(shader_program_, uniform_name);
  if (location == -1) {
    LOGE("%s -- Unable to find parameter %s", __func__, uniform_name);
    return false;
  }

  params_vec4_[location] = vals;
  return true;
}

bool Material::SetParam(const char* uniform_name, Texture* texture) {
  if (shader_program_ == fallback_shader_program_) {
    // The fallback shader ignores all parameters to avoid cluttering
    // up the log.
    return true;
  }

  GLint location = glGetUniformLocation(shader_program_, uniform_name);
  if (location == -1) {
    LOGE("%s -- Unable to find parameter %s", __func__, uniform_name);
    return false;
  }

  params_texture_[location] = texture;
  return true;
}

void Material::BindParams() const {
  for (auto& param : params_float_) {
    glUniform1f(param.first, param.second);
  }
  for (auto& param : params_vec4_) {
    glUniform4fv(param.first, 1, glm::value_ptr(param.second));
  }
  int index = 0;
  for (auto& param : params_texture_) {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(param.second->GetTextureTarget(),
                  param.second->GetTextureID());
    glUniform1i(param.first, index);
    index++;
  }
}

}  // namespace tango_gl
