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

#ifndef TANGO_SUPPORT_TANGO_SUPPORT_H_
#define TANGO_SUPPORT_TANGO_SUPPORT_H_

#include <tango_client_api.h>

#include <stdint.h>

/// @file tango_support.h
/// @brief File containing the core functions of Tango Support C API. The Tango
///   Support C API provides helper functions useful to external developers for
///   building Tango applications.

#ifdef __cplusplus
extern "C" {
#endif

/// @defgroup MiscUtilities Miscellaneous Utility Functions
/// @brief Functions that do not fit into any other group.
/// @{

/// Display orientation. This is a relative orientation between
/// default (natural) screen orientation and current screen orientation.
/// The orientation is calculated based on a clockwise rotation.
///
/// The index number mirrors Android display rotation constant. This
/// allows the developer to directly pass in the index value returned from
/// Android.
typedef enum {
  /// Not apply any rotation.
  TANGO_SUPPORT_ROTATION_IGNORED = -1,

  /// 0 degree rotation (natural orientation)
  TANGO_SUPPORT_ROTATION_0 = 0,

  /// 90 degree rotation.
  TANGO_SUPPORT_ROTATION_90 = 1,

  /// 180 degree rotation.
  TANGO_SUPPORT_ROTATION_180 = 2,

  /// 270 degree rotation.
  TANGO_SUPPORT_ROTATION_270 = 3
} TangoSupport_Rotation;

/// @brief Get the version code of the installed TangoCore package.
/// @param jni_env A pointer to the JNI Context of the native activity. This
///   must be of type JNIEnv*, and implicit type conversion should do the right
///   thing without requiring a cast.
/// @param activity The native activity object handle of the calling native
///   activity.  This should be of type jobject, and implicit type conversion
///   should do the right thing without requiring a cast.
/// @param version Filled out with the version of the installed
///   TangoCore package or 0 if it is not installed.
/// @return @c TANGO_SUCCESS if the version was able to be successfully found.
///   @c TANGO_ERROR if some other error happened.
TangoErrorType TangoSupport_getTangoVersion(void* jni_env, void* activity,
                                            int* version);

/// @brief Typedef for getPostAtTime function signature; required by the
/// @c TangoSupport_initialize method.
typedef TangoErrorType (*TangoSupport_GetPoseAtTimeFn)(
    double timestamp, TangoCoordinateFramePair frame, TangoPoseData* pose);

/// @brief Typedef for getCameraIntrinsics function signature; required by the
///   @c TangoSupport_initialize method.
typedef TangoErrorType (*TangoSupport_GetCameraIntrinsicsFn)(
    TangoCameraId camera_id, TangoCameraIntrinsics* intrinsics);

/// @brief Initialize the support library with function pointers required by
///   the library.
///   NOTE: This function must be called after the Android service has been
///   bound.
///
/// @param getPoseAtTime The function to call to retrieve device pose
///   information. In practice this is TangoService_getPoseAtTime, except
///   for testing.
/// @param getCameraIntrinsics The function to call to retrieve camera
///   intrinsics information. In practice this is
///   TangoService_getCameraIntrinsics, except for testing.
void TangoSupport_initialize(
    TangoSupport_GetPoseAtTimeFn getPoseAtTime,
    TangoSupport_GetCameraIntrinsicsFn getCameraIntrinsics);

/// @brief Get the video overlay UV coordinates based on the display rotation.
///   Given the UV coordinates belonging to a display rotation that
///   matches the camera rotation, this function will return the UV coordinates
///   for any display rotation.
/// @param uv_coordinates The UV coordinates corresponding to a display
///   rotation that matches the camera rotation. Cannot be NULL.
/// @param display_rotation The index of the display rotation between
///   display's default (natural) orientation and current orientation.
/// @param output_uv_coordinates The UV coordinates for this
///   rotation.
/// @return @c TANGO_SUCCESS on success,  @c TANGO_ERROR if the support library
///   was not previously initialized, or @c TANGO_INVALID on invalid input.
TangoErrorType TangoSupport_getVideoOverlayUVBasedOnDisplayRotation(
    const float uv_coordinates[8], TangoSupport_Rotation display_rotation,
    float output_uv_coordinates[8]);

/// @brief Get the camera intrinsics based on the display rotation. This
///   function will query the camera intrinsics and rotate them according to
///   the display rotation.
/// @param camera_id The camera ID to retrieve the calibration intrinsics for.
/// @param display_rotation The index of the display rotation between
///   display's default (natural) orientation and current orientation.
/// @param intrinsics A TangoCameraIntrinsics struct that must be allocated
///   before calling, and is filled with camera intrinsics for the rotated
///   camera @p camera_id upon successful return.
/// @return @c TANGO_SUCCESS on success,  @c TANGO_ERROR if the support library
///   was not previously initialized, or @c TANGO_INVALID on invalid input.
TangoErrorType TangoSupport_getCameraIntrinsicsBasedOnDisplayRotation(
    TangoCameraId camera_id, TangoSupport_Rotation display_rotation,
    TangoCameraIntrinsics* intrinsics);

/// @brief Returns the depth camera rotation in TangoSupport_Rotation format.
/// @return Depth camera rotation.
TangoSupport_Rotation TangoSupport_getAndroidDepthCameraRotation();

/// @brief Returns the depth camera rotation in TangoSupport_Rotation format.
/// @return Color camera rotation.
TangoSupport_Rotation TangoSupport_getAndroidColorCameraRotation();

/// @brief Indicates whether a given rotation is valid or not.
/// @param rotation The input rotation to be verified.
/// @param allowIgnored Allow TANGO_SUPPORT_ROTATION_IGNORED.
/// @return True if the given rotation is valid, false if it is not.
bool TangoSupport_isValidRotation(TangoSupport_Rotation rotation,
                                  bool allowIgnored);

/// @}

/// @defgroup CallbackHelpers Callback Data Support Functions
/// @brief Functions for managing data received from callbacks.
/// @{

/// The TangoSupport_ImageBufferManager maintains a set of image buffers to
/// manage transferring a TangoImageBuffer from the callback thread to a render
/// or computation thread. This holds three buffers internally (back, swap,
/// front). The back buffer is used as the destination for pixels copied from
/// the callback thread. When the copy is complete the back buffer is swapped
/// with the swap buffer while holding a lock. Calling
/// @c TangoSupport_getLatestImagebuffer holds the lock to exchange the swap
/// buffer with the front buffer (if there is newer data in the swap buffer
/// than the current front buffer).
struct TangoSupport_ImageBufferManager;

/// @brief Create an object for maintaining a set of image buffers for a
///   specified image format and size.
///
/// @param format The format of the color camera image.
/// @param width The width in pixels of the color images.
/// @param height The height in pixels of the color images.
/// @param manager A handle to the manager object.
/// @return @c TANGO_SUCCESS if allocation was successful; @c TANGO_INVALID if
///   manager is NULL.
TangoErrorType TangoSupport_createImageBufferManager(
    TangoImageFormatType format, int width, int height,
    TangoSupport_ImageBufferManager** manager);

/// @brief Delete the image buffer manager object.
///
/// @param manager A handle to the manager to delete.
/// @return @c TANGO_SUCCESS if memory was freed successfully; @c TANGO_INVALID
///   otherwise.
TangoErrorType TangoSupport_freeImageBufferManager(
    TangoSupport_ImageBufferManager* manager);

/// @brief Limit copying of the incoming image to a specific range of
///   scan lines.
///
/// This is an optimization when only a portion of the image is
/// required. For the @p begin_line and @p end_line parameters, the
/// following must be true:
///
/// 0 <= @p begin_line <= @p end_line <= (image_height - 1)
///
/// @param manager A handle to the image buffer manager.
/// @param y_only If non-zero and the image is YUV, copy only the Y-portion
///   (grayscale intensities) of the image. If zero, copy Y and UV portions.
/// @param begin_line The first scan line row to copy. Must be less than
///   end_line.
/// @param end_line The last scan line row to copy. Must be greater than
///   begin_line and smaller than the image height.
/// @return @c TANGO_SUCCESS if copy region was updated successfully, or
///   @c TANGO_INVALID if the preconditions are not satisfied.
TangoErrorType TangoSupport_setImageBufferCopyRegion(
    TangoSupport_ImageBufferManager* manager, int y_only, uint32_t begin_line,
    uint32_t end_line);

/// @brief Updates the back buffer of the manager with new data from the
///   callback. This should be called from the image callback thread.
///
/// @param manager A handle to the image buffer manager.
/// @param image_buffer New image buffer data from the camera callback thread.
/// @return Returns @c TANGO_SUCCESS on update of the back image buffer. Returns
///   @c TANGO_INVALID otherwise.
TangoErrorType TangoSupport_updateImageBuffer(
    TangoSupport_ImageBufferManager* manager,
    const TangoImageBuffer* image_buffer);

/// @brief Check if updated color image data is available. If so, swap new data
///   to the front buffer and set image_buffer to point to the front buffer.
///   Multiple calls to this function must be made from the same thread. Set
///   new_data to true when image_buffer points to new data.
///
/// @param manager A handle to the image buffer manager.
/// @param image_buffer After the call contains a pointer to the most recent
///   camera image buffer.
/// @param new_data True if latest_point_cloud points to new data. False
///   otherwise.
/// @return Returns @c TANGO_SUCCESS if params are valid. Returns
///   @c TANGO_INVALID otherwise.
TangoErrorType TangoSupport_getLatestImageBufferAndNewDataFlag(
    TangoSupport_ImageBufferManager* manager, TangoImageBuffer** image_buffer,
    bool* new_data);

/// @brief Check if updated color image data is available. If so, swap new data
///   to the front buffer and set image_buffer to point to the front buffer.
///   Multiple calls to this function must be made from the same thread.
///
/// @param manager A handle to the image buffer manager.
/// @param image_buffer After the call contains a pointer to the most recent
///   camera image buffer.
/// @return Returns @c TANGO_SUCCESS if params are valid. Returns
///   @c TANGO_INVALID otherwise.
TangoErrorType TangoSupport_getLatestImageBuffer(
    TangoSupport_ImageBufferManager* manager, TangoImageBuffer** image_buffer);

/// @}

/// @defgroup TransformationSupport Transformation Support
/// @brief Functions for supporting easy transformation between different
///   frames.
/// @{

/// Enumeration of support engines. Every engine conversion
/// corresponds to an axis swap from the Tango-native frame
typedef enum {
  /// Tango native frame, has a different convention
  /// for forward, right, and up
  /// for each reference frame.
  /// Right-handed coordinate system.
  TANGO_SUPPORT_ENGINE_TANGO,

  /// OpenGL frame, -Z forward, X right, Y up.
  /// Right-handed coordinate system.
  TANGO_SUPPORT_ENGINE_OPENGL,

  /// Unity frame, +Z forward, X, right, Y up.
  /// Left-handed coordinate system.
  TANGO_SUPPORT_ENGINE_UNITY,

  /// UnrealEngine frame, X forward, Y right, Z up.
  /// Left-handed coordinate system.
  TANGO_SUPPORT_ENGINE_UNREAL,

  /// etc.
} TangoSupport_EngineType;

/// @brief Struct to hold transformation float matrix and its metadata.
typedef struct TangoSupport_MatrixTransformData {
  /// Timestamp of the time that this pose estimate corresponds to.
  double timestamp;

  /// Matrix in column major order.
  float matrix[16];

  /// The status of the pose, according to the pose lifecycle.
  TangoPoseStatusType status_code;
} TangoSupport_MatrixTransformData;

/// @brief Struct to hold transformation double matrix and its metadata.
typedef struct TangoSupport_DoubleMatrixTransformData {
  /// Timestamp of the time that this pose estimate corresponds to.
  double timestamp;

  /// Matrix in column major order.
  double matrix[16];

  /// The status of the pose, according to the pose lifecycle.
  TangoPoseStatusType status_code;
} TangoSupport_DoubleMatrixTransformData;

/// @brief Calculates the relative pose of the target frame at time
///   target_timestamp with respect to the base frame at time base_timestamp.
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
/// @return A TangoErrorType value of @c TANGO_SUCCESS on successful
///   calculation, @c TANGO_INVALID if inputs are not supported, or
///   @c TANGO_ERROR if an internal transform cannot be calculated.
TangoErrorType TangoSupport_calculateRelativePose(
    double base_timestamp, TangoCoordinateFrameType base_frame,
    double target_timestamp, TangoCoordinateFrameType target_frame,
    TangoPoseData* base_frame_T_target_frame);

/// @brief Get a pose at a given timestamp from the base to the target frame
///   using the specified target and base engine's coordinate system
///   conventions. The base and target engine must either both be right-handed
///   systems or both be left-handed systems.
///
/// When using engine OpenGL, Unity or Unreal, this function applies the
/// corresponding rotation to base and target frames based on the display
/// rotation. When using engine Tango no rotation is applied to that frame.
///
/// @param timestamp Time specified in seconds. This behaves the same as the
///   @p timestamp parameter in @c TangoService_getPoseAtTime.
/// @param base_frame The base frame of reference to use in the query.
/// @param target_frame The target frame of reference to use in the query.
/// @param base_engine The coordinate system convention of the @p base_frame.
///   Can be OpenGL, Unity, Unreal or Tango but the handed-ness (either
///   left-handed or right-handed) must match the handed-ness of the
///   @p target_engine.
/// @param target_engine The coordinate system convention of the @p
///   target_frame. Can be OpenGL, Unity, Unreal or Tango but the handed-ness
///   (either left-handed or right-handed) must match the handed-ness of the
///   @p base_engine.
/// @param display_rotation The index of the display rotation between
///   display's default (natural) orientation and current orientation.
/// @param pose The pose of target with respect to base frame of reference,
///   accounting for the specified engine and display rotation.
/// @return @c TANGO_SUCCESS on success, @c TANGO_INVALID on invalid input,
///   including mismatched handed-ness of the @p base_engine and @p
///   target_engine, and @c TANGO_ERROR on failure.
///
/// @details The engine types should be set to TANGO_SUPPORT_ENGINE_OPENGL when
///   the target (color camera or device) is used to control a virtual camera
///   for rendering purposes. Typically this is done by using the matrix derived
///   from the result pose as the view matrix of the virtual camera. The target
///   frame engine type should be set to TANGO_SUPPORT_ENGINE_TANGO when
///   computing a transformation for parameters used for calling other Tango
///   support routines.
TangoErrorType TangoSupport_getPoseAtTime(
    double timestamp, TangoCoordinateFrameType base_frame,
    TangoCoordinateFrameType target_frame, TangoSupport_EngineType base_engine,
    TangoSupport_EngineType target_engine,
    TangoSupport_Rotation display_rotation, TangoPoseData* pose);

/// @brief Calculate the tranformation matrix between specified frames and
///   engine types. The output matrix uses floats and is in column-major order.
///
/// When using engine OpenGL, Unity or Unreal, this function applies the
/// corresponding rotation to base and target frames based on the display
/// rotation. When using engine Tango no rotation is applied to that frame.
///
/// @param timestamp The timestamp of the transformation matrix of interest.
/// @param base_frame The frame of reference the matrix converts to.
/// @param target_frame The frame of reference the matrix converts from.
/// @param base_engine Specifies the final orientation convention the matrix
///   converts to.
/// @param target_engine Specifies the original orientation convention the
///   matrix converts from.
/// @param display_rotation The index of the display rotation between
///   display's default (natural) orientation and current orientation.
/// @param matrix_transform The final matrix output with metadata.
/// @return @c TANGO_INVALID on invalid input. @c TANGO_ERROR if the
///   pose calculation returns error. @c TANGO_SUCCESS otherwise.
TangoErrorType TangoSupport_getMatrixTransformAtTime(
    double timestamp, TangoCoordinateFrameType base_frame,
    TangoCoordinateFrameType target_frame, TangoSupport_EngineType base_engine,
    TangoSupport_EngineType target_engine,
    TangoSupport_Rotation display_rotation,
    TangoSupport_MatrixTransformData* matrix_transform);

/// @brief Calculate the tranformation matrix between specified frames and
///   engine types. The output matrix uses doubles and is in column-major order.
///
/// When using engine OpenGL, Unity or Unreal, this function applies the
/// corresponding rotation to base and target frames based on the display
/// rotation. When using engine Tango no rotation is applied to that frame.
///
/// @param timestamp The timestamp of the transformation matrix of interest.
/// @param base_frame The frame of reference the matrix converts to.
/// @param target_frame The frame of reference the matrix converts from.
/// @param base_engine Specifies the final orientation convention the matrix
///   converts to.
/// @param target_engine Specifies the original orientation convention the
///   matrix converts from.
/// @param display_rotation The index of the display rotation between
///   display's default (natural) orientation and current orientation.
/// @param matrix_transform The final matrix output with metadata.
/// @return @c TANGO_INVALID on invalid input. @c TANGO_ERROR if the
///   pose calculation returns error. @c TANGO_SUCCESS otherwise.
TangoErrorType TangoSupport_getDoubleMatrixTransformAtTime(
    double timestamp, TangoCoordinateFrameType base_frame,
    TangoCoordinateFrameType target_frame, TangoSupport_EngineType base_engine,
    TangoSupport_EngineType target_engine,
    TangoSupport_Rotation display_rotation,
    TangoSupport_DoubleMatrixTransformData* matrix_transform);

/// The TangoSupport_PointCloudManager maintains a set of point clouds to
/// manage transferring a TangoPointCloud from the callback thread to a render
/// or computation thread. This holds three buffers internally (back, swap,
/// front). The back buffer is used as the destination for data copied from
/// the callback thread. When the copy is complete the back buffer is swapped
/// with the swap buffer while holding a lock. If there is newer data in
/// the swap buffer than the current front buffer then calling SwapBuffer holds
/// the lock and swaps the swap buffer with the front buffer.
struct TangoSupport_PointCloudManager;

/// @brief Create an object for maintaining a set of point clouds for a
///   specified size.
///
/// @param max_points Maximum number of points in TangoPointCloud. Get value
///   from config.
/// @param manager A handle to the manager object.
/// @return @c TANGO_SUCCESS on successful creation, @c TANGO_INVALID if
///   @p max_points <= 0.
TangoErrorType TangoSupport_createPointCloudManager(
    size_t max_points, TangoSupport_PointCloudManager** manager);

/// @brief Delete the point cloud manager object.
///
/// @param manager A handle to the manager to delete.
/// @return A TangoErrorType value of @c TANGO_SUCCESS on free.
TangoErrorType TangoSupport_freePointCloudManager(
    TangoSupport_PointCloudManager* manager);

/// @brief Updates the back buffer of the manager. Can be safely called from
///   the callback thread. Update is skipped if point cloud is empty.
///
/// @param manager A handle to the point cloud manager.
/// @param point_cloud New point cloud data from the camera callback thread.
/// @return A TangoErrorType value of @c TANGO_INVALID if manager
///   or point_cloud are NULL. Returns @c TANGO_SUCCESS if update
///   is successful.
TangoErrorType TangoSupport_updatePointCloud(
    TangoSupport_PointCloudManager* manager,
    const TangoPointCloud* point_cloud);

/// @brief Check if updated point cloud data is available. If so, swap new data
///   to the front buffer and set latest_point_cloud to point to the front
///   buffer. Multiple calls to this function must be made from the same thread.
///
/// @param manager A handle to the point cloud manager.
/// @param point_cloud After the call contains a pointer to the most recent
///   depth camera buffer.
/// @return @c TANGO_SUCCESS on successful assignment, @c TANGO_INVALID if
///   @p manager is NULL.
TangoErrorType TangoSupport_getLatestPointCloud(
    TangoSupport_PointCloudManager* manager,
    TangoPointCloud** latest_point_cloud);

/// @brief Returns the latest point cloud that has a pose. There is no target
///  frame parameter because only FRAME_CAMERA_DEPTH has meaningful semantics
///  for point clouds. Assumes the same base_engine and target_engine will be
///  passed in each time.
///
/// @param manager A handle to the point cloud manager.
/// @param base_frame The base frame of reference to use in the query.
/// @param base_engine The coordinate system convention of the @p base_frame.
///   Can be OpenGL, Unity, Unreal or Tango but the handed-ness (either
///   left-handed or right-handed) must match the handed-ness of the
///   @p target_engine.
/// @param target_engine The coordinate system convention of the @p
///   target_frame. Can be OpenGL, Unity, Unreal or Tango but the handed-ness
///   (either left-handed or right-handed) must match the handed-ness of the
///   @p base_engine.
/// @param display_rotation The index of the display rotation between
///   display's default (natural) orientation and current orientation.
/// @param point_cloud After the call contains a pointer to the most recent
///   point cloud with a pose, accounting for the specified engine and display
//    rotation.
/// @param pose The pose of target with respect to base frame of reference,
///   accounting for the specified engine and display rotation.
/// @param latest_point_cloud Replaced with the latest point cloud that has a
///   pose, or nullptr on failure. This point cloud is not transformed at all.
/// @param pose Repalced with the pose associated with latest_point_cloud, or
//    nullptr on failure. This pose has been transformed in the same way as
//    getPoseAtTime.
/// @return @c TANGO_SUCCESS on success, @c TANGO_INVALID on invalid input,
///   including mismatched handed-ness of the @p base_engine and @p
///   target_engine, and @c TANGO_ERROR on failure.
TangoErrorType TangoSupport_getLatestPointCloudWithPose(
    TangoSupport_PointCloudManager* manager,
    TangoCoordinateFrameType base_frame, TangoSupport_EngineType base_engine,
    TangoSupport_EngineType target_engine,
    TangoSupport_Rotation display_rotation,
    TangoPointCloud** latest_point_cloud, TangoPoseData* pose);

/// @brief Check if updated point cloud data is available. If so, swap new data
///   to the front buffer and set latest_point_cloud to point to the front
///   buffer. Multiple calls to this function must be made from the same thread.
///   Set @p new_data to true if latest_point_cloud points to new point cloud.
///
/// @param manager A handle to the point cloud manager.
/// @param point_cloud After the call contains a pointer to the most recent
///   depth camera buffer.
/// @param new_data True if latest_point_cloud points to new data. False
///   otherwise.
/// @return @c TANGO_SUCCESS on successful assignment, @c TANGO_INVALID if
///   @p manager is NULL.
TangoErrorType TangoSupport_getLatestPointCloudAndNewDataFlag(
    TangoSupport_PointCloudManager* manager,
    TangoPointCloud** latest_point_cloud, bool* new_data);
/// @}

/// @defgroup DepthSupport Depth Interface Support Functions
/// @brief Functions for managing depth data.
/// @{

/// @brief Fits a plane to a point cloud near a user-specified location. This
///   occurs in two passes. First, all points are projected to the image plane
///   and only points near the user selection are kept. Then a plane is fit to
///   the subset using RANSAC. After the RANSAC fit, all inliers from a larger
///   subset of the original input point cloud are used to refine the plane
///   model. The output is in the base frame of the input translations and
///   rotations. This output frame is usually an application's world frame.
///
/// @param point_cloud The input point cloud. Cannot be NULL and must have at
///   least three points.
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
/// @param intersection_point The intersection of the fitted plane with the user
///   selected camera-ray, in output frame coordinates, accounting for
///   distortion by undistorting the input uv coordinate. Cannot be NULL.
/// @param plane_model The four parameters a, b, c, d for the general plane
///   equation ax + by + cz + d = 0 of the plane fit. The first three
///   components are a unit vector. The output is in the coordinate system of
///   the requested output frame. Cannot be NULL.
/// @return @c TANGO_SUCCESS on success, @c TANGO_INVALID on invalid input, and
///   @c TANGO_ERROR on failure (no plane found).
TangoErrorType TangoSupport_fitPlaneModelNearPoint(
    const TangoPointCloud* point_cloud, const double point_cloud_translation[3],
    const double point_cloud_orientation[4],
    const float color_camera_uv_coordinates[2],
    TangoSupport_Rotation display_rotation,
    const double color_camera_translation[3],
    const double color_camera_orientation[4], double intersection_point[3],
    double plane_model[4]);

/// @brief A structure to define a plane (ax + by + cz + d = 0), including the
/// intersection point of the plane with the user selected camera-ray and
/// inlier information for the plane for points near the user selected
/// camera-ray.
struct TangoSupport_Plane {
  double intersection_point[3];
  double plane_equation[4];
  int inlier_count;
  double inlier_ratio;
};

/// @brief Similar to TangoSupport_fitPlaneModelNearPoint, but after finding
///   a plane at the user selection, points fitting the fit plane are
///   removed from the input points to the RANSAC step and the process is
///   repeated until a fit plane is not found. The output planes are in the
///   base frame of the input translations and rotations. This output frame is
///   usually an application's world frame. Unlike the single plane version,
///   the planes need to be freed by calling TangoSupport_freePlaneList.
///
/// @param point_cloud The input point cloud. Cannot be NULL and must have at
///   least three points.
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
/// @param planes An array of planes fitting the point cloud data near the user
///   selected camera-ray. The plane objects are in the coordinate system of
///   the requested output frame. The array should be deleted by calling
///   TangoSupport_freePlaneList. Cannot be NULL.
/// @param number_of_planes The number of planes in @p planes. Cannot be NULL.
/// @return @c TANGO_SUCCESS on success, @c TANGO_INVALID on invalid input, and
///   @c TANGO_ERROR on failure (no planes found).
TangoErrorType TangoSupport_fitMultiplePlaneModelsNearPoint(
    const TangoPointCloud* point_cloud, const double point_cloud_translation[3],
    const double point_cloud_orientation[4],
    const float color_camera_uv_coordinates[2],
    TangoSupport_Rotation display_rotation,
    const double color_camera_translation[3],
    const double color_camera_orientation[4], TangoSupport_Plane** planes,
    int* number_of_planes);

/// @brief Free memory allocated in call to
/// TangoSupport_fitMultiplePlaneModelsNearPoint.
///
/// @param planes Plane list to free.
void TangoSupport_freePlaneList(TangoSupport_Plane** planes);

/// @}
#ifdef __cplusplus
}
#endif

#endif  // TANGO_SUPPORT_TANGO_SUPPORT_H_
