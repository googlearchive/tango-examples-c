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

#include "tango-area-learning/scene.h"

namespace {
// We want to represent the device properly with respect to the ground so we'll
// add an offset in z to our origin. We'll set this offset to 1.3 meters based
// on the average height of a human standing with a Tango device. This allows us
// to place a grid roughly on the ground for most users.
const glm::vec3 kHeightOffset = glm::vec3(0.0f, 1.3f, 0.0f);

// Color of the ground grid.
const tango_gl::Color kGridColor(0.85f, 0.85f, 0.85f);

// Frustum scale.
const glm::vec3 kFrustumScale = glm::vec3(0.4f, 0.3f, 0.5f);

// Color of the motion tracking trajectory.
const tango_gl::Color kMotionTrackingTraceColor(0.22f, 0.28f, 0.67f);

// Color of the motion tracking trajectory.
const tango_gl::Color KAdfTraceColor(0.39f, 0.56f, 0.03f);
}  // namespace

namespace tango_area_learning {

Scene::Scene() {}

Scene::~Scene() {}

void Scene::InitGLContent() {
  // Allocating render camera and drawable object.
  // All of these objects are for visualization purposes.
  gesture_camera_ = new tango_gl::GestureCamera();
  axis_ = new tango_gl::Axis();
  frustum_ = new tango_gl::Frustum();
  motion_tracking_trace_ = new tango_gl::Trace();
  adf_trace_ = new tango_gl::Trace();
  grid_ = new tango_gl::Grid();

  // Set the frustum scale to 4:3, this doesn't necessarily match the physical
  // camera's aspect ratio, this is just for visualization purposes.
  frustum_->SetScale(kFrustumScale);

  motion_tracking_trace_->SetColor(kMotionTrackingTraceColor);
  adf_trace_->SetColor(KAdfTraceColor);

  grid_->SetColor(kGridColor);
  gesture_camera_->SetCameraType(
      tango_gl::GestureCamera::CameraType::kThirdPerson);
}

void Scene::DeleteResources() {
  delete gesture_camera_;
  delete axis_;
  delete frustum_;
  delete motion_tracking_trace_;
  delete adf_trace_;
  delete grid_;
}

void Scene::SetupViewPort(int w, int h) {
  if (h == 0) {
    LOGE("Setup graphic height not valid");
  }
  gesture_camera_->SetAspectRatio(static_cast<float>(w) /
                                  static_cast<float>(h));
  glViewport(0, 0, w, h);
}

void Scene::Render(const TangoPoseData& cur_pose, bool is_relocalized) {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  // Convert pose data to vec3 for position and quaternion for orientation.
  // Note that the pose data we received here is in the Tango device frame with
  // respect to the Tango start service frame.
  //
  // More information about frame transformation can be found here:
  // Frame of reference:
  //   https://developers.google.com/project-tango/overview/frames-of-reference
  // Coordinate System Conventions:
  //   https://developers.google.com/project-tango/overview/coordinate-systems
  glm::vec3 tango_position =
      glm::vec3(cur_pose.translation[0], cur_pose.translation[1],
                cur_pose.translation[2]);
  glm::quat tango_rotation =
      glm::quat(cur_pose.orientation[3], cur_pose.orientation[0],
                cur_pose.orientation[1], cur_pose.orientation[2]);

  // tango_gl::conversions provides some handy utility functions for the
  // conversion between different frames.
  // We are converting everything into OpenGL frame.
  glm::vec3 position = tango_gl::conversions::Vec3TangoToGl(tango_position);
  glm::quat rotation = tango_gl::conversions::QuatTangoToGl(tango_rotation);

  position += kHeightOffset;

  if (gesture_camera_->GetCameraType() ==
      tango_gl::GestureCamera::CameraType::kFirstPerson) {
    // In first person mode, we directly control camera's motion.
    gesture_camera_->SetPosition(position);
    gesture_camera_->SetRotation(rotation);
  } else {
    // In third person or top down more, we follow the camera movement.
    gesture_camera_->SetAnchorPosition(position);

    frustum_->SetPosition(position);
    frustum_->SetRotation(rotation);
    frustum_->Render(gesture_camera_->GetProjectionMatrix(),
                     gesture_camera_->GetViewMatrix());

    axis_->SetPosition(position);
    axis_->SetRotation(rotation);
    axis_->Render(gesture_camera_->GetProjectionMatrix(),
                  gesture_camera_->GetViewMatrix());
  }

  if (is_relocalized) {
    adf_trace_->UpdateVertexArray(position);
  } else {
    motion_tracking_trace_->UpdateVertexArray(position);
  }

  adf_trace_->Render(gesture_camera_->GetProjectionMatrix(),
                     gesture_camera_->GetViewMatrix());
  motion_tracking_trace_->Render(gesture_camera_->GetProjectionMatrix(),
                                 gesture_camera_->GetViewMatrix());
  grid_->Render(gesture_camera_->GetProjectionMatrix(),
                gesture_camera_->GetViewMatrix());
}

void Scene::SetCameraType(tango_gl::GestureCamera::CameraType camera_type) {
  gesture_camera_->SetCameraType(camera_type);
}

void Scene::OnTouchEvent(int touch_count,
                         tango_gl::GestureCamera::TouchEvent event, float x0,
                         float y0, float x1, float y1) {
  gesture_camera_->OnTouchEvent(touch_count, event, x0, y0, x1, y1);
}

}  // namespace tango_area_learning