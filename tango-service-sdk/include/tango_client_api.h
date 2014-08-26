// Copyright 2013 Motorola Mobility LLC. Part of the Trailmix project.
// CONFIDENTIAL. AUTHORIZED USE ONLY. DO NOT REDISTRIBUTE.
#ifndef TANGO_SERVICE_LIBRARY_TANGO_CLIENT_API_H_
#define TANGO_SERVICE_LIBRARY_TANGO_CLIENT_API_H_

#include <stdbool.h>
#include <stdint.h>

/// @file tango_client_api.h
/// @brief File containing Project Tango C API

/// @defgroup enums Project Tango Enumerations
/// @brief Enums for camera, configuration, coordinate frame, and error types.
/// @{

/// @brief Tango Camera enumerations.
typedef enum {
  TANGO_CAMERA_COLOR = 0, /**< Back-facing color camera */
  TANGO_CAMERA_RGBIR,  /**< Back-facing camera producing IR-sensitive images */
  TANGO_CAMERA_FISHEYE,   /**< Back-facing fisheye wide-angle camera */
  TANGO_CAMERA_DEPTH,     /**< Depth camera */
  TANGO_MAX_CAMERA_ID     /**< Maximum camera allowable */
} TangoCameraId;

/// @brief Tango runtime configuration enumerations.
typedef enum {
  TANGO_CONFIG_DEFAULT = 0,         /**< Default */
  TANGO_CONFIG_CURRENT,             /**< Current */
  TANGO_CONFIG_MOTION_TRACKING,     /**< Motion tracking */
  TANGO_CONFIG_AREA_DESCRIPTION,    /**< Area learning */
  TANGO_MAX_CONFIG_TYPE             /**< Maximum number allowable.  */
} TangoConfigType;

/// @brief Tango coordinate frame enumerations.
typedef enum {
  /** Coordinate system for the entire Earth.
   *  See WGS84: http://en.wikipedia.org/wiki/World_Geodetic_System
   */
  TANGO_COORDINATE_FRAME_GLOBAL = 0,
  /** Origin within a saved area description */
  TANGO_COORDINATE_FRAME_AREA_DESCRIPTION,
  /** Origin when the device started tracking */
  TANGO_COORDINATE_FRAME_START_OF_SERVICE,
  /** Immediately previous device pose */
  TANGO_COORDINATE_FRAME_PREVIOUS_DEVICE_POSE,
  TANGO_COORDINATE_FRAME_DEVICE,             /**< Device coordinate frame */
  TANGO_COORDINATE_FRAME_IMU,                /**< Inertial Measurement Unit */
  TANGO_COORDINATE_FRAME_DISPLAY,            /**< Display */
  TANGO_COORDINATE_FRAME_CAMERA_COLOR,       /**< Color camera */
  TANGO_COORDINATE_FRAME_CAMERA_DEPTH,       /**< Depth camera */
  TANGO_COORDINATE_FRAME_CAMERA_FISHEYE,     /**< Fisheye camera */
  TANGO_MAX_COORDINATE_FRAME_TYPE            /**< Maximum allowed */
} TangoCoordinateFrameType;

/// @brief Tango Error types.
/// Errors less then 0 should be dealt with by the program.
/// Success is denoted by TANGO_SUCCESS = 0.
typedef enum {
  TANGO_ERROR = -1,    /**< Hard error */
  TANGO_SUCCESS = 0,   /**< Success */
} TangoErrorType;

// doxygen does not seem to accept endgroups without the ** comment style
/**@}*/

struct TangoConfig;
typedef struct TangoConfig TangoConfig;

typedef enum {
  TANGO_POSE_INITIALIZING = 0,
  TANGO_POSE_VALID,
  TANGO_POSE_INVALID,
  TANGO_POSE_UNKNOWN
} TangoPoseStatusType;

/// The TangoPoseData struct contains pose information returned from motion
/// tracking.
/// The device pose is given using Android conventions.
/// See http://developer.android.com/guide/topics/sensors/sensors_overview.html#sensors-coords
typedef struct TangoPoseData {
  /// An integer denoting the version of the structure.
  int version;

  /// Timestamp of the time that this pose estimate corresponds to.
  double timestamp;

  /// Orientation, as a quaternion, of the pose of the target frame
  /// relative to the reference frame.
  /// Specified as (x,y,z,w) where RotationAngle is in radians:
  /// x = RotationAxis.x * sin(RotationAngle / 2)
  /// y = RotationAxis.y * sin(RotationAngle / 2)
  /// z = RotationAxis.z * sin(RotationAngle / 2)
  /// w = cos(RotationAngle / 2)
  double orientation[4];

  /// Translation, ordered x, y, z, of the pose of the target frame
  /// relative to the reference frame.
  double translation[3];
  TangoPoseStatusType status_code;
  TangoCoordinateFrameType target_frame;
  TangoCoordinateFrameType reference_frame;
  int confidence;   // Unused.  Integer levels are determined by service.
} TangoPoseData;

