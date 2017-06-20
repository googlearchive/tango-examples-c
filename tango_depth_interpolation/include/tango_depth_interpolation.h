// Copyright 2017 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TANGO_DEPTH_INTERPOLATION_TANGO_DEPTH_INTERPOLATION_H_
#define TANGO_DEPTH_INTERPOLATION_TANGO_DEPTH_INTERPOLATION_H_

#include <tango_client_api.h>
#include <tango_support.h>

/// @file tango_depth_interpolation.h
/// @brief File containing depth interpolation functions of Tango Support C API.
///   The Tango Support C API provides helper functions useful to external
///   developers for building Tango applications. This API is experimental and
///   subject to change.

#ifdef __cplusplus
extern "C" {
#endif

/// @defgroup DepthInterpolationSupport Depth Interpolation Support Functions
/// @brief Functions for interpolating depth.
/// @{

/// @brief Calculates the depth at a user-specified location using
///   nearest-neighbor interpolation. The output is in the base frame of the
///   input translations and rotations. This output frame is usually an
///   application's world frame.
///
/// @param point_cloud The point cloud. Cannot be NULL and must have at least
///   one point.
/// @param point_cloud_translation The translation component of the
///   transformation from the point cloud to the output frame. Cannot be NULL.
/// @param point_cloud_orientation The orientation component (as a quaternion)
///   of the transformation from the point cloud to the output frame.
///   Cannot be NULL.
/// @param color_camera_uv_coordinates The UV coordinates for the user
///   selection. This is expected to be between (0.0, 0.0) and (1.0, 1.0) and
///   can be computed from pixel coordinates by dividing by the width or
///   height. Cannot be NULL.
/// @param display_rotation The index of the display rotation between
///   display's default (natural) orientation and current orientation.
/// @param color_camera_translation The translation component of the
///   transformation from the color camera to the output frame. Cannot be NULL.
/// @param color_camera_orientation The orientation component (as a quaternion)
///   of the transformation from the color camera to the output frame.
///   Cannot be NULL.
/// @param output_point The point (x, y, z), which is the transformation of the
///   point (x', y', z') in the color camera space to the desired output frame.
///   (x', y') is the back-projection of the UV coordinates to the color camera
///   space and z' is the z coordinate of the point in the point cloud nearest
///   to the user selection after projection onto the image plane. If there is
///   no point in the point cloud close to the user selection after projection
///   onto the image plane, then the point will be set to (0.0, 0.0, 0.0) and
///   @c TANGO_ERROR will be returned.
/// @return @c TANGO_SUCCESS on success, @c TANGO_ERROR if a valid point is not
///   found, or @c TANGO_INVALID on invalid input.
TangoErrorType TangoDepthInterpolation_getDepthAtPointNearestNeighbor(
    const TangoPointCloud* point_cloud, const double point_cloud_translation[3],
    const double point_cloud_orientation[4],
    const float color_camera_uv_coordinates[2],
    TangoSupport_Rotation display_rotation,
    const double color_camera_translation[3],
    const double color_camera_orientation[4], float output_point[3]);

/// @brief The TangoDepthInterpolation_Interpolator contains references to
///   camera intrinsics and cached data structures needed to upsample depth data
///   to a camera image.
struct TangoDepthInterpolation_Interpolator;

/// @brief Create an object for depth interpolation.
///
/// @param interpolator A handle to the interpolator object.
/// @return @c TANGO_SUCCESS on successful creation, or @c TANGO_INVALID if
///   @p intrinsics was null.
TangoErrorType TangoDepthInterpolation_createDepthInterpolator(
    TangoDepthInterpolation_Interpolator** interpolator);

/// @brief Free the depth interpolation object.
///
/// @param A handle to the interpolator object.
/// @return @c TANGO_SUCCESS
TangoErrorType TangoDepthInterpolation_freeDepthInterpolator(
    TangoDepthInterpolation_Interpolator* interpolator);

/// @brief Calculates the depth at a user-specified location using bilateral
///   filtering weighted by both spatial distance from the user coordinate and
///   by intensity similarity. The output is in the base frame of the input
///   translations and rotations. This output frame is usually an application's
///   world frame.
///
/// @param interpolator A handle to the interpolator object. The intrinsics of
///   this interpolator object must match those of the image_buffer.
/// @param point_cloud The point cloud. Cannot be NULL and must have at least
///   one point.
/// @param point_cloud_translation The translation component of the
///   transformation from the point cloud to the output frame. Cannot be NULL.
/// @param point_cloud_orientation The orientation component (as a quaternion)
///   of the transformation from the point cloud to the output frame.
///   Cannot be NULL.
/// @param image_buffer The RGB image buffer. This must have intrinsics
///   matching those used to create the interpolator object. Cannot be NULL.
/// @param color_camera_uv_coordinates The UV coordinates for the user
///   selection. This is expected to be between (0.0, 0.0) and (1.0, 1.0) and
///   can be computed from pixel coordinates by dividing by the width or
///   height. Cannot be NULL.
/// @param display_rotation The index of the display rotation between
///   display's default (natural) orientation and current orientation.
/// @param color_camera_translation The translation component of the
///   transformation from the color camera to the output frame. Cannot be NULL.
/// @param color_camera_orientation The orientation component (as a quaternion)
///   of the transformation from the color camera to the output frame.
///   Cannot be NULL.
/// @param output_point The point (x, y, z), which is the transformation of the
///   point (x', y', z') in the color camera space to the desired output frame.
///   (x', y') is the back-projection of the UV coordinates to the color camera
///   space and z' is the bilateral interpolation of the z coordinate of the
///   point. If the bilateral interpolation fails then the point will be set to
///   (0.0, 0.0, 0.0) and @c TANGO_ERROR will be returned.
/// @return @c TANGO_SUCCESS on success, @c TANGO_ERROR if a valid point is not
///   found, or @c TANGO_INVALID on invalid input.
TangoErrorType TangoDepthInterpolation_getDepthAtPointBilateral(
    const TangoDepthInterpolation_Interpolator* interpolator,
    const TangoPointCloud* point_cloud, const double point_cloud_translation[3],
    const double point_cloud_orientation[4],
    const TangoImageBuffer* image_buffer,
    const float color_camera_uv_coordinates[2],
    TangoSupport_Rotation display_rotation,
    const double color_camera_translation[3],
    const double color_camera_orientation[4], float output_point[3]);

/// @brief A structure to hold depth values for image upsampling. The units of
///   the depth are the same as for @c TangoPointCloud.
struct TangoDepthInterpolation_DepthBuffer {
  float* depths;
  uint32_t width;
  uint32_t height;
};

/// @brief Allocate memory for a depth buffer for the given image resolution.
///
/// @param width The width of the image. This should match the width given by
///   the camera intrinsics.
/// @param height The height of the image. This should match the height given by
///   the camera intrinsics.
/// @param depth_buffer The depth buffer to initialize.
/// @return @c TANGO_SUCCESS on success, @c TANGO_INVALID on invalid input, and
///   @c TANGO_ERROR on failure.
TangoErrorType TangoDepthInterpolation_initializeDepthBuffer(
    uint32_t width, uint32_t height,
    TangoDepthInterpolation_DepthBuffer* depth_buffer);

/// @brief Free memory for the depth buffer.
///
/// @param depth_buffer The depth buffer to free.
/// @return @c TANGO_SUCCESS on success, @c TANGO_INVALID on invalid input, and
///   @c TANGO_ERROR on failure.
TangoErrorType TangoDepthInterpolation_freeDepthBuffer(
    TangoDepthInterpolation_DepthBuffer* depth_buffer);

/// @brief Upsamples the depth data to the resolution of the depth buffer. This
///   uses the resolution specified by the intrinsics used to construct the
///   interpolator. This function fills depth around each sample using a fixed
///   radius. The resolution of the intrinsics provided to the interpolator and
///   the resolution of the output depth_buffer must match.
///
/// @param interpolator A handle to the interpolator object. The intrinsics of
///   this interpolator object must match those of the image_buffer. Cannot be
///   NULL.
/// @param point_cloud The point cloud. Cannot be NULL and must have at least
///   one point.
/// @param color_camera_T_point_cloud The pose of the point cloud relative to
///   the color camera used to obtain uv_coordinates. Cannot be NULL.
/// @param depth_buffer A buffer for output of the depth data. Each pixel
///   contains a depth value (in meters) or zero if there is no depth data near
///   enough to the pixel. Cannot be NULL.
/// @return @c TANGO_SUCCESS on success, @c TANGO_INVALID on invalid input, and
///   @c TANGO_ERROR on failure.
TangoErrorType TangoDepthInterpolation_upsampleImageNearestNeighbor(
    const TangoDepthInterpolation_Interpolator* interpolator,
    const TangoPointCloud* point_cloud,
    const TangoPoseData* color_camera_T_point_cloud,
    TangoDepthInterpolation_DepthBuffer* depth_buffer);

/// @brief Upsamples the depth data to the resolution of the depth buffer. This
///   uses the resolution specified by the intrinsics used to construct the
///   interpolator. This function fills depth around each sample using a
///   bilateral filtering approach. The resolution of the intrinsics provided
///   to the interpolator and the resolution of the output depth_buffer must
///   match.
///
/// @param interpolator A handle to the interpolator object. The intrinsics of
///   this interpolator object must match those of the image_buffer. Cannot be
///   NULL.
/// @param approximate If non-zero, uses an approximation technique that is
///   faster but somewhat less accurate. If zero, use the a slower technique
///   that is slightly more accurate.
/// @param point_cloud The point cloud. Cannot be NULL and must have at least
///   one point.
/// @param color_camera_T_point_cloud The pose of the point cloud relative to
///   the color camera used to obtain uv_coordinates. Cannot be NULL.
/// @param depth_buffer A buffer for output of the depth data. Each pixel
///   contains a depth value (in meters) or zero if there is no depth data near
///   enough to the pixel. Cannot be NULL.
/// @return @c TANGO_SUCCESS on success, @c TANGO_INVALID on invalid input, and
///   @c TANGO_ERROR on failure.
TangoErrorType TangoDepthInterpolation_upsampleImageBilateral(
    const TangoDepthInterpolation_Interpolator* interpolator, int approximate,
    const TangoPointCloud* point_cloud, const TangoImageBuffer* image_buffer,
    const TangoPoseData* color_camera_T_point_cloud,
    TangoDepthInterpolation_DepthBuffer* depth_buffer);

/// @}

#ifdef __cplusplus
}
#endif

#endif  // TANGO_DEPTH_INTERPOLATION_TANGO_DEPTH_INTERPOLATION_H_
