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

#ifndef TANGO_TRANSFORM_HELPERS_TANGO_TRANSFORM_HELPERS_H_
#define TANGO_TRANSFORM_HELPERS_TANGO_TRANSFORM_HELPERS_H_

#include <tango_client_api.h>

/// @file tango_transform_helpers.h
/// @brief File containing transformation functions of Tango Support C API.
///   The Tango Support C API provides helper functions useful to external
///   developers for building Tango applications. This API is experimental and
///   subject to change.

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Multiplies a point by a matrix. No projective divide is done, the W
///   component is dropped. We explicitly support the case where point == out to
///   do an in-place transform.
///
/// @param matrix_transform The matrix the point is multiplied by.
/// @param point The original point.
/// @param out The ouput point.
/// @return @c TANGO_INVALID on invalid input; @c TANGO_SUCCESS otherwise.
TangoErrorType TangoTransformHelpers_doubleTransformPoint(
    const double matrix_transform[16], const double point[3], double out[3]);

/// @brief Multiplies a pose (represented as a position and a quaternion) by a
///   matrix. No projective divide is done, the W component is dropped. We
///   explicitly support the case where point == out to do an in-place
///   transform.
///
/// @param matrix_transform The matrix the pose is transformed by.
/// @param position The original pose's translation component.
/// @param quaternion The original pose's rotation component.
/// @param out_position The final pose's translation component.
/// @param out_quaternion The final pose's rotation component.
/// @return @c TANGO_INVALID on invalid input; @c TANGO_SUCCESS otherwise.
TangoErrorType TangoTransformHelpers_doubleTransformPose(
    const double matrix_transform[16], const double position[3],
    const double quaternion[4], double out_position[3],
    double out_quaternion[4]);

/// @brief Multiplies a point cloud (represented as a TangoPointCloud) by a
///   matrix. No projective divide is done, the W component is dropped. We
///   explicitly support the case where point == out to do an in-place
///   transform. The points in the output point cloud must be allocated before
///   calling this function.
///
/// @param matrix_transform The matrix all the points are transformed by.
/// @param point_cloud The original point cloud.
/// @param out The point cloud after translation.
/// @return @c TANGO_INVALID on invalid input; @c TANGO_SUCCESS otherwise.
TangoErrorType TangoTransformHelpers_doubleTransformPointCloud(
    const double matrix_transform[16], const TangoPointCloud* point_cloud,
    TangoPointCloud* out);

/// @brief Multiplies a point by a matrix. No projective divide is done, the W
///   component is dropped. We explicitly support the case where point == out to
///   do an in-place transform.
///
/// @param matrix_transform The matrix the point is multiplied by.
/// @param point The original point.
/// @param out The ouput point.
/// @return @c TANGO_INVALID on invalid input; @c TANGO_SUCCESS otherwise.
TangoErrorType TangoTransformHelpers_transformPoint(
    const float matrix_transform[16], const float point[3], float out[3]);

/// @brief Multiplies a pose (represented as a position and a quaternion) by a
///   matrix. No projective divide is done, the W component is dropped. We
///   explicitly support the case where point == out to do an in-place
///   transform.
///
/// @param matrix_transform The matrix the pose is transformed by.
/// @param position The original pose's translation component.
/// @param quaternion The original pose's rotation component.
/// @param out_position The final pose's translation component.
/// @param out_quaternion The final pose's rotation component.
/// @return @c TANGO_INVALID on invalid input; @c TANGO_SUCCESS otherwise.
TangoErrorType TangoTransformHelpers_transformPose(
    const float matrix_transform[16], const float position[3],
    const float quaternion[4], float out_position[3], float out_quaternion[4]);

/// @brief Multiplies a point cloud (represented as a TangoPointCloud) by a
///   matrix. No projective divide is done, the W component is dropped. We
///   explicitly support the case where point == out to do an in-place
///   transform. The points in the output point cloud must be allocated
///   before calling this function.
///
/// @param matrix_transform The matrix all the points are transformed by.
/// @param point_cloud The original point cloud.
/// @param out The point cloud after translation.
/// @return @c TANGO_INVALID on invalid input; @c TANGO_SUCCESS otherwise.
TangoErrorType TangoTransformHelpers_transformPointCloud(
    const float matrix_transform[16], const TangoPointCloud* point_cloud,
    TangoPointCloud* out);

/// @brief Transforms the point cloud into the same coordinate system as that of
///   the given pose, ignoring the value of pose->status_code.
///
/// @param point_cloud The point cloud to transform.
/// @param pose The pose with which to transform point_cloud.
/// @param transformed_point_cloud Replaced with the transformed point
///   cloud. The caller is expected to manage the memory appropriately by
///   preallocating and disposing of the storage space for the point data.
/// @return @c TANGO_SUCCESS on successful transform, @c TANGO_INVALID if the
///   parameters were null.
TangoErrorType TangoTransformHelpers_transformPointCloudWithPose(
    const TangoPointCloud* point_cloud, const TangoPoseData* pose,
    TangoPointCloud* transformed_point_cloud);

/// @brief Finds a similarity transformation (rotation, translation, and
///   scaling) given two sets of correspondence points. This uses the Umeyama
///   algorithm (http://www.cis.jhu.edu/software/lddmm-similitude/umeyama.pdf)
///   which minimizes the mean squared error. The returned transform is stored
///   in column-major order.
///
/// NOTE: If less than three non-collinear points are passed then this will
/// return one of the many possible transforms that make that correspondence.
///
/// @param src_points An array of 3D source points.
/// @param dest_points An array of 3D destination points.
/// @param num_points Number of correspondence points.
/// @param src_frame_T_dest_frame_matrix An array for output of the
///   transformation.
/// @return @c TANGO_SUCCESS on success, @c TANGO_INVALID on invalid input, and
///   @c TANGO_ERROR on failure.
TangoErrorType TangoTransformHelpers_findCorrespondenceSimilarityTransform(
    double (*src_points)[3], double (*dest_points)[3], int num_points,
    double src_frame_T_dest_frame_matrix[16]);

#ifdef __cplusplus
}
#endif

#endif  // TANGO_TRANSFORM_HELPERS_TANGO_TRANSFORM_HELPERS_H_
