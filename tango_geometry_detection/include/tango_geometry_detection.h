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

#ifndef TANGO_GEOMETRY_DETECTION_TANGO_GEOMETRY_DETECTION_H_
#define TANGO_GEOMETRY_DETECTION_TANGO_GEOMETRY_DETECTION_H_

#include <tango_client_api.h>
#include <tango_support.h>

/// @file tango_geometry_detection.h
/// @brief File containing feature detection functions of Tango Support C API.
///   The Tango Support C API provides helper functions useful to external
///   developers for building Tango applications. This API is experimental and
///   subject to change.

#ifdef __cplusplus
extern "C" {
#endif

/// @defgroup EdgeDetectionSupport Edge Detection Support Functions
/// @brief Functions for detecting edges.
/// @{

/// @brief A structure to define an edge, including the closest point on the
/// edge to the input point to the method that returned the edge.
struct TangoGeometryDetection_Edge {
  float end_points[2][3];
  float closest_point_on_edge[3];
};

/// @brief Find the list of edges "close" to the user-specified location and
///   that are on the plane estimated from the input location. The edges are
///   detected in the color camera image and are output in the base frame
///   of the input translations and rotations. This output frame is usually an
///   application's world frame.
///
/// @param point_cloud The point cloud. Cannot be NULL and must have sufficient
///   points to estimate the plane at the location of the input.
/// @param point_cloud_translation The translation component of the
///   transformation from the point cloud to the output frame. Cannot be NULL.
/// @param point_cloud_orientation The orientation component (as a quaternion)
///   of the transformation from the point cloud to the output frame.
///   Cannot be NULL.
/// @param image_buffer The RGB image buffer. Although accuracy will be
///   reduced, a down-sampled image can be used to improve performance.
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
/// @param edges An array of 3D edges close to the input point and specified in
///   the requested output frame. The edges will lie on the plane estimated at
///   the location of the input point. The array should be deleted by calling
///   TangoGeometryDetection_freeEdgeList. Cannot be NULL.
/// @param number_of_edges The number of edges in @p edges. Cannot be NULL.
/// @return @c TANGO_SUCCESS on success, @c TANGO_INVALID on invalid input, and
///   @c TANGO_ERROR on failure.
TangoErrorType TangoGeometryDetection_findEdgesNearPoint(
    const TangoPointCloud* point_cloud, const double point_cloud_translation[3],
    const double point_cloud_orientation[4],
    const TangoImageBuffer* image_buffer,
    const float color_camera_uv_coordinates[2],
    TangoSupport_Rotation display_rotation,
    const double color_camera_translation[3],
    const double color_camera_orientation[4],
    TangoGeometryDetection_Edge** edges, int* number_of_edges);

/// @brief Free memory allocated in call to
///   TangoGeometryDetection_findEdgesNearPoint.
///
/// @param edges Edge list to free.
/// @return @c TANGO_SUCCESS on success.
TangoErrorType TangoGeometryDetection_freeEdgeList(
    TangoGeometryDetection_Edge** edges);

/// @}

/// @defgroup VolumeSupport Volume Measurement Support Functions
/// @brief Functions for measuring volumes.
/// @{

/// struct for storing a Tango volume object
struct TangoGeometryDetection_Volume;

/// @brief Create a volume measurement object.
/// @return Pointer to a volume measurement object.
TangoGeometryDetection_Volume* TangoGeometryDetection_createVolume();

/// @brief Delete a volume measurement created by
///   TangoGeometryDetection_createVolume().
/// @param volume Pointer to the volume object to be destroyed.
void TangoGeometryDetection_deleteVolume(
    TangoGeometryDetection_Volume** volume);

/// @brief Add the reference plane for the volume measurement. The reference
///   plane is a surface the volume object is on. This routine must be called
///   before calling TangoGeometryDetection_addSeedPointToVolume() or
///   TangoGeometryDetection_addPointCloudToVolume() methods.
///
/// NOTE: This function expects a transformation from the input frame to the
/// output frame. The output frame has to be a right-hand 3D world frame and
/// the same output frame should be used throughout the calls to "this"
/// volume measurement.
///
/// @param volume Pointer to the volume measurement object.
/// @param plane_model Plane model specified in the input frame.
/// @param translation Translation component of the transformation from the
///   input frame to the output frame.
/// @param orientation Rotation component (as a quaternion) of the
///   transformation from the input frame to the output frame.
/// @return @c TANGO_SUCCESS on success and @c TANGO_INVALID on invalid input.
TangoErrorType TangoGeometryDetection_addReferencePlaneToVolume(
    TangoGeometryDetection_Volume* volume, const double plane_model[4],
    const double translation[3], const double orientation[4]);

/// @brief Add a seed point to the volume measurement. Seed points are used as
///   starting locations when searching point clouds for volume object
///   boundaries. A seed point by itself is also used to define the volume
///   boundary. The reference plane of the volume must be specified by
///   TangoGeometryDetection_addReferencePlaneToVolume() method before calling
///   this routine.
///
/// NOTE: This function expects a transformation from the input frame to the
/// output frame. The output frame has to be a right-hand 3D world frame and
/// the same output frame should be used throughout the calls to "this"
/// volume measurement.
///
/// @param volume Pointer to the volume measurement object.
/// @param seed_point The seed point specified in the input frame.
/// @param translation Translation component of the transformation from the
///   input frame to the output frame.
/// @param orientation Rotation component (as a quaternion) of the
///   transformation from the input frame to the output frame.
/// @return @c TANGO_SUCCESS on success, @c TANGO_INVALID on invalid input, and
///   @c TANGO_ERROR on failure.
TangoErrorType TangoGeometryDetection_addSeedPointToVolume(
    TangoGeometryDetection_Volume* volume, const float seed_point[3],
    const double translation[3], const double orientation[4]);

/// @brief Add a point cloud to the volume measurement. Point clouds are used
///   to grow the volume from input seed points to the boundary of the object.
///   This routine should be called after at least one call to
///   TangoSupport_addSeedPointCloudToVolume() method.
///
/// NOTE: This function expects a transformation from the input frame to the
/// output frame. The output frame has to be a right-hand 3D world frame and
/// the same output frame should be used throughout the calls to "this"
/// volume measurement.
///
/// @param volume Pointer to the volume measurement object.
/// @param point_cloud Point cloud specified in the input frame.
/// @param translation Translation component of the transformation from the
///   input frame to the output frame.
/// @param orientation Rotation component (as a quaternion) of the
///   transformation from the input frame to the output frame.
/// @return @c TANGO_SUCCESS on success, @c TANGO_INVALID on invalid input, and
///   @c TANGO_ERROR on failure.
TangoErrorType TangoGeometryDetection_addPointCloudToVolume(
    TangoGeometryDetection_Volume* volume, const TangoPointCloud* point_cloud,
    const double translation[3], const double orientation[4]);

/// @brief Get the current volume measurement information, including both the
///   volume size and the 8 corner points of the volume.
///
/// NOTE: The volume_points are in the output frame specified by previous call
/// of TangoGeometryDetection_addReferencePlaneToVolume(),
/// TangoGeometryDetection_addSeedPointToVolume() and
/// TangoGeometryDetection_addPointCloudToVolume() methods. It is important that
//  the output frame specified in those routines is consistent.
///
/// @param volume Pointer to the volume measurement object.
/// @param volume_size The size of the volume, in mm^3.
/// @param volume_points Array of points defining the oriented bounding-box of
///   the detected volume in the output frame.
/// @return @c TANGO_SUCCESS on success, @c TANGO_INVALID on invalid input, and
///   @c TANGO_ERROR on failure.
TangoErrorType TangoGeometryDetection_getVolumeOutput(
    TangoGeometryDetection_Volume* volume, float* volume_size,
    float volume_points[8][3]);

/// @}

/// @defgroup CornerDetectionSupport Corner Detection Support Functions
/// @brief Functions for detecting corners
/// @{

/// @brief A structure to define a corner, including the corner point, and
///   indices to edges that are associated to the corner. The edges are input
///   parameters to TangoGeometryDetection_detectCorners() routine.
struct TangoGeometryDetection_Corner {
  float corner_point[3];
  float distance_to_poi;
  int* edges;
  int edge_count;
};

/// @brief A structure that stores a list of corners. After calling
///   TangoGeometryDetection_detectCorners() with a
///   TangoGeometryDetection_CornerList object, the object needs to be released
///   by calling TangoGeometryDetection_freeCornerList() function.
struct TangoGeometryDetection_CornerList {
  TangoGeometryDetection_Corner* corners;
  int corner_count;
};

/// @brief Detect corners among a list of edges.
///
/// @param point_of_interest The user-specified location.
/// @param edges A list of edges, usually returned by
///   TangoGeometryDetection_findEdgesNearPoint() routine.
/// @param number_of_edges The number of edges in point_of_interest list.
/// @param corner_list The structure to hold result corners. The structure
///   should be deleted by calling TangoGeometryDetection_freeCornerList. Cannot
///   be NULL.
/// @return @c TANGO_SUCCESS on success, @c TANGO_INVALID on invalid input, and
///   @c TANGO_ERROR on failure.
TangoErrorType TangoGeometryDetection_detectCorners(
    const float point_of_interest[3], const TangoGeometryDetection_Edge** edges,
    const int number_of_edges, TangoGeometryDetection_CornerList* corner_list);

/// @brief Free memory allocated in TangoGeometryDetection_detectCorners.
///
/// @param Corner list to free.
/// @return @c TANGO_SUCCESS on success.
TangoErrorType TangoGeometryDetection_freeCornerList(
    TangoGeometryDetection_CornerList* corner_list);

/// @}

#ifdef __cplusplus
}
#endif

#endif  // TANGO_GEOMETRY_DETECTION_TANGO_GEOMETRY_DETECTION_H_
