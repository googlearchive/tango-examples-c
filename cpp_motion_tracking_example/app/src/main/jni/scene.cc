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
#include <tango_support_api.h>

#include <tango-gl/conversions.h>
#include <tango-gl/util.h>

#include "tango-motion-tracking/scene.h"

namespace {
// We want to represent the device properly with respect to the ground so we'll
// add an offset in z to our origin. We'll set this offset to 1.3 meters based
// on the average height of a human standing with a Tango device. This allows us
// to place a grid roughly on the ground for most users.
const glm::vec3 kHeightOffset = glm::vec3(0.0f, 1.3f, 0.0f);

// Color of the ground grid.
const tango_gl::Color kGridColor(0.85f, 0.85f, 0.85f);
}  // namespace

namespace tango_motion_tracking {

Scene::Scene() {}

Scene::~Scene() {}

void Scene::InitGLContent() {
  // Allocating render camera and drawable object.
  // All of these objects are for visualization purposes.
  camera_ = new tango_gl::Camera();
  grid_ = new tango_gl::Grid();

  grid_->SetColor(kGridColor);
}

void Scene::DeleteResources() {
  delete camera_;
  delete grid_;
}

void Scene::SetupViewPort(int w, int h) {
  if (h == 0) {
    LOGE("Setup graphic height not valid");
  }
  camera_->SetAspectRatio(static_cast<float>(w) / static_cast<float>(h));
  glViewport(0, 0, w, h);
}

void Scene::Render(const glm::vec3& position, const glm::quat& rotation) {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  camera_->SetPosition(position + kHeightOffset);
  camera_->SetRotation(rotation);

  grid_->Render(camera_->GetProjectionMatrix(), camera_->GetViewMatrix());
}

}  // namespace tango_motion_tracking
