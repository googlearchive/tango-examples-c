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

#include "tango-area-learning/pose_data.h"

namespace {
const float kMeterToMillimeter = 1000.0f;
}  // namespace

namespace tango_area_learning {

PoseData::PoseData() {}

PoseData::~PoseData() {}

void PoseData::UpdatePose(const TangoPoseData& pose_data) {
  PoseDataInfo* pose_data_info;
  // We check the frame pair received in the pose_data instance, and store it
  // in the proper cooresponding local member.
  //
  // The frame pairs are the one we registred in the
  // TangoService_connectOnPoseAvailable functions during the setup of the
  // Tango configuration file.
  if (pose_data.frame.base == TANGO_COORDINATE_FRAME_START_OF_SERVICE &&
      pose_data.frame.target == TANGO_COORDINATE_FRAME_DEVICE) {
    pose_data_info = &start_service_T_device_pose_;
  } else if (pose_data.frame.base == TANGO_COORDINATE_FRAME_AREA_DESCRIPTION &&
             pose_data.frame.target == TANGO_COORDINATE_FRAME_DEVICE) {
    pose_data_info = &adf_T_device_pose_;
  } else if (pose_data.frame.base == TANGO_COORDINATE_FRAME_AREA_DESCRIPTION &&
             pose_data.frame.target ==
                 TANGO_COORDINATE_FRAME_START_OF_SERVICE) {
    pose_data_info = &adf_T_start_service_pose_;
    is_relocalized = (pose_data.status_code == TANGO_POSE_VALID);
  } else {
    return;
  }

  pose_data_info->cur_pose = pose_data;

  if (pose_data_info->prev_pose.status_code !=
      pose_data_info->cur_pose.status_code) {
    // Reset pose counter when the status changed.
    pose_data_info->pose_counter = 0;
  }

  // Increase pose counter.
  ++pose_data_info->pose_counter;
  FormatPoseString(pose_data_info);
  pose_data_info->prev_pose = pose_data_info->cur_pose;
}

void PoseData::ResetPoseData() {
  is_relocalized = false;
  start_service_T_device_pose_ = PoseDataInfo();
  adf_T_device_pose_ = PoseDataInfo();
  adf_T_start_service_pose_ = PoseDataInfo();
}

std::string PoseData::GetStartServiceTDeviceString() {
  return start_service_T_device_pose_.pose_string;
}

std::string PoseData::GetAdfTDeviceString() {
  return adf_T_device_pose_.pose_string;
}

std::string PoseData::GetAdfTStartServiceString() {
  return adf_T_start_service_pose_.pose_string;
}

TangoPoseData PoseData::GetCurrentPoseData() {
  if (is_relocalized) {
    return adf_T_device_pose_.cur_pose;
  } else {
    return start_service_T_device_pose_.cur_pose;
  }
}

std::string PoseData::GetStringFromStatusCode(TangoPoseStatusType status) {
  std::string ret_string;
  switch (status) {
    case TANGO_POSE_INITIALIZING:
      ret_string = "initializing";
      break;
    case TANGO_POSE_VALID:
      ret_string = "valid";
      break;
    case TANGO_POSE_INVALID:
      ret_string = "invalid";
      break;
    case TANGO_POSE_UNKNOWN:
      ret_string = "unknown";
      break;
    default:
      ret_string = "status_code_invalid";
      break;
  }
  return ret_string;
}

void PoseData::FormatPoseString(PoseDataInfo* pose_data_info) {
  std::stringstream string_stream;
  string_stream.setf(std::ios_base::fixed, std::ios_base::floatfield);
  string_stream.precision(3);
  string_stream << "status: "
                << GetStringFromStatusCode(pose_data_info->cur_pose.status_code)
                << ", count: " << pose_data_info->pose_counter
                << ", delta time (ms): "
                << (pose_data_info->cur_pose.timestamp -
                    pose_data_info->prev_pose.timestamp) *
                       kMeterToMillimeter << ", position (m): ["
                << pose_data_info->cur_pose.translation[0] << ", "
                << pose_data_info->cur_pose.translation[1] << ", "
                << pose_data_info->cur_pose.translation[2] << "]"
                << ", orientation: [" << pose_data_info->cur_pose.orientation[0]
                << ", " << pose_data_info->cur_pose.orientation[1] << ", "
                << pose_data_info->cur_pose.orientation[2] << ", "
                << pose_data_info->cur_pose.orientation[3] << "]";
  pose_data_info->pose_string = string_stream.str();
  string_stream.flush();
}

} //namespace tango_area_learning