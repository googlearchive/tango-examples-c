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

#include "tango-gl/conversions.h"

#include "rgb-depth-sync/scene.h"

namespace rgb_depth_sync {

// Camera vertical offset value.
static const glm::vec3 kHeightOffset = glm::vec3(0.0f, 1.3f, 0.0f);

// Min/max clamp value of camera observation distance.
static const float kCamViewMinDist = 1.0f;
static const float kCamViewMaxDist = 100.f;

// Scale frustum size for closer near clipping plane.
static const float kFovScaler = 0.1f;

void Scene::SetCameraIntrinsics(const TangoCameraIntrinsics& cc_intrinsics) {
  const float image_width = cc_intrinsics.width;
  const float image_height = cc_intrinsics.height;
  image_plane_ratio_ = image_height / image_width;
}

Scene::Scene() {
  OW_T_W_ = tango_gl::conversions::opengl_world_T_tango_world();
  CC_T_OC_ = tango_gl::conversions::color_camera_T_opengl_camera();

  camera_texture_drawable_.SetBlendAlpha(0.0f);
}

Scene::~Scene() {}

void Scene::SetupViewPort(int screen_width, int screen_height) {
  if (screen_height == 0 || screen_width == 0) {
    LOGE("The Scene received an invalid height of 0 in SetupViewPort.");
    return;
  }

  float render_width = screen_height / image_plane_ratio_;
  if (render_width > screen_width) {
    // The render screen has a wider aspect ratio than the camera image and
    // applies padding on the left and right.
    viewport_x_ = (screen_width - render_width) / 2;
    viewport_y_ = 0;
    viewport_width_ = render_width;
    viewport_height_ = screen_height;
  } else {
    // The render screen has a narrower aspect ratio than the camera image and
    // applies padding on the top and bottom.
    float render_height = screen_width * image_plane_ratio_;
    viewport_x_ = 0;
    viewport_y_ = (screen_height - render_height) / 2;
    viewport_width_ = screen_width;
    viewport_height_ = render_height;
  }
}

void Scene::ClearRender() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

// We'll render the scene from a pose.
void Scene::Render(GLuint color_texture, GLuint depth_texture) {
  if (color_texture == 0 || depth_texture == 0) {
    return;
  }

  glViewport(viewport_x_, viewport_y_, viewport_width_, viewport_height_);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  camera_texture_drawable_.SetColorTextureId(color_texture);
  camera_texture_drawable_.SetDepthTextureId(depth_texture);
  camera_texture_drawable_.RenderImage();
}

void Scene::InitializeGL() { camera_texture_drawable_.InitializeGL(); }

void Scene::SetDepthAlphaValue(float alpha) {
  camera_texture_drawable_.SetBlendAlpha(alpha);
}

}  // namespace rgb_depth_sync
