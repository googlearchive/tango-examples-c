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

#include "tango-gl/conversions.h"

#include "rgb-depth-sync/depth_image.h"

namespace rgb_depth_sync {

DepthImage::DepthImage() {
  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE1);
  glGenTextures(1, &texture_id_);
  tango_gl::util::CheckGlError("DepthImage while initialization");
}

DepthImage::~DepthImage() {
  glDeleteTextures(1, &texture_id_);
}

// Update function will be called in application's main render loop. This funct-
// ion takes care of projecting raw depth points on the image plane, and render
// the depth image into a texture.
// This function also takes care of swapping the Render buffer and shared buffer
// if there is new point cloud data available.
// Ci_T_Cj: 'i' represents the color camera frame's timestamp, 'j' represents
// the depth camera timestamp. The transformation is the camera's frame on
// timestamp j with respect the camera's frame on timestamp i.
// This transform can move a point from depth camera frame to color camera
// frame.
void DepthImage::UpdateAndUpsampleDepth(
    glm::mat4& Ci_T_Cj, const std::vector<float>& render_point_cloud_buffer) {

  int depth_image_width = rgb_camera_intrinsics_.width;
  int depth_image_height = rgb_camera_intrinsics_.height;
  int depth_image_size = depth_image_width * depth_image_height;

  depth_map_buffer_.resize(depth_image_size);
  grayscale_display_buffer_.resize(depth_image_size);
  std::fill(depth_map_buffer_.begin(), depth_map_buffer_.end(), 0);
  std::fill(grayscale_display_buffer_.begin(), grayscale_display_buffer_.end(),
            0);

  int point_cloud_size = render_point_cloud_buffer.size();
  for (int i = 0; i < point_cloud_size; i += 3) {
    float x = render_point_cloud_buffer[i];
    float y = render_point_cloud_buffer[i + 1];
    float z = render_point_cloud_buffer[i + 2];

    // Cj_point is the point in camera frame on timestamp j (depth timestamp).
    glm::vec4 Cj_point = glm::vec4(x, y, z, 1.0);
    // Ci_point is the point in camera frame on timestamp i (color timestamp).
    glm::vec4 Ci_point = Ci_T_Cj * Cj_point;

    int pixel_x, pixel_y;
    // get the coordinate on image plane.
    pixel_x = static_cast<int>((rgb_camera_intrinsics_.fx) *
                                   (Ci_point.x / Ci_point.z) +
                               rgb_camera_intrinsics_.cx);

    pixel_y = static_cast<int>((rgb_camera_intrinsics_.fy) *
                                   (Ci_point.y / Ci_point.z) +
                               rgb_camera_intrinsics_.cy);

    // Color value is the GL_LUMINANCE value used for displaying the depth
    // image.
    // We can query for depth value in mm from grayscale image buffer by
    // getting a `pixel_value` at (pixel_x,pixel_y) and calculating
    // pixel_value * (kMaxDepthDistance / USHRT_MAX)
    float depth_value = Ci_point.z;
    uint16_t grayscale_value =
        (Ci_point.z * kMeterToMillimeter) * USHRT_MAX / kMaxDepthDistance;

    UpSampleDepthAroundPoint(grayscale_value, depth_value, pixel_x, pixel_y,
                             &grayscale_display_buffer_, &depth_map_buffer_);
  }

  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, depth_image_width,
               depth_image_height, 0, GL_LUMINANCE, GL_UNSIGNED_SHORT,
               grayscale_display_buffer_.data());
  tango_gl::util::CheckGlError("DepthImage glTexImage2D");
}

void DepthImage::SetCameraIntrinsics(TangoCameraIntrinsics intrinsics) {
  rgb_camera_intrinsics_ = intrinsics;
}

void DepthImage::UpSampleDepthAroundPoint(
    uint16_t grayscale_value, float depth_value, int pixel_x, int pixel_y,
    std::vector<uint16_t>* grayscale_buffer,
    std::vector<float>* depth_map_buffer) {

  int image_width = rgb_camera_intrinsics_.width;
  int image_height = rgb_camera_intrinsics_.height;
  int image_size = image_height * image_width;
  // Set the neighbour pixels to same color.
  for (int a = -kWindowSize; a <= kWindowSize; ++a) {
    for (int b = -kWindowSize; b <= kWindowSize; ++b) {
      if (pixel_x > image_width || pixel_y > image_height || pixel_x < 0 ||
          pixel_y < 0) {
        continue;
      }
      int pixel_num = (pixel_x + a) + (pixel_y + b) * image_width;

      if (pixel_num > 0 && pixel_num < image_size) {
        (*grayscale_buffer)[pixel_num] = grayscale_value;
        (*depth_map_buffer)[pixel_num] = depth_value;
      }
    }
  }
}

}  // namespace rgb_depth_sync
