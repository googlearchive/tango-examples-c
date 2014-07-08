// Copyright 2013 Motorola Mobility LLC. Part of the Trailmix project.
// CONFIDENTIAL. AUTHORIZED USE ONLY. DO NOT REDISTRIBUTE.

#ifndef TANGO_API_DEPTH_INTERFACE_H_
#define TANGO_API_DEPTH_INTERFACE_H_

#include <tango-api/public-api.h>

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup util-interface Utility Functions

/// \brief Requests Tango Engine to produce Depth Frames.
///
/// API structure requires this function to be executed once during
/// Application context live time, before Depth Frame data requested.
///
/// \param application The application context handle.
/// \return On failure, an error code is returned.
/// \ingroup util-interface
PUBLIC_C_API CAPIErrorCodes DepthStartBuffering(
    application_handle_t* application);

/// \brief Requests Tango Engine to shutdown Depth Frames related functionality.
///
/// Call this function to stop generating depth images and derived data.
///
/// \param application The application context handle.
/// \return On failure, an error code is returned.
/// \ingroup util-interface
PUBLIC_C_API CAPIErrorCodes DepthStopBuffering(
    application_handle_t* application);

/// \brief Get a buffer containing the latest available depth frame.
///
/// This function takes as an input a pointer to a buffer that,
///  upon successful return of the function, will be filled
/// with depth data.  It is the callers responsibility to allocate and
/// manage this memory buffer.
///
/// The buffer has dimensions
/// 320x180 (subject to changes), 32-bit integer values.
/// On successful return, DepthBuffer will be
/// updated with the current depth values.   Each element of the array contains
/// 12-bits of depth data (this integer array can be converted to
/// a 16-bit type to save memory), representing distance perpendicular to
/// the camera sensor plane in millimetres (see diagram below).
///
/// Depth values are updated at 5 frames per second.
///
/// DepthBuffer values can range from 0 to 2e12.  However, because of sensor
/// limitations, typical usable values fall in the range of 500 to  6000
/// millimetres.  A value of 0 in the depth buffer indicates that no reading was
/// taken at that point.
///
/// Use interface DepthGetResolution to request depth frame size.
///
/// \param application The application context handle.
/// \param depth_image_buffer Pointer to pre-allocated buffer of
///  size DepthFrameWidth*DepthFrameHeight*sizeof(int).
/// \param buffer_size Preallocated buffer size.
/// \param actual_timestamp Depth frame timestamp.
/// \return On failure, an error code is returned.
/// \ingroup util-interface
PUBLIC_C_API CAPIErrorCodes DepthGetLatestFrame(
    application_handle_t* application, int* depth_image_buffer, int buffer_size,
    double* actual_timestamp);

/// \brief Get a buffer containing point cloud data.
/// (Experimental, Yellowstone only)
///
/// This is an experimental API and only produces data on Yellowstone devices.
/// This function fills a buffer with point cloud data which is an array of
/// packed coordinate triplets, x,y,z as floating point values.  The number of
/// points in depth_data_buffer populated successfully is variable with each
/// call to the function, and is returned in buffer_size as the number of
/// (x,y,z) triplets populated (e.g. 2 points populated returned means 6 floats,
/// or 6*4 bytes used).
/// Z points in the direction of the camera's optical axis.
/// With the unit in landscape orientation, screen facing the user,
/// X points toward the user's right, and Y points toward the bottom of screen.
/// If DepthGetPointCloudUnityExperimental is called it returns point cloud data
/// as with the above convention, but Y points toward top of screen.
/// Similar to the depth image, the output unit is millimeters.
/// The buffer must be allocated by the calling application, and must
/// be of minimum size 61440 elements, or 61440*3*4 = 737280 Bytes, as on
/// initial Yellowstone devices the depth processing is limited to producing
/// up to 61440 points.
/// The timestamp pointer must be valid memory, which is filled with the
/// timestamp of the pointcloud.  The timestamp pointer must be set to 0.0
/// explicitly by the calling application, which will query the latest
/// available pointcloud.
/// \param application The application context handle.
/// \param depth_data_buffer Preallocated buffer which is filled with points on
///    successful return.  Unused point data is left unchanged.
/// \param timestamp Must be provided as a pointer to valid memory.  Must be set
///    to zero which will return the latest point cloud.  It will be filled in
///    with the timestamp of the returned point cloud.
/// \param max_delta Max allowable delta time for time based querying.
/// \param buffer_size_in_elements  Application must set this to the size of the
///    depth_data_buffer in elements of floating point triplets (x,y,z).
///    On return it is set to the amount of elements that are populated by the
///    call. Function returns error if the buffer_size_in_bytes is less than
///    737280 or 61440 elements on Yellowstone.
PUBLIC_C_API CAPIErrorCodes DepthGetPointCloud(
    application_handle_t* application,
    double* timestamp, double max_delta,
    float* depth_data_buffer,
    int* buffer_size_in_elements);
PUBLIC_C_API CAPIErrorCodes DepthGetPointCloudUnity(
    application_handle_t* application,
    double* timestamp, double max_delta,
    float* depth_data_buffer,
    int* buffer_size);

/// \brief Request latest available depth frame timestamp.
///
/// This function takes as an input a pointer to a double variable,
/// upon successful return of the function, will be filled
/// with latest available depth frame timestamp.
///
/// \param application The application context handle.
/// \param actual_timestamp Latest available depth frame timestamp.
/// \return On failure, an error code is returned.
/// \ingroup util-interface
PUBLIC_C_API CAPIErrorCodes DepthQueryLatestFrame(
    application_handle_t* application, double* actual_timestamp);

/// \brief Request depth frame resolution in pixels.
///
/// This function returns the depth frame resolution in pixels.
/// Depth Frame resolution can be different on different platforms.
///
/// \param widthHeight Depth Frame resolution.
///    On Peanut, it is the pixel dimension of the depth image.
///    On Yellowstone, it is the maximum length (number) of (x,y,z) points
///       that can be generated by the depth sensor.  Returned as [61440, 1].
/// \return On failure, an error code is returned.
/// \ingroup util-interface
PUBLIC_C_API CAPIErrorCodes DepthGetResolution(int widthHeight[2]);

/// \brief Request maximum point cloud size in points.
/// (Experimental, Yellowstone only)
///
/// This function returns the maximum number of points that DepthGetPointCloud
/// or DepthGetPointCloudUnity can return. The buffer passed to
/// DepthGetPointCloud must be sized to accomdate the maximum number of points
/// as returned by this function which, in bytes, should be
/// (size * 3 * sizeof(float)).
///
/// \param size On successful return, this is filled with the maximum number of
/// points DepthGetPointCloud can return. Each point is a (x,y,z) triplet of
/// floating point values.  On error, size is set to -1.
/// \return On failure, an error code is returned, otherwise kCAPISuccess.
/// \ingroup util-interface
PUBLIC_C_API CAPIErrorCodes DepthGetPointCloudMaxSize(int* size);

#ifdef __cplusplus
}
#endif

#endif  // TANGO_API_DEPTH_INTERFACE_H_
