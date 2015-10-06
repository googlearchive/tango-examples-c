// Copyright 2015 Google Inc. All Rights Reserved.
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

#ifndef TANGO_SUPPORT_API_H_
#define TANGO_SUPPORT_API_H_

#include <tango_client_api.h>

/// @file tango_support_api.h
/// @brief File containing the Project Tango Support C API. The Project Tango
/// Support C API provides helper functions useful to external developers for
/// manipulating Project Tango data. The Project Tango Support C API is
/// experimental and subject to change.

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Initializes an empty point cloud with a buffer large enough to store
/// the specific maximum point cloud size. The logical number of vertices
/// (xyz_count) is initialized to zero.
///
/// @param max_point_cloud_size The maximum number of vertices in the point
///   cloud.  This value should typically be retrieved from TangoConfig
///   max_point_cloud_elements.
/// @param point_cloud A pointer to the point cloud to be initialized. Cannot be
///   nullptr.
/// @return Returns <code>TANGO_SUCCESS</code> on successful allocation.
///   Returns <code>TANGO_INVALID</code> if point_cloud is nullptr.
TangoErrorType TangoSupport_createXYZij(uint32_t max_size,
                                        TangoXYZij* point_cloud);

/// @brief Deletes a point cloud.
///
/// @param point_cloud A pointer to the point cloud to be deleted. Cannot be
/// nullptr.
/// @return Returns <code>TANGO_SUCCESS</code> on free.
///   Returns <code>TANGO_INVALID</code> if point_cloud is nullptr.
TangoErrorType TangoSupport_freeXYZij(TangoXYZij* point_cloud);

/// @brief Performs a deep copy between two point clouds.  The point clouds
/// must have been initialized with the same maximum size.
///
/// @param input_mesh The mesh to be copied. Cannot be nullptr.
/// @param output_mesh The output mesh. Cannot be nullptr.
/// @return Returns <code>TANGO_SUCCESS</code> on copy.
///   Returns <code>TANGO_INVALID</code> if input_point_cloud or
///   output_point_cloud is nullptr.
TangoErrorType TangoSupport_copyXYZij(const TangoXYZij* input_point_cloud,
                                      TangoXYZij* output_point_cloud);

/// @brief Fits a plane to a point cloud near a user-specified location. This
/// occurs in two passes. First, all points are projected to the image plane and
/// only points near the user selection are kept.  Then a plane is fit to the
/// subset using RANSAC. After the RANASC fit all inliers from the original
/// input point cloud are used to refine the plane model.
///
/// @param cloud The input point cloud. Cannot be nullptr and must have at least
///   three points.
/// @param intrinsics The camera intrinsics for the color camera.
/// @param uv_coordinates The UV coordinates for the user selection. This is
///   expected to be between (0.0, 0.0) and (1.0, 1.0). Cannot be nullptr.
/// @param intersection_point The output point in depth camera coordinates the
/// user selected. Cannot be nullptr.
/// @param plane_model The four parameters a, b, c, d for the general plane
///   equation ax + by + cz + d = 0 of the plane fit. The first three components
///   are a unit vector. Cannot be nullptr.
/// @return <code>TANGO_SUCCESS</code> on success, <code>TANGO_INVALID</code> on
///   invalid input, and <code>TANGO_ERROR</code> on failure.
TangoErrorType TangoSupport_fitPlaneModelNearClick(
    const TangoXYZij* cloud, TangoCameraIntrinsics intrinsics,
    const float uv_coordinates[2], double intersection_point[3],
    double plane_model[4]);

/// @brief Initializes an empty mesh. No new memory is allocated.
///
/// @param mesh A pointer to the mesh to be initialized. Cannot be nullptr.
/// @return Returns <code>TANGO_SUCCESS</code> on successful initialization.
///   Returns <code>TANGO_INVALID</code> if mesh is nullptr.
TangoErrorType TangoSupport_initializeEmptyMesh(TangoMesh_Experimental* mesh);

/// @brief Deletes a mesh. Memory will be deallocated.
///
/// @param mesh A pointer to the mesh to be deleted. Cannot be nullptr.
/// @return Returns <code>TANGO_SUCCESS</code> on successful free.
///   Returns <code>TANGO_INVALID</code> if mesh is nullptr.
TangoErrorType TangoSupport_freeMesh(TangoMesh_Experimental* mesh);

/// @brief Creates a mesh, allocating memory for vertices, faces, and
/// (optionally) normals and colors.
///
/// @param num_vertices Number of mesh vertices to be allocated.
/// @param num_faces Number of mesh faces to be allocated.
/// @param has_normals If true, will allocate space for per-vertex mesh normals.
/// @param has_colors If true, will allocate space for per-vertex mesh colors.
/// @param mesh A pointer to the mesh to be created. Cannot be nullptr.
/// @return Returns <code>TANGO_SUCCESS</code> on successful free.
///   Returns <code>TANGO_INVALID</code> if mesh is nullptr.
TangoErrorType TangoSupport_createMesh(uint32_t num_vertices,
                                       uint32_t num_faces, bool has_normals,
                                       bool has_colors,
                                       TangoMesh_Experimental* mesh);

/// @brief Performs a deep copy between two meshes. Memory will be allocated
/// for the output mesh.
///
/// @param input_mesh The mesh to be copied. Cannot be nullptr.
/// @param output_mesh The output mesh. Cannot be nullptr.
/// @return Returns <code>TANGO_SUCCESS</code> on successful copy.
///   Returns <code>TANGO_INVALID</code> if input_mesh or output_mesh is
///   nullptr.
TangoErrorType TangoSupport_copyMesh(const TangoMesh_Experimental* input_mesh,
                                     TangoMesh_Experimental* output_mesh);

/// @brief Creates a simplified mesh with a fewer number of faces, given a
/// source mesh. Memory will be allocated for the output mesh.
///
/// @param input_mesh The input mesh. Cannot be nullptr.
/// @param target_num_faces Target number of faces in the output mesh.
/// @param output_mesh The output mesh. Cannot be nullptr.
/// @return Returns <code>TANGO_SUCCESS</code> on successful copy.
///   Returns <code>TANGO_INVALID</code> if input_mesh or output_mesh is
///   nullptr.
TangoErrorType TangoSupport_createSimplifiedMesh(
    const TangoMesh_Experimental* input_mesh, const uint32_t target_num_faces,
    TangoMesh_Experimental* output_mesh);

#ifdef __cplusplus
}
#endif

#endif  // TANGO_SUPPORT_API_H_
