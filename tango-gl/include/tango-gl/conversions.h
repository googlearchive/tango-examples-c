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

#ifndef TANGO_GL_GL_TANGO_CONVERSIONS_H_
#define TANGO_GL_GL_TANGO_CONVERSIONS_H_

#define GLM_FORCE_RADIANS

#include "glm/glm.hpp"

namespace tango_gl {
namespace conversions {

/**
 * @brief Convert (re-express, or rotate) a vector from the Tango ADF (or start-
 * of-service) frame convention [right, forward, up] to the typical OpenGl world
 * frame convention [right, up, backward]. Note this assumes the two frames are
 * coincident, and it doesn't know about any additional offsets between a
 * particular OpenGl scene and the Tango service frames.
 * @param tango_vec A vector expressed in the Tango ADF frame convention.
 * @return The same vector expressed using the Opengl frame convention.
 */
inline glm::vec3 ConvertVec3TangoToGl(const glm::vec3& tango_vec) {
  return glm::vec3(tango_vec.x, tango_vec.z, -tango_vec.y);
}

/**
 * @brief Convert (re-express, or rotate) a vector from the typical OpenGl world
 * frame convention [right, up, backward] to the Tango ADF (or start-of-service)
 * frame convention [right, forward, up]. Note this assumes the two frames are
 * coincident, and it doesn't know about any additional offsets between a
 * particular OpenGl scene and the Tango service frames.
 * @param gl_vec A vector expressed in the Opengl world frame convention.
 * @return The same vector expressed using the Tango ADF frame convention.
 */
inline glm::vec3 ConvertVec3GlToTango(const glm::vec3& gl_vec) {
  return glm::vec3(gl_vec.x, -gl_vec.z, gl_vec.y);
}

/**
 * Get the fixed transformation matrix relating the opengl frame convention
 * (with Y-up, X-right) and the tango frame convention for the start-of-service
 * and ADF frames (with Z-up, X-right), termed "world" here.
 */
glm::mat4 opengl_world_T_tango_world();

/**
 * Get the fixed transformation matrix relating the frame convention of the
 * device's color camera frame (with Z-forward, X-right) and the opengl camera
 * frame (with Z-backward, X-right).
 */
glm::mat4 color_camera_T_opengl_camera();

}  // namespace conversions
}  // namespace tango_gl
#endif  // TANGO_GL_GL_TANGO_CONVERSIONS_H_
