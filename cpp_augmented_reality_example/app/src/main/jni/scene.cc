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

#include <tango-gl/conversions.h>

#include "tango-augmented-reality/scene.h"

namespace {
// We want to represent the device properly with respect to the ground so we'll
// add an offset in z to our origin. We'll set this offset to 1.3 meters based
// on the average height of a human standing with a Tango device. This allows us
// to place a grid roughly on the ground for most users.
const glm::vec3 kHeightOffset = glm::vec3(0.0f, 0.0f, 0.0f);

// Color of the motion tracking trajectory.
const tango_gl::Color kTraceColor(0.22f, 0.28f, 0.67f);

// Color of the ground grid.
const tango_gl::Color kGridColor(0.85f, 0.85f, 0.85f);

// Frustum scale.
const glm::vec3 kFrustumScale = glm::vec3(0.4f, 0.3f, 0.5f);

// Some property for the AR marker.
const glm::quat kMarkerRotation = glm::quat(0.0f, 0.0f, 1.0f, 0.0f);
// The reason we put mark at 0.85f at Y is because the center of the marker
// object is not at the tip of the mark.
const glm::vec3 kMarkerPosition = glm::vec3(0.0f, 0.85f, -3.0f);
const glm::vec3 kMarkerScale = glm::vec3(0.05f, 0.05f, 0.05f);
const tango_gl::Color kMarkerColor(1.0f, 0.f, 0.f);
}  // namespace

namespace tango_augmented_reality {

Scene::Scene() {}

Scene::~Scene() {}

void Scene::InitGLContent() {
  // Allocating render camera and drawable object.
  // All of these objects are for visualization purposes.
  video_overlay_ = new tango_gl::VideoOverlay();
  gesture_camera_ = new tango_gl::GestureCamera();
  axis_ = new tango_gl::Axis();
  frustum_ = new tango_gl::Frustum();
  trace_ = new tango_gl::Trace();
  grid_ = new tango_gl::Grid();
  marker_ = new tango_gl::GoalMarker();

  trace_->SetColor(kTraceColor);
  grid_->SetColor(kGridColor);
  grid_->SetPosition(-kHeightOffset);

  marker_->SetPosition(kMarkerPosition);
  marker_->SetScale(kMarkerScale);
  marker_->SetRotation(kMarkerRotation);
  marker_->SetColor(kMarkerColor);

  gesture_camera_->SetCameraType(
      tango_gl::GestureCamera::CameraType::kThirdPerson);
}

void Scene::DeleteResources() {
  delete gesture_camera_;
  delete video_overlay_;
  delete axis_;
  delete frustum_;
  delete trace_;
  delete grid_;
  delete marker_;
}

void Scene::SetupViewPort(int x, int y, int w, int h) {
  if (h == 0) {
    LOGE("Setup graphic height not valid");
  }
  gesture_camera_->SetAspectRatio(static_cast<float>(w) /
                                  static_cast<float>(h));
  glViewport(x, y, w, h);
}

void Scene::Render(const glm::mat4& cur_pose_transformation) {
  glEnable(GL_DEPTH_TEST);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glm::vec3 position =
      glm::vec3(cur_pose_transformation[3][0], cur_pose_transformation[3][1],
                cur_pose_transformation[3][2]);

  trace_->UpdateVertexArray(position);

  if (gesture_camera_->GetCameraType() ==
      tango_gl::GestureCamera::CameraType::kFirstPerson) {
    // In first person mode, we directly control camera's motion.
    gesture_camera_->SetTransformationMatrix(cur_pose_transformation);

    // If it's first person view, we will render the video overlay in full
    // screen, so we passed identity matrix as view and projection matrix.
    glDisable(GL_DEPTH_TEST);
    video_overlay_->Render(glm::mat4(1.0f), glm::mat4(1.0f));
  } else {
    // In third person or top down more, we follow the camera movement.
    gesture_camera_->SetAnchorPosition(position);

    frustum_->SetTransformationMatrix(cur_pose_transformation);
    // Set the frustum scale to 4:3, this doesn't necessarily match the physical
    // camera's aspect ratio, this is just for visualization purposes.
    frustum_->SetScale(
        glm::vec3(1.0f, camera_image_plane_ratio_, image_plane_distance_));
    frustum_->Render(ar_camera_projection_matrix_,
                     gesture_camera_->GetViewMatrix());

    axis_->SetTransformationMatrix(cur_pose_transformation);
    axis_->Render(ar_camera_projection_matrix_,
                  gesture_camera_->GetViewMatrix());

    trace_->Render(ar_camera_projection_matrix_,
                   gesture_camera_->GetViewMatrix());
    video_overlay_->Render(ar_camera_projection_matrix_,
                           gesture_camera_->GetViewMatrix());
  }
  glEnable(GL_DEPTH_TEST);
  grid_->Render(ar_camera_projection_matrix_, gesture_camera_->GetViewMatrix());
  marker_->Render(ar_camera_projection_matrix_,
                  gesture_camera_->GetViewMatrix());
}

void Scene::SetCameraType(tango_gl::GestureCamera::CameraType camera_type) {
  gesture_camera_->SetCameraType(camera_type);
  if (camera_type == tango_gl::GestureCamera::CameraType::kFirstPerson) {
    video_overlay_->SetParent(nullptr);
    video_overlay_->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
    video_overlay_->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    video_overlay_->SetRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
  } else {
    video_overlay_->SetScale(glm::vec3(1.0f, camera_image_plane_ratio_, 1.0f));
    video_overlay_->SetRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
    video_overlay_->SetPosition(glm::vec3(0.0f, 0.0f, -image_plane_distance_));
    video_overlay_->SetParent(axis_);
  }
}

void Scene::OnTouchEvent(int touch_count,
                         tango_gl::GestureCamera::TouchEvent event, float x0,
                         float y0, float x1, float y1) {
  gesture_camera_->OnTouchEvent(touch_count, event, x0, y0, x1, y1);
}

}  // namespace tango_augmented_reality
