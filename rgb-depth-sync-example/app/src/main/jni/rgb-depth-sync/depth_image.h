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

#ifndef RGB_DEPTH_SYNC_DEPTH_IMAGE_H_
#define RGB_DEPTH_SYNC_DEPTH_IMAGE_H_

#include <tango_client_api.h>
#include <tango-gl/util.h>
#include <thread>
#include <mutex>
namespace rgb_depth_sync {

// DepthImage is a class which projects point cloud on to a drawable
// image plane.
class DepthImage {
 public:
  explicit DepthImage();
  ~DepthImage();

  // Update the depth texture with current transformation and current depth map.
  // @param  Ci_T_Cj: The transformation between the camera frame on timestamp i
  //    (color camera timestamp) and the camera frame on timestamp j (depth
  //    camera timestamp)
  // To convert a point in the camera frame on timestamp j to the camera frame
  // on timestamp i, you could do:
  //    Ci_point = Ci_T_Cj * Cj_point;
  //
  // @param render_point_cloud_buffer: This contains the latest point cloud data
  // that gets projected on to the image plane and fills up the depth_map_buffer
  void UpdateAndUpsampleDepth(
      glm::mat4& Ci_T_Cj, const std::vector<float>& render_point_cloud_buffer);

  // Returns the depth texture id.
  GLuint GetTextureId() const { return texture_id_; }

  // Set camera's intrinsics.
  // The intrinsics are used to project the pointcloud to depth image and
  // and undistort the image to the right size.
  void SetCameraIntrinsics(TangoCameraIntrinsics intrinsics);

 private:
  // This function takes care of upsampling depth around a given point by
  // setting the same value in a bounding box. Note:This is a very rudimentary
  // approach to upsampling depth.
  //
  // @param color_value: The value that needs to set for all points in a
  // bounding box.
  //
  // @param pixel_x: The pixel along x axis in the grayscale_buffer around which
  // depth needs to be upsampled.
  //
  // @param pixel_y: The pixel along y axis in the grayscale_buffer around which
  // depth needs to be upsampled.
  //
  // @param grayscale_buffer: The image buffer in which upsampling needs to be
  // done.
  void UpSampleDepthAroundPoint(uint16_t color_value, float depth_value,
                                int pixel_x, int pixel_y,
                                std::vector<uint16_t>* grayscale_buffer,
                                std::vector<float>* depth_map_buffer);

  // The defined max distance for a depth value.
  static const int kMaxDepthDistance = 4000;

  // The meter to millimeter conversion.
  static const int kMeterToMillimeter = 1000;

  // Window size for splatter upsample
  static const int kWindowSize = 7;

  // The depth texture id. This is used for other rendering class to render, in
  // this class, we only write value to this texture id associated buffer, no
  // rendering tasks.
  GLuint texture_id_;

  std::vector<float> depth_map_buffer_;

  // Color map buffer is for the texture render purpose, this value is written
  // to the texture id buffer, and display as GL_LUMINANCE value.
  std::vector<uint16_t> grayscale_display_buffer_;

  // The camera intrinsics of current device. Note that the color camera and
  // depth camera are the same hardware on the device.
  TangoCameraIntrinsics rgb_camera_intrinsics_;

};
}  // namespace rgb_depth_sync

#endif  // RGB_DEPTH_SYNC_DEPTH_IMAGE_H_
