// Copyright 2013 Motorola Mobility LLC. Part of the Trailmix project.
// CONFIDENTIAL. AUTHORIZED USE ONLY. DO NOT REDISTRIBUTE.
#ifndef TANGO_SERVICE_LIBRARY_TANGO_CLIENT_API_H_
#define TANGO_SERVICE_LIBRARY_TANGO_CLIENT_API_H_

#include <stdbool.h>
#include <stdint.h>

// Tango Camera enumerations.
typedef enum {
  TANGO_CAMERA_COLOR = 0,
  TANGO_CAMERA_RGBIR,
  TANGO_CAMERA_FISHEYE,
  TANGO_CAMERA_DEPTH,
  TANGO_MAX_CAMERA_ID
} TangoCameraId;

// Tango runtime configuration enumerations.
typedef enum {
  TANGO_CONFIG_DEFAULT = 0,
  TANGO_CONFIG_CURRENT,
  TANGO_CONFIG_MOTION_TRACKING,
  TANGO_CONFIG_AREA_DESCRIPTION,
  TANGO_MAX_CONFIG_TYPE     // Maximum number allowable.
} TangoConfigType;

// Tango coordinate frame enumerations.
typedef enum {
  TANGO_COORDINATE_FRAME_GLOBAL = 0,
  TANGO_COORDINATE_FRAME_AREA_DESCRIPTION,
  TANGO_COORDINATE_FRAME_START_OF_SERVICE,
  TANGO_COORDINATE_FRAME_PREVIOUS_DEVICE_POSE,
  TANGO_COORDINATE_FRAME_DEVICE,
  TANGO_COORDINATE_FRAME_IMU,
  TANGO_COORDINATE_FRAME_DISPLAY,
  TANGO_COORDINATE_FRAME_CAMERA_COLOR,
  TANGO_COORDINATE_FRAME_CAMERA_DEPTH,
  TANGO_COORDINATE_FRAME_CAMERA_FISHEYE,
  TANGO_MAX_COORDINATE_FRAME_TYPE
} TangoCoordinateFrameType;

/// Tango Error types.
typedef enum {
  TANGO_ERROR = -1,
  TANGO_SUCCESS = 0,
} TangoErrorType;

struct TangoConfig;
typedef struct TangoConfig TangoConfig;

/// TangoPoseData struct contains information returned from the motion tracking.
/// For details on pose conventions, online documentation should be referenced.
/// Device pose is given in Android conventions.
///
/// version: an integer denoting the version of the structure.
///
/// timestamp: a double type timestamp of the time that this pose estimate
/// corresponds to.
///
/// orientation: orientation, as a quaternion, of the pose of the target frame
/// relative to the reference frame.
/// Specified as (x,y,z,w) where RotationAngle is in radians:
/// x = RotationAxis.x * sin(RotationAngle / 2)
/// y = RotationAxis.y * sin(RotationAngle / 2)
/// z = RotationAxis.z * sin(RotationAngle / 2)
/// w = cos(RotationAngle / 2)
///
/// translation: translation, ordered x, y, z, of the pose of the target frame
/// relative to the reference frame.
typedef enum {
  TANGO_POSE_INITIALIZING = 0,
  TANGO_POSE_VALID,
  TANGO_POSE_INVALID,
  TANGO_POSE_UNKNOWN
} TangoPoseStatusType;

typedef struct TangoPoseData {
  int version;
  double timestamp;
  double orientation[4];
  double translation[3];
  TangoPoseStatusType status_code;
  TangoCoordinateFrameType target_frame;
  TangoCoordinateFrameType reference_frame;
  int confidence;   // Unused.  Integer levels are determined by service.
} TangoPoseData;

/// The TangoXYZij struct contains information returned from the depth sensor.
/// version: an integer denoting the version of the structure.
///
/// timestamp: a double timestamp of the time of capture of the depth data
/// for this struct.
///
/// xyz: An array of packed coordinate triplets, x,y,z as floating point values.
/// With the unit in landscape orientation, screen facing the user:
/// Z points in the direction of the camera's optical axis, and is measured
/// perpendicular to the plane of the camera.
/// X points toward the user's right, and Y points toward the bottom of screen.
/// The output is in units of metres.
///
/// xyz_count: The number of points in depth_data_buffer populated successfully
/// is variable with each call to the function, and is returned i
/// (x,y,z) triplets populated (e.g. 2 points populated returned means 6 floats,
/// or 6*4 bytes used).
///
/// ij_rows, ij_cols: The dimensions of the ij index buffer.
///
/// ij: A 2D buffer, of size ij_rows x ij_cols in raster ordering that contains
/// the index of a point in the xyz array that was generated at this "IJ"
/// location.  A value of -1 denotes there was no corresponding point generated
/// at that position. This buffer can be used to find neighbouring points in the
/// point cloud.
typedef struct TangoXYZij {
  int version;
  double timestamp;

  int xyz_count;
  float (*xyz)[3];

  int ij_rows;
  int ij_cols;
  int *ij;
} TangoXYZij;

