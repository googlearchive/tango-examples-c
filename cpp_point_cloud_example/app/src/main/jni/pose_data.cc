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

#include <sstream>
#include <tango-gl/conversions.h>

#include "tango-point-cloud/pose_data.h"

namespace {
const float kMeterToMillimeter = 1000.0f;
}  // namespace

namespace tango_point_cloud {

PoseData::PoseData() {}

PoseData::~PoseData() {}

void PoseData::UpdatePose(const TangoPoseData* pose_data) {
  cur_pose_ = *pose_data;
}

glm::mat4 PoseData::GetLatestPoseMatrix() {
  return GetMatrixFromPose(cur_pose_);
}

glm::mat4 PoseData::GetExtrinsicsAppliedOpenGLWorldFrame(
    const glm::mat4 pose_matrix) {
  // This full multiplication is equal to:
  //   opengl_world_T_opengl_camera =
  //      opengl_world_T_start_service *
  //      start_service_T_device *
  //      device_T_imu *
  //      imu_T_depth_camera *
  //      depth_camera_T_opengl_camera;
  //
  // More information about frame transformation can be found here:
  // Frame of reference:
  //   https://developers.google.com/project-tango/overview/frames-of-reference
  // Coordinate System Conventions:
  //   https://developers.google.com/project-tango/overview/coordinate-systems
  return tango_gl::conversions::opengl_world_T_tango_world() * pose_matrix *
         glm::inverse(GetImuTDevice()) * GetImuTDepthCamera() *
         tango_gl::conversions::depth_camera_T_opengl_camera();
}

glm::mat4 PoseData::GetMatrixFromPose(const TangoPoseData& pose) {
  // Convert pose data to vec3 for position and quaternion for orientation.
  //
  // More information about frame transformation can be found here:
  // Frame of reference:
  //   https://developers.google.com/project-tango/overview/frames-of-reference
  // Coordinate System Conventions:
  //   https://developers.google.com/project-tango/overview/coordinate-systems
  glm::vec3 translation =
      glm::vec3(pose.translation[0], pose.translation[1], pose.translation[2]);
  glm::quat rotation = glm::quat(pose.orientation[3], pose.orientation[0],
                                 pose.orientation[1], pose.orientation[2]);
  glm::mat4 matrix =
      glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(rotation);
  return matrix;
}

}  // namespace tango_point_cloud
