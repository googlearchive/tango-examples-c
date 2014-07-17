// Copyright 2013 Motorola Mobility LLC. Part of the Trailmix project.
// CONFIDENTIAL. AUTHORIZED USE ONLY. DO NOT REDISTRIBUTE.
/*! \file */

#ifndef TANGO_API_VIO_INTERFACE_H_
#define TANGO_API_VIO_INTERFACE_H_

#include <stdint.h>

#include <tango-api/public-api.h>

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup vio-interface VIO (Visual Inertial Odometry) Interface Module

enum CAPIOdometryStatusCodes {
  kCAPIInitializing = 500,
  kCAPIRunning = 501,
  kCAPITrackingLost = 502,
  kCAPIRecovering = 503,
  kCAPIUnknown = -1
};

#pragma pack(push, 4)
/// \brief Visual Inertial Odometry status.
///
/// Describes status of Visual Inertial Odometry engine at the given point of
/// time.
struct VIOStatus {
  /// Rotation of device represented by quaternion.
  float rotation[4];
  /// Translation of device represented by translation vector.
  float translation[3];
  /// Actual timestamp of VIO status measurement.
  double timestamp;
  /// Flags if Sparse Mapping Relocalisation occurred at the frame with this
  /// timestamp.
  int relocalized;
  /// VIO engine internal status.
  CAPIOdometryStatusCodes status_code;
};
/// \brief Map metadata. (Experimental, subject to change)
///
/// Describes properties of a map. All strings are null-terminated.
struct MapMetadata {
  /// The 36-character map ID.
  char id[37];
  /// The directory that contains the map files. Includes a trailing slash.
  char map_path[256];
  /// Type of the map, map_structure::Map::[UNKNOWN, POSE_GRAPH, BUNDLE_ADJUST].
  uint32_t map_type;
  /// User-given name for the map.
  char name[256];
  /// Map creation date in milliseconds since Jan 1, 1970 UTC (Unix time).
  uint64_t date;
};
#pragma pack(pop)

/// \brief Initializes and starts the VIO (Visual Inertial Odometry) engine.
///
/// This initializes and starts the VIO (Visual Inertial Odometry) engine.  The
/// VIO may be initialized with or without "sparse mapping" enabled.
/// When not enabled, the device tracks its pose open-loop, similar to dead
/// reckoning.  When sparse mapping is enabled, the device will attempt to
/// recognize places it has been before, and periodically correct its pose
/// relative to those places.  A recognition of place and a corresponding
/// correction of pose is called "relocalization".
///
/// With sparse mapping, relocalization may occur in one of two ways.  If no
/// map filename is provided (i.e. map_path is NULL) the library creates and
/// maintains a new sparse map in memory until the library is shut down,
/// and attempts to relocalize against it.
///
/// If a map filename is provided, then the device will attempt to relocalize
/// against the provided map.
///
/// \param application_handle The application context handle.
/// \param enable_sparse_mapping If non-zero, this will enable the sparse
///   mapping mode of VIO.
/// \param path_map If sparse mapping is enabled, then this argument may be
///   set to a filename of a previously recorded sparse map, or set to NULL
///   to not match to a pre-existing sparse map.
/// \return On success, returns kCAPISuccess.  On failure returns an error code.
/// \ingroup vio-interface
/// \note todo(jfung) if a map is loaded, does sparse change it?
PUBLIC_C_API CAPIErrorCodes VIOInitialize(
    application_handle_t* application_handle, int enable_sparse_mapping,
    const char* path_map);

