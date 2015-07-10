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

#ifndef TANGO_MOTION_TRACKING_POSE_DATA_H_
#define TANGO_MOTION_TRACKING_POSE_DATA_H_

#include <jni.h>
#include <mutex>

#include <tango_client_api.h>  // NOLINT
#include <tango-gl/util.h>

namespace tango_motion_tracking {

// PoseData holds all pose related data. E.g. pose position, rotation and time-
// stamp. It also produce the debug information strings.
class PoseData {
 public:
  PoseData();
  ~PoseData();

  // Update current pose and previous pose.
  //
  // @param pose: pose data of current frame.
  void UpdatePose(const TangoPoseData* pose_data);

  // Compose the pose debug string.
  //
  // @return: pose debug strings for dispaly on Java activity.
  std::string GetPoseDebugString();

  // Get pose data in current frame.
  //
  // @return: curent pose data.
  TangoPoseData GetCurrentPoseData();

 private:
  // Convert TangoPoseStatusType to string.
  //
  // @param: status, status code needs to be converted.
  //
  // @return: corresponding string based on status passed in.
  std::string GetStringFromStatusCode(TangoPoseStatusType status);

  // Format the pose debug string based on current pose and previous pose data.
  void FormatPoseString();

  // Pose data of current frame.
  TangoPoseData cur_pose_;

  // prev_pose_, pose_counter_ and pose_debug_string_ are used for composing the
  // debug string to display the useful information on screen.
  TangoPoseData prev_pose_;

  // Debug pose string.
  std::string pose_string_;

  // Pose counter for debug purpose.
  size_t pose_counter_;
};
}  // namespace tango_motion_tracking

#endif  // TANGO_MOTION_TRACKING_POSE_DATA_H_