/// The TangoXYZij struct contains information returned from the depth sensor.
typedef struct TangoXYZij {
  /// An integer denoting the version of the structure.
  int version;

  /// Time of capture of the depth data for this struct (in seconds).
  double timestamp;

  /// The number of points in depth_data_buffer populated successfully
  /// is variable with each call to the function, and is returned in
  /// (x,y,z) triplets populated (e.g. 2 points populated returned means 6
  /// floats, or 6*4 bytes used).
  int xyz_count;

  /// An array of packed coordinate triplets, x,y,z as floating point values.
  /// With the unit in landscape orientation, screen facing the user:
  /// Z points in the direction of the camera's optical axis, and is measured
  /// perpendicular to the plane of the camera.
  /// X points toward the user's right, and Y points toward the bottom of
  /// the screen.
  /// The output is in units of metres.
  float (*xyz)[3];

  /// The dimensions of the ij index buffer.
  int ij_rows;
  /// The dimensions of the ij index buffer.
  int ij_cols;

  /// A 2D buffer, of size ij_rows x ij_cols in raster ordering that contains
  /// the index of a point in the xyz array that was generated at this "IJ"
  /// location.  A value of -1 denotes there was no corresponding point
  /// generated at that position. This buffer can be used to find neighbouring
  /// points in the point cloud.
  int *ij;
} TangoXYZij;

