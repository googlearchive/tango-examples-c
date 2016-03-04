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

// Transformation for device rotation on 270 degrees.
const glm::mat4 kRotation270TDefault(0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 1.0f);

// Transformation for device rotation on 180 degrees.
const glm::mat4 kRotation180TDefault(-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 1.0f);

// Transformation for device rotation on 90 degrees.
const glm::mat4 kRotation90TDefault(0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 1.0f);

// Transformation for device rotation on default orientation.
const glm::mat4 kRotation0TDefault(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                   0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                                   0.0f, 1.0f);

// Convert a Tango Pose into glm::mat4.
//
// @param pose: input TangoPoseData.
glm::mat4 GetMatrixFromPose(const TangoPoseData& pose) {
  glm::vec3 translation =
      glm::vec3(pose.translation[0], pose.translation[1], pose.translation[2]);
  glm::quat rotation = glm::quat(pose.orientation[3], pose.orientation[0],
                                 pose.orientation[1], pose.orientation[2]);
  glm::mat4 matrix =
      glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(rotation);
  return matrix;
}

// Convert a Tango Pose into glm::mat4 with the rotation matrix applied.
// The index is following Android screen rotation enum.
// see Android documentation for detail:
//   http://developer.android.com/reference/android/view/Surface.html#ROTATION_0
//
// @param screen_rotation_index: the screen rotation index.
// @param device_pose: input TangoPoseData in device with respect to start
// service.
// @param opengl_position: output position vector in OpenGL coordinate.
// @param opengl_rotation: output rotation quaternion in OpenGL coordinate.
void GetOpenGlPoseWithScreenRotation(int screen_rotation_index,
                                     const TangoPoseData& device_pose,
                                     glm::vec3* opengl_position,
                                     glm::quat* opengl_rotation) {
  const glm::mat4* screen_rotation_mat;
  switch (screen_rotation_index) {
    case 0:
      screen_rotation_mat = &kRotation0TDefault;
      break;
    case 1:
      screen_rotation_mat = &kRotation90TDefault;
      break;
    case 2:
      screen_rotation_mat = &kRotation180TDefault;
      break;
    case 3:
      screen_rotation_mat = &kRotation270TDefault;
      break;
    default:
      screen_rotation_mat = &kRotation0TDefault;
      break;
  }

  glm::vec3 scale = glm::vec3(0.0f, 0.0f, 0.0f);

  glm::mat4 start_service_T_deivce = GetMatrixFromPose(device_pose);

  glm::mat4 opengl_world_T_opengl_camera =
      tango_gl::conversions::opengl_world_T_tango_world() *
      start_service_T_deivce * (*screen_rotation_mat);

  tango_gl::util::DecomposeMatrix(opengl_world_T_opengl_camera,
                                  *opengl_position, *opengl_rotation, scale);
}

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

void Scene::Render(const TangoPoseData& cur_pose, int rotation_index = 0) {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  GetOpenGlPoseWithScreenRotation(rotation_index, cur_pose, &position,
                                  &rotation);

  position += kHeightOffset;

  camera_->SetPosition(position);
  camera_->SetRotation(rotation);

  grid_->Render(camera_->GetProjectionMatrix(), camera_->GetViewMatrix());
}

}  // namespace tango_motion_tracking
