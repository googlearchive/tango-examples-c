/*
 * Copyright 2017 Google Inc. All Rights Reserved.
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

#include "tango-marker-detection/mesh_object.h"

namespace tango_marker_detection {

MeshObject::MeshObject() : initialized_(false), visible_(false) {}

MeshObject::~MeshObject() { Delete(); }

void MeshObject::MakeSphere(AAssetManager* aasset_manager,
                            const char* texture_file, double radius) {
  // Delete if it has been created.
  if (initialized_) {
    Delete();
  }
  // Init earth mesh and material
  mesh_ = tango_gl::meshes::MakeSphereMesh(20, 20, radius);

  material_ = new tango_gl::Material();
  texture_ = new tango_gl::Texture(aasset_manager, texture_file);

  material_->SetShader(tango_gl::shaders::GetTexturedVertexShader().c_str(),
                       tango_gl::shaders::GetTexturedFragmentShader().c_str());
  material_->SetParam("texture", texture_);

  initialized_ = true;
}

void MeshObject::Delete() {
  if (initialized_) {
    delete mesh_;
    mesh_ = nullptr;
    delete material_;
    material_ = nullptr;
    delete texture_;
    texture_ = nullptr;
    initialized_ = false;
  }
}

void MeshObject::Render(tango_gl::Camera* camera) {
  if (initialized_ && visible_) {
    tango_gl::Render(*mesh_, *material_, transform_, *camera);
  }
}

void MeshObject::Transform(const double* translation, const double* rotation) {
  transform_.SetPosition(
      glm::vec3(translation[0], translation[1], translation[2]));
  transform_.SetRotation(
      glm::quat(rotation[0], rotation[1], rotation[2], rotation[3]));
}

void MeshObject::SetVisible(bool visible) { visible_ = visible; }

}  // namespace tango_marker_detection
