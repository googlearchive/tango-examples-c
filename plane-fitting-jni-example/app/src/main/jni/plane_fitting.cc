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

#include "tango-plane-fitting/plane_fitting.h"

#include <tango-gl/util.h>

namespace tango_plane_fitting {

void PlaneTransform(const glm::vec4& in_plane, const glm::mat4& out_T_in,
                    glm::vec4* out_plane) {
  if (!out_plane) {
    LOGE("PlaneFitting: Invalid input to plane transform");
    return;
  }

  const glm::vec4 input_normal(glm::vec3(in_plane), 0.0f);
  const glm::vec4 input_origin(
      -static_cast<float>(in_plane[3]) * glm::vec3(input_normal), 1.0f);

  const glm::vec4 out_origin = out_T_in * input_origin;
  const glm::vec4 out_normal =
      glm::transpose(glm::inverse(out_T_in)) * input_normal;

  *out_plane =
      glm::vec4(glm::vec3(out_normal),
                -glm::dot(glm::vec3(out_origin), glm::vec3(out_normal)));
}

}  // namespace tango_plane_fitting
