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
#include <tango_support.h>

#include <tango-gl/conversions.h>
#include <tango-gl/util.h>
#include <tango-gl/tango-gl.h>
#include <tango-gl/shaders.h>
#include <tango-gl/texture.h>
#include <tango-gl/meshes.h>

#include "tango-motion-tracking/scene.h"

namespace {
// We want to represent the device properly with respect to the ground so we'll
// add an offset in z to our origin. We'll set this offset to 1.3 meters based
// on the average height of a human standing with a Tango device. This allows us
// to place a grid roughly on the ground for most users.
const glm::vec3 kHeightOffset = glm::vec3(0.0f, 1.3f, 0.0f);

// Sky-like background color
const glm::vec3 kSkyBackgroundColor =
    glm::vec3(128.0f / 255.0f, 218.0f / 255.0f, 235.0f / 255.0f);
}  // namespace

namespace tango_motion_tracking {

Scene::Scene() {}

Scene::~Scene() {}

void Scene::InitGLContent(AAssetManager* aasset_manager) {
  // Allocating render camera and drawable object.
  // All of these objects are for visualization purposes.
  camera_ = new tango_gl::Camera();

  // Floor: Mesh, Material and Texture
  grass_texture_ = new tango_gl::Texture(aasset_manager, "grass.png");
  floor_mesh_ = tango_gl::meshes::MakePlaneMesh(2.0f, 2.0f, 50.0f);
  floor_material_ = new tango_gl::Material();
  floor_material_->SetShader(
      tango_gl::shaders::GetTexturedVertexShader().c_str(),
      tango_gl::shaders::GetTexturedFragmentShader().c_str());
  floor_material_->SetParam("texture", grass_texture_);

  floor_transform_.SetRotation(glm::quat(sqrt(0.5), -sqrt(0.5), 0.0f, 0.0f));
  floor_transform_.SetScale(glm::vec3(50.0f, 50.0f, 1.0f));

  // Cube: Mesh, Material and Texture
  cube_mesh_ = tango_gl::meshes::MakeCubeMesh(1.0f);
  logo_texture_ = new tango_gl::Texture(aasset_manager, "tango_logo.png");
  cube_material_ = new tango_gl::Material();
  cube_material_->SetShader(
      tango_gl::shaders::GetTexturedVertexShader().c_str(),
      tango_gl::shaders::GetTexturedFragmentShader().c_str());
  cube_material_->SetParam("texture", logo_texture_);

  cube_transform_.SetPosition(glm::vec3(0.0f, 1.3f, -5.0f));
}

void Scene::DeleteResources() {
  delete camera_;
  camera_ = nullptr;
  delete cube_mesh_;
  cube_mesh_ = nullptr;
  delete cube_material_;
  cube_material_ = nullptr;
  delete floor_mesh_;
  floor_mesh_ = nullptr;
  delete floor_material_;
  floor_material_ = nullptr;
  delete logo_texture_;
  logo_texture_ = nullptr;
  delete grass_texture_;
  grass_texture_ = nullptr;
}

void Scene::SetupViewPort(int w, int h) {
  if (h == 0) {
    LOGE("Setup graphic height not valid");
  }
  camera_->SetAspectRatio(static_cast<float>(w) / static_cast<float>(h));
  glViewport(0, 0, w, h);
}

void Scene::Render(const TangoPoseData& pose) {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glClearColor(kSkyBackgroundColor.x, kSkyBackgroundColor.y,
               kSkyBackgroundColor.z, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glm::vec3 position =
      glm::vec3(pose.translation[0], pose.translation[1], pose.translation[2]);

  glm::quat orientation = glm::quat(pose.orientation[3], pose.orientation[0],
                                    pose.orientation[1], pose.orientation[2]);

  camera_->SetPosition(position + kHeightOffset);
  camera_->SetRotation(orientation);

  tango_gl::Render(*cube_mesh_, *cube_material_, cube_transform_, *camera_);
  tango_gl::Render(*floor_mesh_, *floor_material_, floor_transform_, *camera_);
}

void Scene::RotateCubeByPose(const TangoPoseData& pose) {
  if (last_pose_timestamp_ > 0) {
    // Calculate time difference in seconds
    double delta_time = pose.timestamp - last_pose_timestamp_;
    // Calculate the corresponding angle movement considering
    // a total rotation time of 6 seconds
    double delta_angle = delta_time * 2 * M_PI / 6;
    // Add this angle to the last known angle
    double angle = last_angle_ + delta_angle;
    last_angle_ = angle;

    double w = cos(angle / 2);
    double y = sin(angle / 2);

    cube_transform_.SetRotation(glm::quat(w, 0.0f, y, 0.0f));
  }
  last_pose_timestamp_ = pose.timestamp;
}

}  // namespace tango_motion_tracking
