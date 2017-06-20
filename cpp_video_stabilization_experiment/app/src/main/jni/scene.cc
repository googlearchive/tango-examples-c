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
#include <glm/gtx/quaternion.hpp>
#include <tango-gl/conversions.h>
#include <tango-gl/obj_loader.h>
#include <tango-gl/tango-gl.h>
#include <tango-gl/texture.h>
#include <tango-gl/shaders.h>
#include <tango-gl/meshes.h>

#include "tango-video-stabilization/scene.h"

namespace {
// Returns rotation projected along a certain plane defined by its normal.
inline static glm::quat GetRotationOnPlane(const glm::quat& rotation,
                                           const glm::vec3& plane_normal) {
  const glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);

  // The transformed up vector for the device.
  glm::vec3 transformed = glm::rotate(rotation, world_up);

  // Project transformed vector onto plane.
  glm::vec3 flattened = glm::normalize(
      world_up - (glm::dot(world_up, plane_normal) * plane_normal));

  // Get angle between original vector and projected transform to get angle
  // around normal.
  return glm::rotation(transformed, flattened);
}
}  // namespace

namespace tango_video_stabilization {

Scene::Scene() {}

Scene::~Scene() {}

void Scene::InitGLContent() {
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // Allocating render camera and drawable object.
  // All of these objects are for visualization purposes.
  pose_buffer_.resize(kNumLookaheadFrames + 2);
  video_overlay_buffer_.resize(kNumLookaheadFrames + 1);
  for (unsigned i = 0; i < video_overlay_buffer_.size(); ++i) {
    video_overlay_buffer_[i] = new tango_gl::VideoOverlay();
  }

  is_content_initialized_ = true;
}

void Scene::DeleteResources() {
  if (is_content_initialized_) {
    for (unsigned i = 0; i < video_overlay_buffer_.size(); ++i) {
      delete video_overlay_buffer_[i];
    }

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
  camera_.SetProjectionMatrix(projection_matrix);
}

void Scene::Clear() {
  glViewport(0, 0, viewport_width_, viewport_height_);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void Scene::Render() {
  glViewport(0, 0, viewport_width_, viewport_height_);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  tango_gl::VideoOverlay* video_overlay =
      video_overlay_buffer_[oldest_video_overlay_buffer_index_];

  // The actual pose is the newest pose in the buffer.
  int current_pose_buffer_index =
      (oldest_pose_buffer_index_ + 1) % pose_buffer_.size();
  const TangoPoseData& current_pose = pose_buffer_[current_pose_buffer_index];
  glm::quat current_video_overlay_rotation =
      glm::quat(current_pose.orientation[3], current_pose.orientation[0],
                current_pose.orientation[1], current_pose.orientation[2]);

  video_stabilization_mutex_.lock();
  if (enable_video_stabilization_) {
    video_stabilization_mutex_.unlock();

    glm::quat current_camera_rotation;
    camera_locked_mutex_.lock();
    if (!is_camera_locked_) {
      current_camera_rotation = GetSmoothedCameraRotation();
    } else {
      current_camera_rotation = glm::quat(current_video_overlay_rotation);
    }
    camera_locked_mutex_.unlock();

    glm::quat roll_stabilizer =
        GetRotationOnPlane(current_video_overlay_rotation,
                           glm::rotate(current_video_overlay_rotation,
                                       glm::vec3(0.0f, 0.0f, 1.0f)));
    current_camera_rotation = roll_stabilizer * current_camera_rotation;
    camera_.SetRotation(current_camera_rotation);

    // Correct the video overlay's rotation to the gravity stabilized, linearly
    // smoothed rotation. Zoomed in 0%.
    video_overlay->SetScale(
        glm::vec3(kVideoOverlayZoomFactor,
                  camera_image_plane_ratio_ * kVideoOverlayZoomFactor,
                  kVideoOverlayZoomFactor));
    video_overlay->SetRotation(current_video_overlay_rotation);
    glm::vec3 forward = glm::vec3(glm::mat4_cast(video_overlay->GetRotation()) *
                                  glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
    video_overlay->SetPosition(forward * -image_plane_distance_);

    // Render the video overlay.
    glDisable(GL_DEPTH_TEST);
    video_overlay->Render(camera_.GetProjectionMatrix(),
                          camera_.GetViewMatrix());
    glEnable(GL_DEPTH_TEST);
  } else {
    video_stabilization_mutex_.unlock();
    camera_.SetRotation(current_video_overlay_rotation);

    video_overlay->SetScale(glm::vec3(kVideoOverlayZoomFactor));
    video_overlay->SetRotation(glm::quat(0.0f, 0.0f, 0.0f, 0.0f));
    video_overlay->SetPosition(glm::vec3(0.0f));

    // Render the video overlay.
    glDisable(GL_DEPTH_TEST);
    video_overlay->Render(glm::mat4(1.0f), glm::mat4(1.0f));
    glEnable(GL_DEPTH_TEST);
  }
}

glm::quat Scene::GetSmoothedCameraRotation() {
  // The actual pose is the newest pose in the buffer.
  int current_pose_buffer_index =
      (oldest_pose_buffer_index_ + 1) % pose_buffer_.size();
  const TangoPoseData& current_pose = pose_buffer_[current_pose_buffer_index];
  glm::quat current_video_overlay_rotation =
      glm::quat(current_pose.orientation[3], current_pose.orientation[0],
                current_pose.orientation[1], current_pose.orientation[2]);

  // Predict the future camera position.
  // Correct for difference between previous frame and current frame.
  int prev_pose_buffer_index = oldest_pose_buffer_index_;
  const TangoPoseData& prev_pose = pose_buffer_[prev_pose_buffer_index];
  glm::quat prev_video_overlay_rotation =
      glm::quat(prev_pose.orientation[3], prev_pose.orientation[0],
                prev_pose.orientation[1], prev_pose.orientation[2]);
  glm::quat prev_R_current_video_overlay_rotation =
      current_video_overlay_rotation *
      glm::conjugate(prev_video_overlay_rotation);

  // Use pose from future frames to correct for larger motions in the
  // future.
  int newest_pose_buffer_index =
      (oldest_pose_buffer_index_ + kNumLookaheadFrames + 1) %
      pose_buffer_.size();
  const TangoPoseData& newest_pose = pose_buffer_[newest_pose_buffer_index];
  for (int i = 0; i < kNumLookaheadFrames; ++i) {
    int lookahead_pose_buffer_index =
        (oldest_pose_buffer_index_ + 2 + i) % pose_buffer_.size();
    const TangoPoseData& lookahead_pose =
        pose_buffer_[lookahead_pose_buffer_index];
    glm::quat lookahead_video_overlay_rotation =
        glm::quat(lookahead_pose.orientation[3], lookahead_pose.orientation[0],
                  lookahead_pose.orientation[1], lookahead_pose.orientation[2]);
    glm::quat prev_R_lookahead_video_overlay_rotation =
        lookahead_video_overlay_rotation *
        glm::conjugate(prev_video_overlay_rotation);

    // The closer to our current frame the pose, the more we weight it's
    // rotation. The current pose would effectively have a weight of 0.
    float weight = (newest_pose.timestamp - lookahead_pose.timestamp) /
                   (newest_pose.timestamp - current_pose.timestamp);
    prev_R_current_video_overlay_rotation =
        glm::slerp(prev_R_current_video_overlay_rotation,
                   prev_R_lookahead_video_overlay_rotation, weight);
  }
  // Correct the camera rotation to the stabilized rotation.
  return prev_R_current_video_overlay_rotation * prev_video_overlay_rotation;
}

GLuint Scene::GetVideoOverlayTextureId() {
  int index = oldest_video_overlay_buffer_index_;
  oldest_video_overlay_buffer_index_ =
      (oldest_video_overlay_buffer_index_ + 1) % video_overlay_buffer_.size();
  return video_overlay_buffer_[index]->GetTextureId();
}

void Scene::AddNewPose(const TangoPoseData& pose) {
  pose_buffer_[oldest_pose_buffer_index_] = pose;
  oldest_pose_buffer_index_ =
      (oldest_pose_buffer_index_ + 1) % pose_buffer_.size();
}

void Scene::SetEnableVideoStabilization(bool enable_video_stabilization) {
  std::lock_guard<std::mutex> lock(video_stabilization_mutex_);
  enable_video_stabilization_ = enable_video_stabilization;
}

void Scene::SetCameraLocked(bool camera_locked) {
  std::lock_guard<std::mutex> lock(camera_locked_mutex_);
  is_camera_locked_ = camera_locked;
}

void Scene::SetDisplayRotation(TangoSupport_Rotation display_rotation) {
  for (unsigned i = 0; i < video_overlay_buffer_.size(); ++i) {
    video_overlay_buffer_[i]->SetDisplayRotation(display_rotation);
  }
}
}  // namespace tango_video_stabilization
