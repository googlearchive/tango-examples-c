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

#ifndef CPP_BASIC_EXAMPLES_HELLO_AREA_DESCRIPTION_POSE_DATA_H_
#define CPP_BASIC_EXAMPLES_HELLO_AREA_DESCRIPTION_POSE_DATA_H_

#include <jni.h>
#include <mutex>
#include <string>

#include <tango_client_api.h>  // NOLINT

namespace hello_area_description {
// PoseData holds all pose related data. E.g. pose position, rotation and time-
// stamp. It also produce the debug information strings.
class PoseData {
 public:
  PoseData();
  ~PoseData();

  // Update current pose and previous pose.
  //
  // @param pose: pose data of current frame.
  void UpdatePose(const TangoPoseData& pose_data);

  // Reset all saved pose data.
  void ResetPoseData();

  // Get pose data in current frame.
  //
  // @return: curent pose data.
  TangoPoseData GetCurrentPoseData();

  // Check if the device is relocalized.
  //
  // @return: relocalized flag.
  bool IsRelocalized() { return is_relocalized_; }

 private:
  // Relocalized flag, the relocalization is determined by the pose in start of
  // service with respect to ADF turns to valid.
  bool is_relocalized_ = false;

  TangoPoseData adf_T_device_pose_;
  TangoPoseData start_service_T_device_pose_;
};
}  // namespace hello_area_description

#endif  // CPP_BASIC_EXAMPLES_HELLO_AREA_DESCRIPTION_POSE_DATA_H_
