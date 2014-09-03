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
  /// with reference to the base frame.
  /// Specified as (x,y,z,w) where RotationAngle is in radians:
  /// x = RotationAxis.x * sin(RotationAngle / 2)
  /// y = RotationAxis.y * sin(RotationAngle / 2)
  /// z = RotationAxis.z * sin(RotationAngle / 2)
  /// w = cos(RotationAngle / 2)
  double orientation[4];

  /// Translation, ordered x, y, z, of the pose of the target frame
  /// with reference to the base frame.
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
  /**< Average image pixel value of the fisheye camera is over the value of
       config flag sensor_status_max_pixel_value. */
  TANGO_STATUS_FOV_OVER_EXPOSED,
  /**< Average image pixel value of the fisheye camera is under the value of
       sensor_status_min_pixel_value. */
  TANGO_STATUS_FOV_UNDER_EXPOSED,
  /**< Number of features being tracked is less than the value of the config
       flag sensor_status_min_features. */
  TANGO_STATUS_TOO_FEW_FEATURES_TRACKED,
  /**< Average image pixel value of the color camera is over the value of
       config flag sensor_status_max_pixel_value. */
  TANGO_STATUS_COLOR_OVER_EXPOSED,
  /**< Average image pixel value of the color camera is under the value of
       sensor_status_min_pixel_value. */
  TANGO_STATUS_COLOR_UNDER_EXPOSED,
  TANGO_STATUS_COUNT
} TangoStatusType;

