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

#ifndef TANGO_POINT_CLOUD_TANGO_EVENT_DATA_H_
#define TANGO_POINT_CLOUD_TANGO_EVENT_DATA_H_

#include <jni.h>
#include <mutex>

#include <tango_client_api.h>  // NOLINT

namespace tango_point_cloud {

// TangoEvent is handling tango event callbacks (e.g, TooFewFeaturesTracked)
// Currently, we are just exposing each event through the debug text displayed on
// screen. But developers could take advantage of these events to handle the
// exception in a more user friendly manner.
class TangoEventData {
 public:
  // Constructor and deconstructor.
  TangoEventData();
  ~TangoEventData();

  // Update the current event string to the event passed in.
  // In this application, we are just using these event for debug purposes, but
  // in other application, developers could catch this event for exception
  // handling.
  //
  // @param: event, the TangoEvent.
  void UpdateTangoEvent(const TangoEvent* event);

  // Clear event string. Set event_string_ to empty.
  void ClearEventString();

  // Get formated event string for debug dispaly purposes.
  std::string GetTangoEventString();
 private:
  // Current event string.
  std::string event_string_;
};
}  // namespace tango_point_cloud

#endif  // TANGO_POINT_CLOUD_TANGO_EVENT_DATA_H_
