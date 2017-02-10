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

#include <math.h>

#include <tango-gl/conversions.h>
#include <tango-gl/tango-gl.h>
#include <tango-gl/texture.h>
#include <tango-gl/shaders.h>
#include <tango-gl/meshes.h>

#include "tango-augmented-reality/scene.h"

namespace {
// We want to represent the device properly with respect to the ground so we'll
// add an offset in z to our origin. We'll set this offset to 1.3 meters based
// on the average height of a human standing with a Tango device. This allows us
// to place a grid roughly on the ground for most users.
const glm::vec3 kHeightOffset = glm::vec3(0.0f, 0.0f, 0.0f);

// Frustum scale.
const glm::vec3 kFrustumScale = glm::vec3(0.4f, 0.3f, 0.5f);

}  // namespace

namespace tango_augmented_reality {

Scene::Scene() {}

Scene::~Scene() {}

void Scene::InitGLContent(AAssetManager* aasset_manager) {
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // Allocating render camera and drawable object.
  // All of these objects are for visualization purposes.
  video_overlay_ = new tango_gl::VideoOverlay();
  camera_ = new tango_gl::Camera();

  // Init earth mesh and material
  earth_mesh_ = tango_gl::meshes::MakeSphereMesh(20, 20, 1.0f);
  earth_material_ = new tango_gl::Material();
  earth_texture_ = new tango_gl::Texture(aasset_manager, "earth.png");

  earth_material_->SetShader(
      tango_gl::shaders::GetTexturedVertexShader().c_str(),
      tango_gl::shaders::GetTexturedFragmentShader().c_str());
  earth_material_->SetParam("texture", earth_texture_);

  // Init moon mesh and material
  moon_mesh_ = tango_gl::meshes::MakeSphereMesh(10, 10, 0.3f);
  moon_material_ = new tango_gl::Material();
  moon_texture_ = new tango_gl::Texture(aasset_manager, "moon.png");
  moon_material_->SetShader(
      tango_gl::shaders::GetTexturedVertexShader().c_str(),
      tango_gl::shaders::GetTexturedFragmentShader().c_str());
  moon_material_->SetParam("texture", moon_texture_);

  earth_transform_.SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));
  moon_transform_.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

  is_content_initialized_ = true;
}

void Scene::DeleteResources() {
  if (is_content_initialized_) {
    delete camera_;
    camera_ = nullptr;
    delete video_overlay_;
    video_overlay_ = nullptr;
    delete earth_mesh_;
    earth_mesh_ = nullptr;
    delete earth_material_;
    earth_material_ = nullptr;
    delete earth_texture_;
    earth_texture_ = nullptr;
    delete moon_mesh_;
    moon_mesh_ = nullptr;
    delete moon_material_;
    moon_material_ = nullptr;
    delete moon_texture_;
    moon_texture_ = nullptr;

    is_content_initialized_ = false;
  }
}

void Scene::SetupViewport(int w, int h) {
  if (h <= 0 || w <= 0) {
    LOGE("Setup graphic height not valid");
    return;
  }

  viewport_width_ = w;
  viewport_height_ = h;
}

void Scene::SetProjectionMatrix(const glm::mat4& projection_matrix) {
  camera_->SetProjectionMatrix(projection_matrix);
}

void Scene::Clear() {
  glViewport(0, 0, viewport_width_, viewport_height_);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void Scene::Render(const glm::mat4& cur_pose_transformation) {
  glViewport(0, 0, viewport_width_, viewport_height_);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  // In first person mode, we directly control camera's motion.
  camera_->SetTransformationMatrix(cur_pose_transformation);

  // If it's first person view, we will render the video overlay in full
  // screen, so we passed identity matrix as view and projection matrix.
  glDisable(GL_DEPTH_TEST);
  video_overlay_->Render(glm::mat4(1.0f), glm::mat4(1.0f));
  glEnable(GL_DEPTH_TEST);

  tango_gl::Render(*earth_mesh_, *earth_material_, earth_transform_, *camera_);
  tango_gl::Render(*moon_mesh_, *moon_material_, moon_transform_, *camera_);
}

void Scene::RotateEarthForTimestamp(double timestamp) {
  RotateYAxisForTimestamp(timestamp, &earth_transform_, &earth_last_angle_,
                          &earth_last_timestamp_);
}

void Scene::RotateMoonForTimestamp(double timestamp) {
  RotateYAxisForTimestamp(timestamp, &moon_transform_, &moon_last_angle_,
                          &moon_last_timestamp_);
}

void Scene::TranslateMoonForTimestamp(double timestamp) {
  if (moon_last_translation_timestamp_ > 0) {
    // Calculate time difference in seconds
    double delta_time = timestamp - moon_last_translation_timestamp_;
    // Calculate the corresponding angle movement considering
    // a total rotation time of 6 seconds
    double delta_angle = delta_time * 2 * M_PI / 6;
    // Add this angle to the last known angle
    double angle = moon_last_translation_angle_ + delta_angle;
    moon_last_translation_angle_ = angle;

    double x = 2.0f * sin(angle);
    double z = 2.0f * cos(angle);

    moon_transform_.SetPosition(glm::vec3(x, 0.0f, z - 5.0f));
  }
  moon_last_translation_timestamp_ = timestamp;
}

void Scene::RotateYAxisForTimestamp(double timestamp,
                                    tango_gl::Transform* transform,
                                    double* last_angle,
                                    double* last_timestamp) {
  if (*last_timestamp > 0) {
    // Calculate time difference in seconds
    double delta_time = timestamp - *last_timestamp;
    // Calculate the corresponding angle movement considering
    // a total rotation time of 6 seconds
    double delta_angle = delta_time * 2 * M_PI / 6;
    // Add this angle to the last known angle
    double angle = *last_angle + delta_angle;
    *last_angle = angle;

    double w = cos(angle / 2);
    double y = sin(angle / 2);

    transform->SetRotation(glm::quat(w, 0.0f, y, 0.0f));
  }
  *last_timestamp = timestamp;
}

void Scene::SetVideoOverlayRotation(int display_rotation) {
  if (is_content_initialized_) {
    video_overlay_->SetDisplayRotation(
        static_cast<TangoSupportRotation>(display_rotation));
  }
}

}  // namespace tango_augmented_reality
