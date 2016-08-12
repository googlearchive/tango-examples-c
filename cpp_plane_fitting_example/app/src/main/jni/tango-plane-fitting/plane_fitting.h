/*
 * Copyright 2015 Google Inc. All Rights Reserved.
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

#ifndef TANGO_PLANE_FITTING_PLANE_FITTING_H_
#define TANGO_PLANE_FITTING_PLANE_FITTING_H_

#include <vector>

#include <glm/glm.hpp>
#include <tango_support_api.h>

namespace tango_plane_fitting {

void PlaneTransform(const glm::vec4& in_plane, const glm::mat4& out_T_in,
                    glm::vec4* out_plane);

}  // namespace tango_plane_fitting

#endif  // TANGO_PLANE_FITTING_PLANE_FITTING_H_
