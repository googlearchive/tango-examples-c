/*
 * Copyright 2014 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
  TANGO_CONFIG_DEFAULT = 0,         /**< Default, motion tracking only. */
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
  TANGO_COORDINATE_FRAME_INVALID,
  TANGO_MAX_COORDINATE_FRAME_TYPE            /**< Maximum allowed */
} TangoCoordinateFrameType;

/// @brief Tango coordinate frame pairs, since individual frames are
/// meaningless.
typedef struct {
  TangoCoordinateFrameType base;
  TangoCoordinateFrameType target;
} TangoCoordinateFramePair;

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


/// @brief Tango pose status lifecycle enumerations.  Every pose has a state
/// denoted by this enum, which provides information about the internal
/// status of the position estimate. The application may use the status to
/// decide what actions or rendering should be taken. A change in the status
/// between poses and subsequent timestamps can denote lifecycle state changes.
/// The status affects the rotation and position estimates. Other fields are
/// considered valid (i.e. version or timestamp).
typedef enum {
  TANGO_POSE_INITIALIZING = 0,  /**< Motion estimation is being initialized */
  TANGO_POSE_VALID,             /**< The pose of this estimate is valid */
  TANGO_POSE_INVALID,           /**< The pose of this estimate is not valid */
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
  /// The status of the pose, according to the pose lifecycle.
  TangoPoseStatusType status_code;
  /// The pair of coordinate frames.
  TangoCoordinateFramePair frame;
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
  /// +Z points in the direction of the camera's optical axis, and is measured
  /// perpendicular to the plane of the camera.
  /// +X points toward the user's right, and +Y points toward the bottom of
  /// the screen.
  /// The output is in units of metres.
  float (*xyz)[3];

  /// The dimensions of the ij index buffer.
  int ij_rows;
  /// The dimensions of the ij index buffer.
  int ij_cols;

  /// A 2D buffer, of size ij_rows x ij_cols in raster ordering that contains
  /// the index of a point in the xyz array that was generated at this "ij"
  /// location.  A value of -1 denotes there was no corresponding point
  /// generated at that position. This buffer can be used to find neighbouring
  /// points in the point cloud.
  int *ij;
} TangoXYZij;

#define UUID_LEN 37

/// UUID struct contains the unique id associated with a single area
/// description.  Should be 36 characters including dashes and a null
/// terminating character.
typedef struct {
  char data[UUID_LEN];
} UUID;

/// UUID_list struct contains a set of area descriptions, generally
/// the area descriptions available for loading on a particular
/// device.
typedef struct {
  int count;
  UUID* uuid;
} UUID_list;

/// Metadata_entry struct contains a single key/value pair associated
/// with a single entry in the area description metadata struct.
typedef struct {
  size_t key_size;
  char* key;
  size_t value_size;
  char* value;
} Metadata_entry;

/// Metadata_list struct contains a set of Metadata_entry objects
/// associated with a single area description.
typedef struct {
  int num_entries;
  Metadata_entry* metadata_entries;
} Metadata_list;

/// Tango Event types.
typedef enum {
  TANGO_EVENT_UNKNOWN,
  TANGO_EVENT_STATUS_UPDATE,
  TANGO_EVENT_ADF_UPDATE,
} TangoEventType;

typedef enum {
  TANGO_STATUS_UNKNOWN,
  TANGO_STATUS_FOV_OVER_EXPOSED,
  TANGO_STATUS_FOV_UNDER_EXPOSED,
  TANGO_STATUS_TOO_FEW_FEATURES_TRACKED,
  TANGO_STATUS_COLOR_OVER_EXPOSED,
  TANGO_STATUS_COLOR_UNDER_EXPOSED,
  TANGO_STATUS_COUNT
} TangoStatusType;

typedef struct TangoEvent {
  int version;
  double timestamp;
  TangoEventType type;

  union {
    TangoStatusType status;
    struct {
      TangoCoordinateFrameType target_frame;
      TangoCoordinateFrameType reference_frame;
    } adf;
  } data;
} TangoEvent;

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
/// Returns a handle (TangoConfig *) for a newly allocated, empty and
/// uninitialized TangoConfig object. This TangoConfig object must be configured
/// with at least TangoService_getConfig() to initialize it with default values,
/// and then TangConfig_set functions can be used to set specific configuration
/// values.
TangoConfig* TangoConfig_alloc();

/// Deallocate a TangoConfig object.
/// Destroys the TangoConfig object for the handle specified by the config.
/// variable.
void TangoConfig_free(TangoConfig* config);

/// Allocates and return a string with one key=value pair per line of all of the
/// configuration values of Tango Service.  Note many of these are 'read-only',
/// unless otherwise documented.
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

