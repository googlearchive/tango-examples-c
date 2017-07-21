/*
 * Copyright 2017 Google Inc. All Rights Reserved.
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

#ifndef CPP_MARKER_DETECTION_EXAMPLE_TANGO_MARKER_DETECTION_MARKER_OBJECT_H_
#define CPP_MARKER_DETECTION_EXAMPLE_TANGO_MARKER_DETECTION_MARKER_OBJECT_H_

#include <vector>
#include <tango-gl/camera.h>
#include <tango-gl/util.h>

#include <tango_markers.h>

namespace tango_marker_detection {

// Marker object renders the bounding box and the coordinate axes of the local
// frame of a marker.
class MarkerObject {
 public:
  // Constructor and destructor.
  // @param marker_size the size of the marker in meter.
  explicit MarkerObject(float marker_size);
  ~MarkerObject();

  // Render the marker object.
  // @param camera the camera object to setup the view matrix.
  void Render(tango_gl::Camera* camera) const;

  // Update the object with the information from a marker.
  // @param marker a marker detected from input image.
  void Update(const TangoMarkers_Marker& marker);

 protected:
  void DrawBoundingBox(tango_gl::Camera* camera) const;
  void DrawAxes(tango_gl::Camera* camera) const;

 private:
  bool visible_;

  // Shader program and variables for displaying bounding box.
  GLuint box_shader_program_;
  GLuint box_uniform_mvp_;
  GLuint box_attrib_vertices_;
  GLuint box_uniform_color_;

  // Vertex array of bounding box.
  std::vector<glm::vec3> box_vertices_;

  // Shader program and variables for displaying axes of marker local frame.
  GLuint axis_shader_program_;
  GLuint axis_uniform_mvp_;
  GLuint axis_attrib_vertices_;
  GLuint axis_attrib_colors_;

  // Vertex array and color array of axes.
  std::vector<glm::vec3> axis_vertices_;
  std::vector<glm::vec4> axis_colors_;

  // Transformation from marker local frame to world frame.
  glm::mat4 world_T_local_;
};
}  // namespace tango_marker_detection

#endif  // CPP_MARKER_DETECTION_EXAMPLE_TANGO_MARKER_DETECTION_MARKER_OBJECT_H_
