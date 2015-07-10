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

#include "tango-point-cloud/tango_event_data.h"

namespace tango_point_cloud {

TangoEventData::TangoEventData() {}

TangoEventData::~TangoEventData() {}

void TangoEventData::UpdateTangoEvent(const TangoEvent* event) {
  std::stringstream string_stream;
  string_stream << event->event_key << ": " << event->event_value;
  event_string_ = string_stream.str();
  string_stream.flush();
}

void TangoEventData::ClearEventString() {
  event_string_.clear();
}

std::string TangoEventData::GetTangoEventString() {
  return event_string_;
}

}  // namespace tango_point_cloud
