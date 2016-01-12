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

#include <stdint.h>

/// @file tango_support_api.h
/// @brief File containing the Project Tango Support C API. The Project Tango
/// Support C API provides helper functions useful to external developers for
/// manipulating Project Tango data. The Project Tango Support C API is
/// experimental and subject to change.

#ifdef __cplusplus
extern "C" {
#endif

/// @defgroup CallbackHelpers Callback Data Support Functions
/// @brief Functions for managing data received from callbacks.
/// @{

/// The TangoSupportImageBufferManager maintains a set of image buffers to
/// manage transferring a TangoImageBuffer from the callback thread to a render
/// or computation thread. This holds three buffers internally (back, swap,
/// front). The back buffer is used as the destination for pixels copied from
/// the callback thread. When the copy is complete the back buffer is swapped
/// with the swap buffer while holding a lock. Calling
/// TangoSupport_getLatestImagebuffer holds the lock to swap the swap buffer
/// with the front buffer (if there is newer data in the swap buffer than the
/// current front buffer).
struct TangoSupportImageBufferManager;

/// @brief Create an object for maintaining a set of image buffers for a
/// specified image format and size.
///
/// @param format Format of the color camera image.
/// @param width Width in pixels of the color images.
/// @param height Height in pixels of the color images.
/// @param manager A handle to the manager object.
/// @return Returns <code>TANGO_SUCCESS</code> on successful allocation.
///   Returns <code>TANGO_INVALID</code> if manager is NULL.
TangoErrorType TangoSupport_createImageBufferManager(
    TangoImageFormatType format, int width, int height,
    TangoSupportImageBufferManager** manager);

/// @brief Delete the image buffer manager object.
///
/// @param manager A handle to the manager to delete.
/// @return Returns <code>TANGO_SUCCESS</code> on free.
TangoErrorType TangoSupport_freeImageBufferManager(
    TangoSupportImageBufferManager* manager);

/// @brief Limit copying of the incoming image to a specific range of
/// scan lines. This is an optimization when only a portion of the image is
/// required.
///
/// @param manager A handle to the image buffer manager.
/// @param y_only For YUV images copy only the Y-portion (grayscale
///   intensities) if value is non-zero. For zero value copy Y and UV portions.
/// @param begin_line Copy only scan lines start at this row. Must be less than
///   end_line.
/// @param end_line Copy only scan lines to this row. Must be greater than
///   begin_line.
/// @pre 0 <= begin_line <= end_line <= image_height - 1.
/// @return Returns <code>TANGO_SUCCESS</code> on update of the copy
///   region. Returns <code>TANGO_INVALID</code> if the preconditions are not
///   satisfied.
TangoErrorType TangoSupport_setImageBufferCopyRegion(
    TangoSupportImageBufferManager* manager, int y_only, uint32_t begin_line,
    uint32_t end_line);

/// @brief Updates the back buffer of the manager with new data from the
/// callback. This should be called from the image callback thread.
///
/// @param manager A handle to the image buffer manager.
/// @param image_buffer New image buffer data from the camera callback thread.
/// @return Returns <code>TANGO_SUCCESS</code> on update of the back image
/// buffer.
TangoErrorType TangoSupport_updateImageBuffer(
    TangoSupportImageBufferManager* manager,
    const TangoImageBuffer* image_buffer);

/// @brief Check if updated color image data is available. If so, swap new data
/// to the front buffer.  Set image_buffer to point to the front buffer. This
/// should be called from a single computation or render thread.
///
/// @param manager A handle to the image buffer manager.
/// @param image_buffer After the call contains a pointer to the most recent
///   camera image buffer.
/// @return Returns <code>TANGO_SUCCESS</code>.
TangoErrorType TangoSupport_getLatestImageBuffer(
    TangoSupportImageBufferManager* manager, TangoImageBuffer** image_buffer);

/// @}

/// @defgroup DepthSupport Depth Interface Support Functions
/// @brief Functions for managing depth data.
/// @{

/// @brief Initializes an empty point cloud with a buffer large enough to store
/// the specific maximum point cloud size. The logical number of vertices
/// (xyz_count) is initialized to zero.
///
/// @param max_point_cloud_size The maximum number of vertices in the point
///   cloud. This value should typically be retrieved from TangoConfig
///   max_point_cloud_elements.
/// @param point_cloud A pointer to the point cloud to be initialized. Cannot be
///   NULL.
/// @return Returns <code>TANGO_SUCCESS</code> on successful allocation.
///   Returns <code>TANGO_INVALID</code> if point_cloud is NULL.
TangoErrorType TangoSupport_createXYZij(uint32_t max_point_cloud_size,
                                        TangoXYZij* point_cloud);

/// @brief Deletes a point cloud.
///
/// @param point_cloud A pointer to the point cloud to be deleted. Cannot be
///   NULL.
/// @return Returns <code>TANGO_SUCCESS</code> on free.
///   Returns <code>TANGO_INVALID</code> if point_cloud is NULL.
TangoErrorType TangoSupport_freeXYZij(TangoXYZij* point_cloud);

/// @brief Performs a deep copy between two point clouds. The point clouds must
/// have been initialized with the same maximum size.
///
/// @param input_point_cloud The point cloud to be copied. Cannot be NULL.
/// @param output_point_cloud The output point cloud. Cannot be NULL.
/// @return Returns <code>TANGO_SUCCESS</code> on copy.
///   Returns <code>TANGO_INVALID</code> if input_point_cloud or
///   output_point_cloud is NULL.
TangoErrorType TangoSupport_copyXYZij(const TangoXYZij* input_point_cloud,
                                      TangoXYZij* output_point_cloud);

/// @brief Fits a plane to a point cloud near a user-specified location. This
/// occurs in two passes. First, all points are projected to the image plane
/// and only points near the user selection are kept. Then a plane is fit to
/// the subset using RANSAC. After the RANSAC fit, all inliers from the original
/// input point cloud are used to refine the plane model. The output is in the
/// coordinate system of the input point cloud.
///
/// @param point_cloud The input point cloud. Cannot be NULL and must have at
///   least three points.
/// @param camera_intrinsics The camera intrinsics for the color camera. Cannot
///   be NULL.
/// @param color_camera_T_point_cloud The pose of the point cloud relative to
///   the color camera used to obtain uv_coordinates.
/// @param uv_coordinates The UV coordinates for the user selection. This is
///   expected to be between (0.0, 0.0) and (1.0, 1.0). Cannot be NULL.
/// @param intersection_point The output point in point cloud coordinates the
///   user selected. Cannot be NULL.
/// @param plane_model The four parameters a, b, c, d for the general plane
///   equation ax + by + cz + d = 0 of the plane fit. The first three
///   components are a unit vector. The output is in the coordinate system of
///   the point cloud. Cannot be NULL.
/// @return <code>TANGO_SUCCESS</code> on success, <code>TANGO_INVALID</code> on
///   invalid input, and <code>TANGO_ERROR</code> on failure.
TangoErrorType TangoSupport_fitPlaneModelNearClick(
    const TangoXYZij* point_cloud,
    const TangoCameraIntrinsics* camera_intrinsics,
    const TangoPoseData* color_camera_T_point_cloud,
    const float uv_coordinates[2], double intersection_point[3],
    double plane_model[4]);

/// @brief Calculates the relative pose from the target frame at time
/// target_timestamp to the base frame at time base_timestamp.
///
/// @param base_timestamp The timestamp for base frame position. Must be
///   non-negative. If set to 0.0, the most recent pose estimate is used.
/// @param base_frame the coordinate frame type of target frame. Must be
///   TANGO_COORDINATE_FRAME_CAMERA_*.
/// @param target_timestamp The timestamp for target frame position. Must be
///   non-negative. If set to 0.0, the most recent pose estimate is used.
/// @param target_frame The coordinate frame type of base frame. Must be
///   TANGO_COORDINATE_FRAME_CAMERA_*.
/// @param base_frame_T_target_frame A TangoPoseData object with the calculated
///   orientation and translation. The output represents the transform from
///   target frame to base frame.
/// @return A TangoErrorType value of <code>TANGO_SUCCESS</code> on successful
///   calculation. Returns <code>TANGO_INVALID</code> if inputs are not
///   supported. Returns <code>TANGO_ERROR</code> if an internal transform
///   cannot be calculated.
TangoErrorType TangoSupport_calculateRelativePose(
    double base_timestamp, TangoCoordinateFrameType base_frame,
    double target_timestamp, TangoCoordinateFrameType target_frame,
    TangoPoseData* base_frame_T_target_frame);

/// The TangoSupportPointCloudManager maintains a set of point clouds to
/// manage transferring a TangoXYZij from the callback thread to a render
/// or computation thread. This holds three buffers internally (back, swap,
/// front). The back buffer is used as the destination for data copied from
/// the callback thread. When the copy is complete the back buffer is swapped
/// with the swap buffer while holding a lock. If there is newer data in
/// the swap buffer than the current front buffer then calling SwapBuffer holds
/// the lock and swaps the swap buffer with the front buffer.
struct TangoSupportPointCloudManager;

/// @brief Create an object for maintaining a set of point clouds for a
/// specified size.
///
/// @param max_points Maximum number of points in TangoXYZij. Get value from
///   config.
/// @param manager A handle to the manager object.
/// @return A TangoErrorType value of <code>TANGO_INVALID</code> if
///   max_points <= 0. Returns <code>TANGO_SUCCESS</code> otherwise.
TangoErrorType TangoSupport_createPointCloudManager(
    size_t max_points, TangoSupportPointCloudManager** manager);

/// @brief Delete the point cloud manager object.
///
/// @param manager A handle to the manager to delete.
/// @return A TangoErrorType value of <code>TANGO_SUCCESS</code> on free.
TangoErrorType TangoSupport_freePointCloudManager(
    TangoSupportPointCloudManager* manager);

/// @brief Updates the back buffer of the manager. Can be safely called from
///   the callback thread. Update is skipped if point cloud is empty.
///
/// @param manager A handle to the point cloud manager.
/// @param point_cloud New point cloud data from the camera callback thread.
/// @return A TangoErrorType value of <code>TANGO_INVALID</code> if manager
///   or point_cloud are NULL. Returns <code>TANGO_SUCCESS</code> if update
///   is successful.
TangoErrorType TangoSupport_updatePointCloud(
    TangoSupportPointCloudManager* manager, const TangoXYZij* point_cloud);

/// @brief Assign the pointer to the updated front buffer.
///   Can safely be called from the render thread. Update
///   only occurs if point_cloud timestamp is more recent.
///
/// @param manager A handle to the point cloud manager.
/// @param point_cloud After the call contains a pointer to the most recent
///   depth camera buffer.
/// @return A TangoErrorType value of <code>TANGO_INVALID</code> if manager
///   is NULL. Returns <code>TANGO_SUCCESS</code> otherwise.
TangoErrorType TangoSupport_getLatestPointCloud(
    TangoSupportPointCloudManager* manager, TangoXYZij** latest_point_cloud);

/**@} */

/// @defgroup SceneReconstructionSupport Scene Reconstruction Support Functions
/// @brief Functions for managing mesh data from scene reconstruction.
/// @{

/// @brief Initializes an empty mesh. No new memory is allocated.
///
/// @param mesh A pointer to the mesh to be initialized. Cannot be NULL.
/// @return Returns <code>TANGO_SUCCESS</code> on successful initialization.
///   Returns <code>TANGO_INVALID</code> if mesh is NULL.
TangoErrorType TangoSupport_initializeEmptyMesh(TangoMesh_Experimental* mesh);

/// @brief Deletes a mesh. Memory will be deallocated.
///
/// @param mesh A pointer to the mesh to be deleted. Cannot be NULL.
/// @return Returns <code>TANGO_SUCCESS</code> on successful free.
TangoErrorType TangoSupport_freeMesh(TangoMesh_Experimental* mesh);

/// @brief Creates a mesh, allocating memory for vertices, faces, and
/// (optionally) normals and colors.
///
/// @param num_vertices Number of mesh vertices to be allocated.
/// @param num_faces Number of mesh faces to be allocated.
/// @param has_normals If true, will allocate space for per-vertex mesh normals.
/// @param has_colors If true, will allocate space for per-vertex mesh colors.
/// @param mesh A pointer to the mesh to be created. Cannot be NULL.
/// @return Returns <code>TANGO_SUCCESS</code> on successful free.
///   Returns <code>TANGO_INVALID</code> if mesh is NULL.
TangoErrorType TangoSupport_createMesh(uint32_t num_vertices,
                                       uint32_t num_faces, bool has_normals,
                                       bool has_colors,
                                       TangoMesh_Experimental* mesh);

/// @brief Performs a deep copy between two meshes. Memory will be allocated
/// for the output mesh.
///
/// @param input_mesh The mesh to be copied. Cannot be NULL.
/// @param output_mesh The output mesh. Cannot be NULL.
/// @return Returns <code>TANGO_SUCCESS</code> on successful copy.
///   Returns <code>TANGO_INVALID</code> if input_mesh or output_mesh is
///   NULL.
TangoErrorType TangoSupport_copyMesh(const TangoMesh_Experimental* input_mesh,
                                     TangoMesh_Experimental* output_mesh);

/// @brief Creates a simplified mesh with a fewer number of faces, given a
/// source mesh. Memory will be allocated for the output mesh.
///
/// @param input_mesh The input mesh. Cannot be NULL.
/// @param target_num_faces Target number of faces in the output mesh.
/// @param output_mesh The output mesh. Cannot be NULL.
/// @return Returns <code>TANGO_SUCCESS</code> on successful copy. Returns
///   <code>TANGO_INVALID</code> if input_mesh or output_mesh is NULL.
TangoErrorType TangoSupport_createSimplifiedMesh(
    const TangoMesh_Experimental* input_mesh, const uint32_t target_num_faces,
    TangoMesh_Experimental* output_mesh);

/// @}

/// @defgroup TransformationSupport Transformation Support.
/// @brief Functions for supporting easy transformation between different
/// frames.
/// @{

/// @brief Coordinate conventions supported by the Tango Support API.
typedef enum {
  /// OpenGL coordinate convention.
  TANGO_SUPPORT_COORDINATE_CONVENTION_OPENGL,
  /// Unity3D coordinate convention.
  TANGO_SUPPORT_COORDINATE_CONVENTION_UNITY,
} TangoCoordinateConvention;

/// @brief Calculates the relative pose from the target frame at time
/// target_timestamp to the base frame at time base_timestamp.
///
/// @param base_timestamp The timestamp for base frame position. Must be
///   non-negative. If set to 0.0, the most recent pose estimate is used.
/// @param base_frame the coordinate frame type of target frame. Must be
///   TANGO_COORDINATE_FRAME_CAMERA_*.
/// @param target_timestamp The timestamp for target frame position. Must be
///   non-negative. If set to 0.0, the most recent pose estimate is used.
/// @param target_frame The coordinate frame type of base frame. Must be
///   TANGO_COORDINATE_FRAME_CAMERA_*.
/// @param base_frame_T_target_frame A TangoPoseData object with the calculated
///   orientation and translation. The output represents the transform from
///   target frame to base frame.
/// @return A TangoErrorType value of <code>TANGO_SUCCESS</code> on successful
///   calculation. Returns <code>TANGO_INVALID</code> if inputs are not
///   supported. Returns <code>TANGO_ERROR</code> if an internal transform
///   cannot be calculated.
TangoErrorType TangoSupport_calculateRelativePose(
    double base_timestamp, TangoCoordinateFrameType base_frame,
    double target_timestamp, TangoCoordinateFrameType target_frame,
    TangoPoseData* base_frame_T_target_frame);

/// @brief Convert device with respect to start of service pose into camera
/// with respect to world pose for supported platforms.
///
/// @param coordinate_convention The targeted coordinate convention.
/// @param tango_pose_data A pointer to the TangoPoseData going to be
///   converted. This must be for the frame pair (base frame: START_SERVICE,
///   target frame: DEVICE).
/// @param translation The camera with respect to world translation,
///   in the order of x, y, z.
/// @param orientation The camera with respect to world orientation,
///   in the order of x, y, z, w.
/// @return <code>TANGO_SUCCESS</code> on success and <code>TANGO_INVALID</code>
///   on invalid input.
TangoErrorType TangoSupport_getWorldTCameraPose(
    TangoCoordinateConvention coordinate_convention,
    const TangoPoseData* pose_start_service_T_device, double translation[3],
    double orientation[4]);

/// @}

/// @defgroup DepthInterpolationSupport Depth Interpolation Support Functions
/// @brief Functions for interpolating depth.
/// @{

/// @brief Calculates the depth in the color camera space at a user-specified
/// location using nearest-neighbor interpolation.
///
/// @param point_cloud The point cloud. Cannot be NULL and must have at least
///   one point.
/// @param camera_intrinsics The camera intrinsics for the color camera. Cannot
//    be NULL.
/// @param color_camera_T_point_cloud The pose of the point cloud relative to
///   the color camera used to obtain uv_coordinates.
/// @param uv_coordinates The UV coordinates for the user selection. This is
///   expected to be between (0.0, 0.0) and (1.0, 1.0). Cannot be NULL.
/// @param point_cloud_point The point (x, y, z), where (x, y) is the
///   back-projection of the UV coordinates to the color camera space and z is
//    the z coordinate of the point in the point cloud nearest to the user
//    selection after projection onto the image plane. If there is not a point
//    cloud point close to the user selection after projection onto the image
//    plane, then the point will be set to (0.0, 0.0, 0.0) and is_valid_point
///   will be set to 0.
/// @param is_valid_point A flag valued 1 if there is a point cloud point close
///   to the user selection after projection onto the image plane and valued 0
///   otherwise.
/// @return <code>TANGO_SUCCESS</code> on success and <code>TANGO_INVALID</code>
///   on invalid input.
TangoErrorType TangoSupport_getDepthAtPointNearestNeighbor(
    const TangoXYZij* point_cloud,
    const TangoCameraIntrinsics* camera_intrinsics,
    const TangoPoseData* color_camera_T_point_cloud,
    const float uv_coordinates[2], float point_cloud_point[3],
    int* is_valid_point);

/// @brief The TangoSupportDepthInterpolator contains references to camera
/// intrinsics
/// and cached data structures needed to upsample depth data to a camera image.
struct TangoSupportDepthInterpolator;

/// @brief Create an object for depth interpolation.
///
/// @param intrinsics The camera intrinsics for the camera to upsample.
/// @param interpolator A handle to the interpolator object.
/// @return A TangoErrorType value of <code>TANGO_INVALID</code> if intrinsics
///   is null. Returns <code>TANGO_SUCCESS</code> otherwise.
TangoErrorType TangoSupport_createDepthInterpolator(
    TangoCameraIntrinsics* intrinsics,
    TangoSupportDepthInterpolator** interpolator);

/// @brief Free the depth interpolation object.
///
/// @param A handle to the interpolator object.
/// @return <code>TANGO_SUCCESS</code>
TangoErrorType TangoSupport_freeDepthInterpolator(
    TangoSupportDepthInterpolator* interpolator);

/// @brief Calculates the depth in the color camera space at a user-specified
/// location using bilateral filtering weighted by both spatial distance from
/// the user coordinate and by intensity similarity.
///
/// @param interpolator A handle to the interpolator object. The intrinsics of
///   this interpolator object must match those of the image_buffer.
/// @param point_cloud The point cloud. Cannot be NULL and must have at least
///   one point.
/// @param image_buffer The RGB image buffer. This must have intrinsics matching
///   those used to create the interpolator object. Cannot be NULL.
/// @param color_camera_T_point_cloud The pose of the point cloud relative to
///   the color camera used to obtain uv_coordinates.
/// @param uv_coordinates The UV coordinates for the user selection. This is
///   expected to be between (0.0, 0.0) and (1.0, 1.0). Cannot be NULL.
/// @param point_cloud_point The point (x, y, z), where (x, y) is the
///   back-projection of the UV coordinates to the color camera space and z is
///   the bilateral interpolation of the z coordinate of the point. If there is
///   not a point cloud point close to the user selection after projection onto
///   the image plane, then the point will be set to (0.0, 0.0, 0.0) and
///   is_valid_point will be set to 0.
/// @param is_valid_point A flag valued 1 if there is a point cloud point close
///   to the user selection after projection onto the image plane and valued 0
///   otherwise.
/// @return <code>TANGO_SUCCESS</code> on success, <code>TANGO_INVALID</code> on
///   invalid input, and <code>TANGO_ERROR</code> on failure.
TangoErrorType TangoSupport_getDepthAtPointBilateral(
    const TangoSupportDepthInterpolator* interpolator,
    const TangoXYZij* point_cloud, const TangoImageBuffer* image_buffer,
    const TangoPoseData* color_camera_T_point_cloud,
    const float uv_coordinates[2], float point_cloud_point[3],
    int* is_valid_point);

/// @brief A structure to hold depth values for image upsampling. The units of
/// the depth are the same as for <code>TangoXYZij</code>.
struct TangoSupportDepthBuffer {
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
/// @return <code>TANGO_SUCCESS</code> on success, <code>TANGO_INVALID</code> on
///   invalid input, and <code>TANGO_ERROR</code> on failure.
TangoErrorType TangoSupport_initializeDepthBuffer(
    uint32_t width, uint32_t height, TangoSupportDepthBuffer* depth_buffer);

/// @brief Free memory for the depth buffer.
///
/// @param depth_buffer The depth buffer to free.
/// @return <code>TANGO_SUCCESS</code> on success, <code>TANGO_INVALID</code> on
///   invalid input, and <code>TANGO_ERROR</code> on failure.
TangoErrorType TangoSupport_freeDepthBuffer(
    TangoSupportDepthBuffer* depth_buffer);

/// @brief Upsamples the depth data to the resolution of the color image. This
/// uses the resolution specified by the intrinsics used to construct the
/// interpolator. This function fills depth around each sample using a fixed
/// radius. The resolution of the intrinsics provided to the interpolator and
/// the the resolution of the output depth_buffer must match.
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
/// @return <code>TANGO_SUCCESS</code> on success, <code>TANGO_INVALID</code> on
///   invalid input, and <code>TANGO_ERROR</code> on failure.
TangoErrorType TangoSupport_upsampleImageNearest(
    const TangoSupportDepthInterpolator* interpolator,
    const TangoXYZij* point_cloud,
    const TangoPoseData* color_camera_T_point_cloud,
    TangoSupportDepthBuffer* depth_buffer);

/// @brief Upsamples the depth data to the resolution of the color image. This
/// uses the resolution specified by the intrinsics used to construct the
/// interpolator. This function fills depth around using a bilateral filtering
/// approach. The resolution of the intrinsics provided to the interpolator and
/// the the resolution of the output depth_buffer must match.
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
/// @return <code>TANGO_SUCCESS</code> on success, <code>TANGO_INVALID</code> on
///   invalid input, and <code>TANGO_ERROR</code> on failure.
TangoErrorType TangoSupport_upsampleImageBilateral(
    const TangoSupportDepthInterpolator* interpolator, int approximate,
    const TangoXYZij* point_cloud, const TangoImageBuffer* image_buffer,
    const TangoPoseData* color_camera_T_point_cloud,
    TangoSupportDepthBuffer* depth_buffer);

/// @}

#ifdef __cplusplus
}
#endif

#endif  // TANGO_SUPPORT_API_H_
