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

#ifndef TANGO_AREA_LEARNING_POSE_DATA_H_
#define TANGO_AREA_LEARNING_POSE_DATA_H_

#include <jni.h>
#include <mutex>

#include <tango_client_api.h>  // NOLINT
#include <tango-gl/util.h>

namespace tango_area_learning {

// PoseDataInfo is a data container class for storing pose data to compute all
// the debug data we want show on screen.
class PoseDataInfo {
 public:
  PoseDataInfo() {
    pose_string = "N/A";
    cur_pose = TangoPoseData();
    prev_pose = TangoPoseData();
    pose_counter = 0;
  }

  TangoPoseData cur_pose;
  TangoPoseData prev_pose;
  std::string pose_string;
  size_t pose_counter;
};

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

  // Get the debug string for the Device with respect to Start Service frame.
  //
  // @return: pose debug strings for dispaly on Java activity.
  std::string GetStartServiceTDeviceString();

  // Get the debug string for the Device with respect to ADF frame.
  //
  // @return: pose debug strings for dispaly on Java activity.
  std::string GetAdfTDeviceString();

  // Get the debug string for the Start service with respect to ADF frame.
  //
  // @return: pose debug strings for dispaly on Java activity.
  std::string GetAdfTStartServiceString();

  // Get pose data in current frame.
  //
  // @return: curent pose data.
  TangoPoseData GetCurrentPoseData();

  // Check if the device is relocalized.
  //
  // @return: relocalized flag.
  bool IsRelocalized() { return is_relocalized; }

 private:
  // Convert TangoPoseStatusType to string.
  //
  // @param: status, status code needs to be converted.
  //
  // @return: corresponding string based on status passed in.
  std::string GetStringFromStatusCode(TangoPoseStatusType status);

  // Format the pose debug string based on current pose and previous pose data.
  void FormatPoseString(PoseDataInfo* pose_data_info);

  // Pose data and debug information of start_service_T_device pose data.
  // start_service_T_device represents device with respect to start of service
  // frame.
  PoseDataInfo start_service_T_device_pose_;

  // Pose data and debug information of adf_T_device pose data.
  PoseDataInfo adf_T_device_pose_;

  // Pose data and debug information of adf_T_start_service pose data.
  PoseDataInfo adf_T_start_service_pose_;

  // Relocalized flag, the relocalization is determined by the pose in start of
  // service with respect to ADF turns to valid.
  bool is_relocalized = false;
};
}  // namespace tango_area_learning

#endif  // TANGO_AREA_LEARNING_POSE_DATA_H_