/// \brief Gets the latest pose of the device.
///
/// VIOGetLatestPose gets the latest pose and status from the position
/// estimation engine.  The pose of the device is estimated as the position of
/// the device's IMU (Inertial Measurement Unit).
/// Look here to see more information related to coordinate conventions:
/// https://sites.google.com/a/google.com/project-tango-sdk/home/software-tutorials/coordinate-system-conventions
/// VIOGetLatestPose returns the position and orientation of the device
/// in the "Estimator" convention used by the library.
/// The quaternion is in x-y-z-w convention (JPL convention please ref.
/// [http://www-users.cs.umn.edu/~trawny/Publications/Quaternions_Techreport.htm])
/// and in Estimator convention, the direction of gravity is negative z-axis.
/// Support for space stations which lack artificial gravity is pending.
///
/// The pose may be computed "open loop" if the VIO context was initialized
/// with sparse mapping disabled.
/// In this case, the returned pose is always relative to the position at the
/// time of initialization of the library.  That is, on startup, the pose of
/// the device is assumed to be the origin.
///
/// If the context was initialized with sparse mapping, and a sparse map has
/// been loaded, on a relocalization event the returned
/// pose is relative to the sparse map's origin.
///
/// If sparse mapping is enabled, but no map was specified, a new sparse map is
/// created internally and used for relocalization (loop closure).   The library
/// will attempt to relocalize against this internal sparse map,  may occur
/// against this internal sparse map.   The returned pose is relative to the
/// origin, determined by the position of the device at startup.
///
/// When a relocalization occurs in the sparse mapping modes, the most updated
/// pose is provided, and there is no smoothing to transition to the relocalized
/// pose.  It is up to the application to implement smoothing if desired.
///
/// \param application_handle The application context handle.
/// \param status Pointer to allocated VIOStatus.
/// \return kCAPISuccess if successfully initialized, an error code otherwise.
/// \ingroup vio-interface
/// \note The format of the estimator data may change in future versions.
/// \note Prefer to use function VIOGetLatestPoseUnity/Android/OpenGL,
/// to avoid fragmentation.
PUBLIC_C_API CAPIErrorCodes VIOGetLatestPose(
    application_handle_t* application_handle, VIOStatus* status);

/// \brief Gets the latest pose of the device.
/// Same as VIOGetLatestPose, except it returns pose in Unity format.
PUBLIC_C_API CAPIErrorCodes VIOGetLatestPoseUnity(
    application_handle_t* application_handle, VIOStatus* status);

/// \brief Gets the latest pose of the device.
/// Same as VIOGetLatestPose, except it returns pose in Android format.
PUBLIC_C_API CAPIErrorCodes VIOGetLatestPoseAndroid(
    application_handle_t* application_handle, VIOStatus* status);

/// \brief Gets the latest pose of the device.
/// Same as VIOGetLatestPose, except it returns pose in OpenGL format.
PUBLIC_C_API CAPIErrorCodes VIOGetLatestPoseOpenGL(
    application_handle_t* application_handle, VIOStatus* status);

/// \brief Request the closest pose of the device to timestamp.
/// Same as VIOGetLatestPose, except it returns pose closest to
/// expected_timestamp.
PUBLIC_C_API CAPIErrorCodes VIOGetClosestPoseToTime(
    application_handle_t* application_handle, double expected_timestamp,
    double max_delta, VIOStatus* status);

/// \brief Gets the latest pose of the device.
/// Same as VIOGetClosestPoseToTime, except it returns pose in Unity format.
PUBLIC_C_API CAPIErrorCodes VIOGetClosestPoseToTimeUnity(
    application_handle_t* application_handle, double expected_timestamp,
    double max_delta, VIOStatus* status);

/// \brief Gets the latest pose of the device.
/// Same as VIOGetClosestPoseToTime, except it returns pose in Android format.
PUBLIC_C_API CAPIErrorCodes VIOGetClosestPoseToTimeAndroid(
    application_handle_t* application_handle, double expected_timestamp,
    double max_delta, VIOStatus* status);

/// \brief Gets the latest pose of the device.
/// Same as VIOGetClosestPoseToTime, except it returns pose in OpenGL format.
PUBLIC_C_API CAPIErrorCodes VIOGetClosestPoseToTimeOpenGL(
    application_handle_t* application_handle, double expected_timestamp,
    double max_delta, VIOStatus* status);

/// \brief Allow VIO to reset itself in case of VIO filter become unstable.
///
/// \param auto_reset Enable auto reset functionality is auto_reset != 0.
/// \return On success, returns kCAPISuccess.  On failure returns an error code.
/// \ingroup application-interface
PUBLIC_C_API CAPIErrorCodes VIOSetAutoReset(int auto_reset);