#ifdef __cplusplus
extern "C" {
#endif


/// @defgroup configtemplates Configuration Templates
/// @brief Functions for setting configurations for connecting to the device.
///
/// Configuration Templates.  A configuration is a set of settings that
/// must be set before connecting to the service, and cannot be changed after
/// the service has been connected to.
/// @{

/// @brief Allocate a TangoConfig object.
/// Returns a handle (TangoConfig *) for a newly allocated TangoConfig object.
TangoConfig* TangoConfig_alloc();

/// Deallocate a TangoConfig object.
/// Destroys the TangoConfig object for the handle specified by the config.
/// variable.
void TangoConfig_free(TangoConfig* config);

/// Allocates and return a string with one key=value pair per line.
char *TangoConfig_toString(TangoConfig* config);

/**@}*/

/// Tango Service Functions.  These functions are used to connect to,
/// configure, and start the Tango Service.
/// Conceptually they are grouped into the following functional groupings:
///
/// Lifecycle Interface
/// Pose Interface
/// Depth Interface
/// Camera Interface

/// @defgroup lifecycle Tango Service: Lifecycle Interface
/// @brief Functions for initializing and shutting down the Tango Service.
/// @{

/// Initializes the TangoService.   This function must be called first before
/// other Tango functions are called.
TangoErrorType TangoService_initialize();

/// Given an allocated TangoConfig, config, fill in config with configuration
/// settings.  This can be used to find the current configuration, or
/// to fill a config with a set of commonly used, default parameters before
/// altering a few for the application's needs.
///    TANGO_CONFIG_DEFAULT = Default configuration of the service.
int TangoService_getConfig(TangoConfigType config_type, TangoConfig* config);

/// Lock a new configuration.  This will place the service into the
/// configuration given by config.  This will interrupt other client
/// configurations.
int TangoService_lockConfig(TangoConfig* config);

/// Unlock the Tango Service.   This function will unlock the Tango Services'
/// configuration.  It should be called when the service's specific
/// configuration is no longer needed, such as when shutting down.  This will
/// allow other clients to change the configuration of the service.
int TangoService_unlockConfig();

/// Connect to the Tango Service.   This connects to and starts the service
/// running.  Callbacks will begin to be generated, and pose can be queried.
int TangoService_connect();

/// Disconnect from the Tango Service. Callbacks will no longer be generated.
/// Applications should always call disconnect when the service is no longer
/// needed.
void TangoService_disconnect();

/**@}*/

/// @defgroup pose Tango Service: Pose Interface
/// @brief Functions for getting the pose of the device.
/// @{

/// Attach an onPoseAvailable callback.   The callback is called as new
/// pose updates become available.
int TangoService_connectOnPoseAvailable(
    TangoCoordinateFrameType target_frame,
    TangoCoordinateFrameType reference_frame,
    void (*TangoService_onPoseAvailable)(TangoPoseData* pose));

/// Get a pose at a given timestamp from the reference to the target frame.
///
/// @param timestamp Time specified in seconds.  If not 0.0, getPoseAtTime
/// retrieves the pose closest to this timestamp.
///
/// @param target_frame pose is always retrieved as a target frames'
/// pose relative to a reference frame.
/// @param reference_frame pose is always retrieved as a target frames'
/// pose relative to a reference frame.  See online documentation for a list
/// of implemented target and reference frames available.
///
/// @param pose must be allocated by the caller and upon return it is filled
/// with the returned pose.
///
/// @param pose_size unused.
int TangoService_getPoseAtTime(double timestamp,
                               TangoCoordinateFrameType target_frame,
                               TangoCoordinateFrameType reference_frame,
                               TangoPoseData *pose);
/**@}*/

/// @defgroup depth Tango Service: Depth Interface
/// @brief Functions for getting depth information from the device.
/// @{

/// Attach an onXYZijAvailable callback.  The callback is called each time new
/// depth data is available, at an approximate nominal period given by the
/// double key "depth_period_in_seconds".
int TangoService_connectOnXYZijAvailable(
    void (*TangoService_onXYZijAvailable)(TangoXYZij* xyz_ij));

/**@}*/

/// @defgroup camera Tango Service: Camera Interface
/// @brief Functions for getting input from the device's cameras.
/// @{

/// Connect a Texture ID to a camera. The camera is selected via TangoCameraId.
/// Currently on TANGO_CAMERA_COLOR is supported.  The texture must be the ID
/// of a texture that has been allocated and initialized by the calling
/// application.
int TangoService_connectTextureId(TangoCameraId, int tex);

/// Update the texture that has been connected to camera referenced by
/// TangoCameraId. The texture is updated with the latest image from the
/// camera.
/// If timestamp is not NULL, it will be filled with the image timestamp.
int TangoService_updateTexture(TangoCameraId, double* timestamp);

/**@}*/

/// @defgroup configparams Configuration Parameters Get and Set Functions.
/// @brief Configuration Parameters Get and Set Functions.
///
/// For an allocated TangoConfig handle, these functions get and set parameters
/// of that TangoConfig handle.  You can use the handle to query the current
/// state, or you can create a new handle and alter its contents to change the
/// settings later with lockConfig().  For each type of configuration parameter
/// (bool, double, string, etc) you call the corresponding get/set function,
/// such as getBool for a boolean.
///
/// The supported configuration parameters that can be set are:
/// (denoted below as "type" "name"):
///
///     boolean config_enable_depth:
///         enables depth output if true.
///
///     boolean config_enable_motion_tracking:
///         enables motion tracking if true.
///
///     boolean config_enable_dataset_recording:
///         enables recording of a dataset to disk.
///
/// The supported configuration parameters that can be queried are:
///
///     char* tango_service_library_version:
/// the version of the Tango Service Library that implements the Tango
/// functionality.  The version is returned as YYMMDD-{git hash}-{ARCHITECTURE}.
/// For example: 140806-e33db4c-YELLOWSTONE was built on August 6, 2014, from
/// git hash e33db4c for the Yellowstone device.
///
///     double depth_period_in_seconds:
/// Nominal time between successive frames of depth data.  Use the depth data
/// structure fields to get more accurate depth frame times.
///
///     int32 max_point_cloud_elements:
/// Maximum number of points returned in depth point clouds.  For Yellowstone
/// this is 60000.  Typically no more than to 15000 are returned.
///
/// @{

/// TODO
int TangoConfig_setBool(TangoConfig *config, const char *key, bool value);

/// TODO
int TangoConfig_setInt32(TangoConfig *config, const char *key, int32_t value);

/// TODO
int TangoConfig_setInt64(TangoConfig *config, const char *key, int64_t value);

/// TODO
int TangoConfig_setDouble(TangoConfig *config, const char *key, double value);

/// TODO
int TangoConfig_setString(TangoConfig *config, const char *key,
                          const char *value);

/// TODO
int TangoConfig_getBool(TangoConfig *config, const char *key, bool *value);

/// TODO
int TangoConfig_getInt32(TangoConfig *config, const char *key, int32_t *value);

/// TODO
int TangoConfig_getInt64(TangoConfig *config, const char *key, int64_t *value);

/// TODO
int TangoConfig_getDouble(TangoConfig *config, const char *key, double *value);

/// TODO
int TangoConfig_getString(TangoConfig *config, const char *key, char *value,
                          size_t size);
/**@}*/

#ifdef __cplusplus
}
#endif

#endif  // TANGO_SERVICE_LIBRARY_TANGO_CLIENT_API_H_