typedef struct TangoEvent {
  double timestamp;
  TangoEventType type;
  const char *description;
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
/// configure, and start, and reset the Tango Service.
/// Conceptually they are grouped into the following functional groupings:
///
/// Lifecycle Interface
/// Pose Interface
/// Depth Interface
/// Camera Interface

/// @defgroup lifecycle Tango Service: Lifecycle Interface
/// @brief Functions for initializing, shutting down, and resetting
/// the Tango Service.
/// @{

/// Initializes the TangoService.  This function must be called first before
/// other Tango functions are called.  A connection to the service is created.
/// To succed, the calling application must have camera permissions enabled.
/// @return Returns TANGO_SUCCESS if a connection was made or exists.  Returns
/// TANGO_ERROR if a connection could not be made.
TangoErrorType TangoService_initialize();

/// Given a TangoConfig object that was allocated with TangoConfig_alloc(),
/// TangoService_getConfig() fills in Config with configuration
/// settings from the service.  This should be used to initialize a Config
/// object before setting custom settings and locking that configuration.  This
/// function can also be used to find the current configuration of the service.
///    TANGO_CONFIG_DEFAULT = Default configuration of the service.
/// @return On successful setting of config, returns TANGO_SUCCESS.  Returns
/// TANGO_ERROR if config_type is not valid, config is NULL, or failure to
/// get the requested configuration.
TangoErrorType TangoService_getConfig(TangoConfigType config_type,
    TangoConfig* config);

/// Lock a new configuration.  This will place the service into the
/// configuration given by config.  This will interrupt other client
/// configurations.  The config object must be initialized with
/// TangoService_getConfig().
/// @return Returns TANGO_SUCCESS if the configuration was set successfully.
/// Returns TANGO_ERROR if config is NULL, or if a connection was not
/// initialized with TangoService_initialize(), or if a configuration was not
/// locked.
TangoErrorType TangoService_lockConfig(TangoConfig* config);

/// Unlock the Tango Service.   This function will unlock the Tango Services'
/// configuration.  It should be called when the service's specific
/// configuration is no longer needed, such as when shutting down.  This will
/// allow other clients to change the configuration of the service.
/// @return Returns TANGO_SUCCESS on unlocking a configuration.  Returns
/// TANGO_ERROR if a connection was not initialized with
/// TangoService_initialize(), or if the configuration was not unlocked.
TangoErrorType TangoService_unlockConfig();

/// Connect to the Tango Service.   This connects to and starts the service
/// running.  The service will run with the current configuration, in either
/// its default configuration, or most recently locked configuration if new
/// configuration values were set.  TangoService_connect() starts the motion
/// estimation and other components of the service.  After calling
/// TangoService_connect(), callbacks will begin to be generated, and pose can
/// be queried, and other data (camera, depth) will become available.
/// @return Returns TANGO_SUCCESS on successfully starting the configuration.
/// Returns TANGO_ERROR on failure, or if a connection was not initialized with
/// TangService_initialize().
TangoErrorType TangoService_connect(void *callback_context);

/// Disconnect from the Tango Service. Callbacks will no longer be generated.
/// Applications should always call disconnect when the service is no longer
/// needed.
void TangoService_disconnect();

/// Sends a reset to the motion tracking system, designed for use
/// after the system has encountered a fault.  If the system is not in
/// a fault state then this command will be ignored.
void TangoService_resetMotionTracking();

/**@}*/

/// @defgroup pose Tango Service: Pose Interface
/// @brief Functions for getting the pose of the device.
/// @{

/// Attach an onPoseAvailable callback.   The callback is called as new pose
/// updates become available for the registered pair.  When registering the
/// callback, specify the the target and base frame of interest, and the
/// callback will be called on each change of the pose of that target with
/// reference to that base frame.  Only some pairs of base/target are
/// currently supported.  For example TANGO_COORDINATE_FRAME_DEVICE to
/// base TANGO_COORDINATE_FRAME_START_OF_SERVICE is a typical motion
/// tracking pair to track the motion of the device with reference to its
/// starting position in the base frame of reference (see documentation
/// for details).
/// @param count The number of base/target pairs to listen to.
/// @param frames The base/targer pairs to listen to.
/// @param TangoService_onPoseAvailable function pointer to callback function.
TangoErrorType TangoService_connectOnPoseAvailable(
    int count, const TangoCoordinateFramePair *frames,
    void (*TangoService_onPoseAvailable)(void *context,
                                         const TangoPoseData *pose));

/// Get a pose at a given timestamp from the base to the target frame.
///
/// @param timestamp Time specified in seconds. If not set to 0.0, getPoseAtTime
/// retrieves the pose closest to this timestamp.  If set to 0.0, the most
/// recent pose estimate for the target-base pair is returned.  The time
/// of the returned pose is contained in the pose output structure and may
/// differ from the queried timestamp.
///
/// @param frame A pair of coordinate frames of which, the target frame is
/// typically TANGO_COORDINATE_FRAME_DEVICE and the base frame is typically
/// TANGO_COORDINATE_FRAME_START_OF_SERVICE.
/// @param pose The pose of target with respect to base frame of reference.
/// Must be allocated by the caller, and is overwritten upon return.
/// @return Returns TANGO_SUCCESS if a pose was returned successfully.  The
/// pose status field should be checked for vailidity.  Returns TANGO_ERROR
/// if no connection was initialized with TangoService_initailize(), or if the
/// base and target frame are the same, or if the base or target frame are not
/// valid, or if timestamp is less than 0.
TangoErrorType TangoService_getPoseAtTime(double timestamp,
    TangoCoordinateFramePair frame, TangoPoseData* pose);

/**@}*/

/// @defgroup depth Tango Service: Depth Interface
/// @brief Functions for getting depth information from the device.
/// @{

/// Attach an onXYZijAvailable callback.  The callback is called each time new
/// depth data is available, at an approximate nominal period given by the
/// double key "depth_period_in_seconds".
/// @return Returns TANGO_ERROR if the listener function pointer is NULL.
/// Returns TANGO_SUCCESS otherwise.
TangoErrorType TangoService_connectOnXYZijAvailable(
    void (*TangoService_onXYZijAvailable)(
    void* context, const TangoXYZij* xyz_ij));

/**@}*/

/// @defgroup event Tango Service: Event Notification Interface
/// @brief Functions for getting event notifications from the device.
/// @{

/// Attach an onTangoEvent callback.  The callback is called each time a
/// Tango event happens.
TangoErrorType TangoService_connectOnTangoEvent(
    void (*TangoService_onTangoEvent)(void* context, const TangoEvent* event));

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
TangoErrorType TangoService_connectTextureId(TangoCameraId id, int tex);

/// Update the texture that has been connected to camera referenced by
/// TangoCameraId. The texture is updated with the latest image from the
/// camera.
/// If timestamp is not NULL, it will be filled with the image timestamp.
/// @param id The ID of the camera to connect this texture to.  Only
/// TANGO_CAMERA_COLOR is supported.
/// @param timestamp Upon return, if not NULL upon calling, timestamp contains
/// the timestamp of the image that has been pushed to the connected texture.
TangoErrorType TangoService_updateTexture(TangoCameraId id, double* timestamp);

/**@}*/


/// @defgroup ADF Tango Service: Area Description Interface
/// @brief Functions for handling Area Descriptions for localization.
/// Note that loading an Area Description is handled by specifying the
/// configuration item config_load_area_description_UUID when the configration
/// is locked.
/// @{

/// Saves the area description, returning the unique ID associated
/// with the saved map.  Will only have an effect after connect has
/// occured, but can be called at any point after that, and can be
/// called repeatedly if desired.
/// @param uuid Upon saving, the generated UUID to refer to this map is
/// returned in uuid.
/// @return Returns TANGO_SUCCESS on success, and TANGO_ERROR on failure, or if
/// uuid is NULL.
TangoErrorType TangoService_saveAreaDescription(UUID* uuid);

/// Gets the full list of unique area description IDs available on a
/// device. Allocates memory which should be destroyed by calling
/// TangoService_destroyAreaDescriptionUuidList.  Can be called either
/// before or after connect.
/// @param uuid_list Upon successful return, uuid_list is filled with a
/// UUID_list structure of available UUIDs.
/// @return Returns TANGO_SUCCESS on success, and TANGO_ERROR on failure.
TangoErrorType TangoService_getAreaDescriptionUUIDList(UUID_list *uuid_list);

/// Destroys the memory allocated by a call to
/// TangoService_getAreaDescriptionUuidList.
/// @param uuid_list The UUID_list to be deallocated.
/// @return Returns TANGO_SUCCESS.
TangoErrorType TangoService_destroyAreaDescriptionUUIDList(
    UUID_list *uuid_list);

/// Gets the metadata associated with a single area description unique
/// ID.  Allocates memory which should be destroyed by calling
/// TangoService_destroyAreaDescriptionMetadata.
/// @param uuid The UUID for which to load the metadata.
/// @param metadata_list The metadata for the uuid.
/// @return Returns TANGO_SUCCESS on successful load of metadata.
TangoErrorType TangoService_loadAreaDescriptionMetadata(UUID uuid,
    Metadata_list* metadata_list);

/// Saves the metadata associated with a single area description unique ID.
/// @param uuid The UUID for which to save the meta data of.
/// @param metadata_list The metadata to be saved for the uuid.
/// @return Returns TANGO_SUCCESS on successful save, or TANGO_ERROR on failure.
TangoErrorType TangoService_saveAreaDescriptionMetadata(UUID uuid,
    Metadata_list* metadata_list);

/// Destroys the memory allocated by a call to
/// TangoService_getAreaDescriptionMetadata.
/// @param metdata_list The handle to the metadata to be deallocated.
/// @return Returns TANGO_SUCCESS.
TangoErrorType TangoService_destroyAreaDescriptionMetadata(
    Metadata_list* metadata_list);

/// Searches through the metadata list for a key that matches the parameter
/// 'key'.  If such a key is found, returns the value_size and value associated
/// with that key.
/// @param key The string key value of the parameter to set.
/// @param size The size in bytes of value, as allocated by the
/// caller.  value will be written only up to this size in bytes.
/// @param value value The value to set the key to.  This array must
/// be allocated by the caller.
/// @return Returns TANGO_SUCCESS if the key is found, otherwise returns
/// TANGO_ERROR.
TangoErrorType TangoAreaDescriptionMetadata_get(
    Metadata_list* metadata_list, char* key, uint32_t* size, char** value);

/// Sets the value associated with an area description key to a new
/// value.  Returns 0 on success, and otherwise returns non-zero.
/// @param key The string key value of the parameter to set.
/// @param size The size in bytes of value, as allocated by the
/// caller.  value will be written only up to this size in bytes.
/// @param value The value to set the key to.
/// @return Returns TANGO_SUCCESS if the key is set, otherwise returns
/// TANGO_ERROR.
TangoErrorType TangoAreaDescriptionMetadata_set(
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
///     boolean config_enable_auto_reset:
///         If tracking is lost, this will automatically reset the tracking and
///         attempt to reinitialize. If an area description is loaded, or if
///         learning mode is enabled, this will also automatically try to
///         localize. Defaults to true.
///
///     boolean config_enable_depth:
///         Enables depth output if true.  Defaults to false.
///
///     boolean config_enable_learning_mode:
///         Enables learning mode if true.  Defaults to false.
///
///     boolean config_enable_motion_tracking:
///         Enables motion tracking if true.  Defaults to true.
///
///     boolean config_enable_dataset_recording:
///         Enables recording of a dataset to disk.
///
///     string config_load_area_description_uuid:
///         Loads the given Area Description with given UUID and attempts to
///         localize against that Area Description.  Empty string will disable
///         localization.  Defaults to empty.
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
/// @return Returns TANGO_ERROR if the key is not found, and TANGO_SUCCESS
/// on success.
TangoErrorType TangoConfig_setBool(TangoConfig *config, const char *key,
    bool value);

/// Set an int32 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
/// @return Returns TANGO_ERROR if the key is not found, and TANGO_SUCCESS
/// on success.
TangoErrorType TangoConfig_setInt32(TangoConfig *config, const char *key,
    int32_t value);

/// Set an int64 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
/// @return Returns TANGO_ERROR if the key is not found, and TANGO_SUCCESS
/// on success.
TangoErrorType TangoConfig_setInt64(TangoConfig *config, const char *key,
    int64_t value);

/// Set a double configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
/// @return Returns TANGO_ERROR if the key is not found, and TANGO_SUCCESS
/// on success.
TangoErrorType TangoConfig_setDouble(TangoConfig *config, const char *key,
    double value);

/// Set a character string configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
/// @return Returns TANGO_ERROR if the key is not found, and TANGO_SUCCESS
/// on success.
TangoErrorType TangoConfig_setString(TangoConfig *config, const char *key,
    const char *value);

/// Get a boolean configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
/// @return Returns TANGO_ERROR if the key is not found, and TANGO_SUCCESS
/// on success.
TangoErrorType TangoConfig_getBool(TangoConfig *config, const char *key,
    bool *value);

/// Get an int32 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
/// @return Returns TANGO_ERROR if the key is not found, and TANGO_SUCCESS
/// on success.
TangoErrorType TangoConfig_getInt32(TangoConfig *config, const char *key,
    int32_t *value);

/// Get an int64 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
/// @return Returns TANGO_ERROR if the key is not found, and TANGO_SUCCESS
/// on success.
TangoErrorType TangoConfig_getInt64(TangoConfig *config, const char *key,
    int64_t *value);

/// Get a double configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.
/// @return Returns TANGO_ERROR if the key is not found, and TANGO_SUCCESS
/// on success.
TangoErrorType TangoConfig_getDouble(TangoConfig *config, const char *key,
    double *value);

/// Get a character string configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @value value The value to set the configuration key to.  This array must be
/// allocated by the caller.
/// @size The size in bytes of value, as allocated by the caller.  value will be
/// written only up to this size in bytes.
/// @return Returns TANGO_ERROR if the key is not found, and TANGO_SUCCESS
/// on success.
TangoErrorType TangoConfig_getString(TangoConfig *config, const char *key,
    char *value, size_t size);
/**@}*/

#ifdef __cplusplus
}
#endif

#endif  // TANGO_SERVICE_LIBRARY_TANGO_CLIENT_API_H_
