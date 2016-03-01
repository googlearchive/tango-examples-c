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

#include "tango-augmented-reality/tango_event_data.h"

namespace tango_augmented_reality {

TangoEventData::TangoEventData() {}

TangoEventData::~TangoEventData() {}

// Update current event string to the event passed in.
// In this application, we are just using these event for debug purpose, but in
// other application, developers could catch this event for exception handling.
//
// @param: event, TangoEvent in current frame.
void TangoEventData::UpdateTangoEvent(const TangoEvent* event) {
  std::stringstream string_stream;
  string_stream << event->event_key << ": " << event->event_value;
  event_string_ = string_stream.str();
  string_stream.flush();
}

// Clear event string. Set event_string_ to empty.
void TangoEventData::ClearEventString() { event_string_.clear(); }

// Get formated event string for debug dispaly purpose.
std::string TangoEventData::GetTangoEventString() { return event_string_; }

}  // namespace tango_augmented_reality