#ifdef __cplusplus
extern "C" {
#endif

/// Configuration Templates.  A configuration is a set of settings that
/// must be set before connecting to the service, and cannot be changed after
/// the service has been connected to.

/// Allocate a TangoConfig object.
/// Returns a handle (TangoConfig *) for a newly allocated TangoConfig object.
TangoConfig* TangoConfig_alloc();

/// Deallocate a TangoConfig object.
/// Destroys the TangoConfig object for the handle specified by the config.
/// variable.
void TangoConfig_free(TangoConfig* config);

/// Allocates and return a string with one key=value pair per line.
char *TangoConfig_toString(TangoConfig* config);

/// Tango Service Functions.  These functions are used to connect to, configure,
/// and start the Tango Service.
/// Conceptually they are grouped into the following functional groupings:
///
/// Lifecycle Interface
/// Pose Interface
/// Depth Interface
/// Camera Interface

/// Tango Service: Lifecycle Interface.
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


/// Tango Service: Pose Interface.
/// Attach an onPoseAvailable callback.   The callback is called as new
/// pose updates become available.
int TangoService_connectOnPoseAvailable(
    TangoCoordinateFrameType target_frame,
    TangoCoordinateFrameType reference_frame,
    void (*TangoService_onPoseAvailable)(TangoPoseData* pose));

/// Get a pose at a given timestamp from the reference to the target frame.
///
/// timestamp: if not 0.0, getPoseAtTime retrieves the pose closes to timestamp.
///
/// target_frame/reference_frame: pose is always retrieves as a target frames'
/// pose relative to a reference frame.  See online documentation for a list
/// of implemented target and reference frames available.
///
/// pose: must be allocated by the caller and upon return it is filled with
/// the returned pose.
///
/// pose_size: unused.
int TangoService_getPoseAtTime(double timestamp,
                               TangoCoordinateFrameType target_frame,
                               TangoCoordinateFrameType reference_frame,
                               TangoPoseData* pose);

/// Tango Service: Depth Interface.
/// Attach an onXYZijAvailable callback.  The callback is called each time new
/// depth data is available, at an approximate nominal period given by the
/// double key "depth_period_in_seconds".
int TangoService_connectOnXYZijAvailable(
    void (*TangoService_onXYZijAvailable)(TangoXYZij* xyz_ij));

/// Tango Service: Camera Interface.
/// Connect a Texture ID to a camera. The camera is selected via TangoCameraId.
/// Currently on TANGO_CAMERA_COLOR is supported.  The texture must be the ID of
/// a texture that has been allocated and initialized by the calling
/// application.
int TangoService_connectTextureId(TangoCameraId, int tex);

/// Update the texture that has been connected to camera referenced by
/// TangoCameraId. The texture is updated with the latest image from the camera.
/// If timestamp is not NULL, it will be filled with the image timestamp.
int TangoService_updateTexture(TangoCameraId, double* timestamp);

/// Configuration Parameters Get and Set Functions.
///
/// For an allocated TangoConfig handle, these functions get and set parameters
/// of that TangoConfig handle.  This can be used to query current state, or a
/// handle can be created, then have its contents altered to change the settings
/// later with lockConfig().   For each type of configuration paramter (bool,
/// double, string, etc.) the corredsponding get/set function (e.g. getBool
/// should be called).
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

int TangoConfig_setBool(TangoConfig* config, const char* key, bool value);

int TangoConfig_setInt32(TangoConfig* config, const char* key, int32_t value);

int TangoConfig_setInt64(TangoConfig* config, const char* key, int64_t value);

int TangoConfig_setDouble(TangoConfig* config, const char* key, double value);

int TangoConfig_setString(TangoConfig* config, const char* key,
                          const char* value);

int TangoConfig_getBool(TangoConfig* config, const char* key, bool* value);

int TangoConfig_getInt32(TangoConfig* config, const char* key, int32_t* value);

int TangoConfig_getInt64(TangoConfig* config, const char* key, int64_t* value);

int TangoConfig_getDouble(TangoConfig* config, const char* key, double* value);

int TangoConfig_getString(TangoConfig* config, const char* key, char* value,
                          size_t size);

#ifdef __cplusplus
}
#endif

#endif  // TANGO_SERVICE_LIBRARY_TANGO_CLIENT_API_H_