/// Initializes the TangoService.  This function must be called first before
/// other Tango functions are called.  A connection to the service is created.
/// To succed, the calling application must have camera permissions enabled.
TangoErrorType TangoService_initialize();

/// Given a TangoConfig object that was allocated with TangoConfig_alloc(),
/// TangoService_getConfig() fills in Config with configuration
/// settings from the service.  This should be used to initialize a Config
/// object before setting custom settings and locking that configuration.  This
/// function can also be used to find the current configuration of the service.
///    TANGO_CONFIG_DEFAULT = Default configuration of the service.
int TangoService_getConfig(TangoConfigType config_type, TangoConfig* config);

/// Lock a new configuration.  This will place the service into the
/// configuration given by config.  This will interrupt other client
/// configurations.  The config object must be initialized with
/// TangoService_getConfig() oth
int TangoService_lockConfig(TangoConfig* config);

/// Unlock the Tango Service.   This function will unlock the Tango Services'
/// configuration.  It should be called when the service's specific
/// configuration is no longer needed, such as when shutting down.  This will
/// allow other clients to change the configuration of the service.
int TangoService_unlockConfig();

/// Connect to the Tango Service.   This connects to and starts the service
/// running.  The service will run with the current configuration, in either
/// its default configuration, or most recently locked configuration if new
/// configuration values were set.  TangoService_connect() starts the motion
/// estimation and other components of the service.  After calling
/// TangoService_connect(), callbacks will begin to be generated, and pose can
/// be queried, and other data (camera, depth) will become available.
int TangoService_connect();

/// Disconnect from the Tango Service. Callbacks will no longer be generated.
/// Applications should always call disconnect when the service is no longer
/// needed.
void TangoService_disconnect();

/**@}*/

/// @defgroup pose Tango Service: Pose Interface
/// @brief Functions for getting the pose of the device.
/// @{

/// Attach an onPoseAvailable callback.   The callback is called as new pose
/// updates become available for the registered pair.  When registering the
/// callback, specify the the target and reference frame of interest, and the
/// callback will be called on each change of the pose of that target with
/// respect to that reference frame.  Only some pairs of target/reference are
/// currently supported.  For example TANGO_COORDINATE_FRAME_DEVICE to
/// reference TANGO_COORDINATE_FRAME_START_OF_SERVICE is a typical motion
/// tracking pair to track the motion of the device with reference to its
/// starting position in the "local level" frame of reference (see documentation
/// for details).
/// @param target_frame The target frame of reference, typically
/// TANGO_COORDINATE_FRAME_DEVICE.
/// @param reference_frame The reference frame,
/// for example TANGO_COORDINATE_FRAME_START_OF_SERVICE.
/// @param TangoService_onPoseAvailable function pointer to callback function.
int TangoService_connectOnPoseAvailable(
    void (*TangoService_onPoseAvailable)(const TangoPoseData* pose));

/// Get a pose at a given timestamp from the reference to the target frame.
///
/// @param timestamp Time specified in seconds. If not set to 0.0, getPoseAtTime
/// retrieves the pose closest to this timestamp.  If set to 0.0, the most
/// recent pose estimate for the target-reference pair is returned.  The time
/// of the returned pose is contained in the pose output structure and may
/// differ from the queried timestamp.
///
/// @param frame A pair of coordinate frames of which, the target frame is
/// typically TANGO_COORDINATE_FRAME_DEVICE and the base frame is typically
/// TANGO_COORDINATE_FRAME_START_OF_SERVICE.
/// @param pose The pose of target with respect to reference.  Must be
/// allocated by the caller, and is overwritten upon return.
int TangoService_getPoseAtTime(double timestamp, TangoCoordinateFramePair frame,
                               TangoPoseData* pose);

/// @brief Set the list of TangoCoordinateFramePairs for the onPoseAvailable
/// callbacks.
int TangoService_setPoseListenerFrames(int count,
                                       const TangoCoordinateFramePair *frames);
/**@}*/

/// @defgroup depth Tango Service: Depth Interface
/// @brief Functions for getting depth information from the device.
/// @{

/// Attach an onXYZijAvailable callback.  The callback is called each time new
/// depth data is available, at an approximate nominal period given by the
/// double key "depth_period_in_seconds".
int TangoService_connectOnXYZijAvailable(
    void (*TangoService_onXYZijAvailable)(const TangoXYZij* xyz_ij));

/**@}*/

/// @defgroup camera Tango Service: Camera Interface
/// @brief Functions for getting input from the device's cameras.
/// @{

