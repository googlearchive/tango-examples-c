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

#include <tango-gl/conversions.h>
#include <tango-gl/tango-gl.h>
#include <tango_support.h>

#include "mesh_builder/scene.h"

namespace {
const char* kPerVertexColorVS =
    "precision mediump float;\n"
    "precision mediump int;\n"
    "\n"
    "attribute vec4 vertex;\n"
    "attribute vec4 color;\n"
    "\n"
    "uniform mat4 mvp;\n"
    "\n"
    "varying vec4 vs_color;\n"
    "void main() {\n"
    "  gl_Position = mvp * vertex;\n"
    "  vs_color = color;\n"
    "}\n";

const char* kPerVertexColorPS =
    "precision mediump float;\n"
    "\n"
    "varying vec4 vs_color;\n"
    "\n"
    "void main() {\n"
    "  gl_FragColor = vs_color;\n"
    "}\n";
}  // namespace

namespace mesh_builder {

Scene::Scene() {}

Scene::~Scene() {}

void Scene::InitGLContent() {
  // Allocating render camera and drawable object.
  // All of these objects are for visualization purposes.
  camera_ = new tango_gl::Camera();

  // Material used for rendering the dynamic meshes from 3D
  // reconstruction.
  dynamic_mesh_material_ = new tango_gl::Material();
  dynamic_mesh_material_->SetShader(kPerVertexColorVS, kPerVertexColorPS);
}

void Scene::DeleteResources() {
  delete dynamic_mesh_material_;
  dynamic_mesh_material_ = nullptr;
}

void Scene::SetupViewPort(int w, int h) {
  if (h == 0) {
    LOGE("Setup graphic height not valid");
  }
  camera_->SetAspectRatio(static_cast<float>(w) / static_cast<float>(h));
  glViewport(0, 0, w, h);
}

void Scene::Render() {
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  for (const tango_gl::StaticMesh* mesh : dynamic_meshes_) {
    tango_gl::Render(*mesh, *dynamic_mesh_material_, tango_gl::Transform(),
                     *camera_);
  }
}

void Scene::AddDynamicMesh(tango_gl::StaticMesh* mesh) {
  dynamic_meshes_.push_back(mesh);
}

void Scene::ClearDynamicMeshes() { dynamic_meshes_.clear(); }

}  // namespace mesh_builder
