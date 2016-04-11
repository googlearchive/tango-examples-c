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

#include "hello_area_description/pose_data.h"

namespace hello_area_description {

PoseData::PoseData() {}

PoseData::~PoseData() {}

void PoseData::UpdatePose(const TangoPoseData& pose_data) {
  // We check the frame pair received in the pose_data instance, and store it
  // in the proper cooresponding local member.
  //
  // The frame pairs are the one we registred in the
  // TangoService_connectOnPoseAvailable functions during the setup of the
  // Tango configuration file.
  if (pose_data.frame.base == TANGO_COORDINATE_FRAME_START_OF_SERVICE &&
      pose_data.frame.target == TANGO_COORDINATE_FRAME_DEVICE) {
    start_service_T_device_pose_ = pose_data;
  } else if (pose_data.frame.base == TANGO_COORDINATE_FRAME_AREA_DESCRIPTION &&
             pose_data.frame.target == TANGO_COORDINATE_FRAME_DEVICE) {
    adf_T_device_pose_ = pose_data;
  } else if (pose_data.frame.base == TANGO_COORDINATE_FRAME_AREA_DESCRIPTION &&
             pose_data.frame.target ==
                 TANGO_COORDINATE_FRAME_START_OF_SERVICE) {
    is_relocalized_ = (pose_data.status_code == TANGO_POSE_VALID);
  } else {
    return;
  }
}

void PoseData::ResetPoseData() { is_relocalized_ = false; }

TangoPoseData PoseData::GetCurrentPoseData() {
  if (is_relocalized_) {
    return adf_T_device_pose_;
  } else {
    return start_service_T_device_pose_;
  }
}

}  // namespace hello_area_description