/// Connect a Texture ID to a camera. The camera is selected via TangoCameraId.
/// Currently only TANGO_CAMERA_COLOR is supported.  The texture must be the ID
/// of a texture that has been allocated and initialized by the calling
/// application.
/// @param id The ID of the camera to connect this texture to.  Only
/// TANGO_CAMERA_COLOR is supported.
/// @param tex The texture ID of the texture to connect the camera to.  Must be
/// a valid texture in the applicaton.
int TangoService_connectTextureId(TangoCameraId id, int tex);

/// Update the texture that has been connected to camera referenced by
/// TangoCameraId. The texture is updated with the latest image from the
/// camera.
/// If timestamp is not NULL, it will be filled with the image timestamp.
/// @param id The ID of the camera to connect this texture to.  Only
/// TANGO_CAMERA_COLOR is supported.
/// @param timestamp Upon return, if not NULL upon calling, timestamp contains
/// the timestamp of the image that has been pushed to the connected texture.
int TangoService_updateTexture(TangoCameraId id, double* timestamp);

/**@}*/

/// Saves the area description, returning the unique ID associated
/// with the saved map.  Will only have an effect after connect has
/// occured, but can be called at any point after that, and can be
/// called repeatedly if desired.  Returns 0 on success, and non-zero
/// on failure.
int TangoService_saveAreaDescription(UUID* uuid);

/// Gets the full list of unique area description IDs available on a
/// device. Allocates memory which should be destroyed by calling
/// TangoService_destroyAreaDescriptionUuidList.  Can be called either
/// before or after connect.  Returns 0 on success, and non-zero on
/// failure.
int TangoService_getAreaDescriptionUUIDList(UUID_list *uuid_list);

/// Destroys the memory allocated by a call to
/// TangoService_getAreaDescriptionUuidList.  Returns 0.
int TangoService_destroyAreaDescriptionUUIDList(UUID_list *uuid_list);

/// Gets the metadata associated with a single area description unique
/// ID.  Allocates memory which should be destroyed by calling
/// TangoService_destroyAreaDescriptionMetadata.
int TangoService_loadAreaDescriptionMetadata(UUID uuid,
                                            Metadata_list* metadata_list);

/// Saves the metadata associated with a single area description unique
/// ID.
int TangoService_saveAreaDescriptionMetadata(UUID uuid,
                                            Metadata_list* metadata_list);

/// Destroys the memory allocated by a call to
/// TangoService_getAreaDescriptionMetadata. Returns 0.
int TangoService_destroyAreaDescriptionMetadata(Metadata_list* metadata_list);

/// Searches through the metadata list for a key that matches the
/// parameter 'key'.  If such a key is found, returns the value_size
/// and value associated with that key.  Returns 0 if the key is
/// found, otherwise returns non-zero.
int TangoAreaDescriptionMetadata_get(
    Metadata_list* metadata_list, char* key, uint32_t* size, char** value);

/// Sets the value associated with an area description key to a new
/// value.  Returns 0 on success, and otherwise returns non-zero.
int TangoAreaDescriptionMetadata_set(
    Metadata_list* metadata_list, char* key, uint32_t value_size, char* value);

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
///     string config_load_area_description_uuid:
///         loads the given Area Description with given UUID and attempt to
///         localize against that Area Description.
///
///     int32 config_experimental_output_pose_format
///         0: (default) Coordinates in Android convention.
///         1: Coordinates in Unity convention.
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

/// Set a boolean configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
int TangoConfig_setBool(TangoConfig *config, const char *key, bool value);

/// Set an int32 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
int TangoConfig_setInt32(TangoConfig *config, const char *key, int32_t value);

/// Set an int64 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
int TangoConfig_setInt64(TangoConfig *config, const char *key, int64_t value);

/// Set a double configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
int TangoConfig_setDouble(TangoConfig *config, const char *key, double value);

/// Set a character string configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
int TangoConfig_setString(TangoConfig *config, const char *key,
                          const char *value);

/// Get a boolean configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
int TangoConfig_getBool(TangoConfig *config, const char *key, bool *value);

/// Get an int32 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
int TangoConfig_getInt32(TangoConfig *config, const char *key, int32_t *value);

/// Get an int64 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
int TangoConfig_getInt64(TangoConfig *config, const char *key, int64_t *value);

/// Get a double configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
int TangoConfig_getDouble(TangoConfig *config, const char *key, double *value);

/// Get a character string configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.  This array must be
/// allocated by the caller.
/// @size The size in bytes of value, as allocated by the caller.  value will be
/// written only up to this size in bytes.
int TangoConfig_getString(TangoConfig *config, const char *key, char *value,
                          size_t size);
/**@}*/

#ifdef __cplusplus
}
#endif

#endif  // TANGO_SERVICE_LIBRARY_TANGO_CLIENT_API_H_