/// \brief Request VIO to reset it is state.
/// \param application The application context handle.
/// \return On success, returns kCAPISuccess.  On failure returns an error code.
/// \ingroup application-interface
PUBLIC_C_API CAPIErrorCodes VIOReset(application_handle_t* application_handle);

/// \brief Save the current Area Description file.
///
/// Note: "Sparse Map" terminology is replaced by "Area Description".
/// When VIO is initialized in area learning mode, it will create an internal
/// area description file of the current run.  This function saves that
/// area description file.
/// \param application_handle The application context handle.
/// \param map_path Full absolute path and filename (e.g. /sdcard/defaultArea)
/// \ingroup vio-interface
/// \warning The current format of the area description file is not guaranteed
///   to be compatible between versions.  This may be included in future
///   versions.
PUBLIC_C_API CAPIErrorCodes VIOSaveSparseMap(
    application_handle_t* application_handle, char* map_path);

/// \brief Save the current Area Description file, using Tango map management
/// services. (Experimental)
///
/// Note: Tango Area Description management service API is experimental and
///   subject to change.
/// When VIO is initialized in sparse mapping mode, it will create an internal
/// sparse map of the current run.  This function saves that sparse map to a
/// file.
/// \param application_handle The application context handle.
/// \param map_id Output parameter for the map ID.
/// \ingroup vio-interface
/// \warning The current format of the sparse map is not guaranteed to be
///  compatible between versions.  Backward compatibility may be included.
PUBLIC_C_API CAPIErrorCodes VIOSaveSparseMapTangoSystem(
    application_handle_t* application_handle, char map_id[37]);

/// \brief Return a list of information about all the maps on the device.
/// (Experimental)
///
/// For use with the Tango mapping serivce API.
/// The array will be allocated internally and must be destroyed using
/// VIODestroyMapMetadata after it has been used.
/// \param Output parameter for the number of entries in the array.
/// \param Output parameter pointing to the beginning of the array.
PUBLIC_C_API CAPIErrorCodes VIOListMapMetadata(
    int* num_entries, MapMetadata** map_metadata_array);

/// \brief Deallocate the metadata array returned by VIOListMapMetadata.
/// (Experimental)
/// For use with the Tango mapping service API.
PUBLIC_C_API CAPIErrorCodes VIODestroyMapMetadata(
    MapMetadata* map_metadata_array);

/// \brief Write the given metadata to disk. (Experimental)
/// For use with the Tango mapping service API.
PUBLIC_C_API CAPIErrorCodes VIOWriteMapMetadata(
    MapMetadata* map_metadata);

/// \brief Not Supported
PUBLIC_C_API CAPIErrorCodes VIORegisterNewAnchor(char* anchor_id);
/// \brief Not Supported
PUBLIC_C_API CAPIErrorCodes VIORegisterExistingAnchors(
    int num_ids, unsigned char* registered_ids);
/// \brief Not Supported
PUBLIC_C_API CAPIErrorCodes VIOQueryAnchorPose(
    char registered_ID[16], float* pose_g_T_A_transformation_matrix);
/// \brief Not Supported
PUBLIC_C_API CAPIErrorCodes VIOQueryAnchorPoseUnity(
    char registered_ID[16], float* pose_g_T_A_transformation_matrix_Unity);
/// \brief Not Supported
PUBLIC_C_API CAPIErrorCodes VIOClearAnchors(int num_ids,
                                            unsigned char* registered_ids);

/// \brief Shuts down the VIO engine.
/// \param application The application context handle.
/// \return On success, returns kCAPISuccess.  On failure returns an error code.
/// \note This function is currently not implemented.
/// \ingroup application-interface
PUBLIC_C_API CAPIErrorCodes VIOShutdown(
    application_handle_t* application_handle);

#ifdef __cplusplus
}
#endif

#endif  // TANGO_API_VIO_INTERFACE_H_
