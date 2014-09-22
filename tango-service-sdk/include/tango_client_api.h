// Copyright 2013 Motorola Mobility LLC. Part of the Trailmix project.
// CONFIDENTIAL. AUTHORIZED USE ONLY. DO NOT REDISTRIBUTE.
#ifndef TANGO_SERVICE_LIBRARY_TANGO_CLIENT_API_H_
#define TANGO_SERVICE_LIBRARY_TANGO_CLIENT_API_H_

#include <stdbool.h>
#include <stdint.h>

/// @file tango_client_api.h
/// @brief File containing Project Tango C API

/// @defgroup enums Project Tango Enumerated Types
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

/// @brief Tango Error types.
/// Errors less then 0 should be dealt with by the program.
/// Success is denoted by TANGO_SUCCESS = 0.
typedef enum {
  TANGO_INVALID = -2, /**< Input argument invalid */
  TANGO_ERROR = -1,   /**< Hard error */
  TANGO_SUCCESS = 0,  /**< Success */
} TangoErrorType;

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
  TANGO_POSE_UNKNOWN            /**< Could not estimate pose at this time */
} TangoPoseStatusType;

/// Tango Event types.
typedef enum {
  TANGO_EVENT_UNKNOWN,        /**< TODO */
  TANGO_EVENT_STATUS_UPDATE,  /**< TODO */
  TANGO_EVENT_ADF_UPDATE,     /**< TODO */
} TangoEventType;

/// Tango Camera Calibration types.
typedef enum {
  TANGO_CALIBRATION_UNKNOWN,
   /**< f-theta fisheye lens model. See
        http://scholar.google.com/scholar?cluster=13508836606423559694&hl=en&as_sdt=2005&sciodt=0,5 */
  TANGO_CALIBRATION_FOV,
  TANGO_CALIBRATION_POLYNOMIAL_2_PARAMETERS,
  /**< Tsai's k1,k2,k3 Model. See
       http://scholar.google.com/scholar?cluster=3512800631607394002&hl=en&as_sdt=0,5&sciodt=0,5 */
  TANGO_CALIBRATION_POLYNOMIAL_3_PARAMETERS,
  TANGO_CALIBRATION_POLYNOMIAL_5_PARAMETERS,
} TangoCalibrationType;

// doxygen does not seem to accept endgroups without the ** comment style
/**@} devsitenav Enumerations */

struct TangoConfig;
typedef struct TangoConfig TangoConfig;

/// @brief The TangoCoordinateFrameType struct contains a pair of coordinate
/// frames of reference.
///
/// Tango pose data is calculated as a transformation between two frames
/// of reference (so, for example, you can be asking for the pose of the
/// device within a learned area).
///
/// This struct is used to specify the desired base and target frames of
/// reference when requesting pose data.  You can also use it when you have
/// a TangoPoseData structure returned from the API and want to examine which
/// frames of reference were used to get that pose.
typedef struct {
  /// Base frame of reference to compare against when requesting pose data.
  /// For example, if you have loaded an area and want to find out where the
  /// device is within it, you would use the
  /// TANGO_COORDINATE_FRAME_AREA_DESCRIPTION frame of reference as your base.
  TangoCoordinateFrameType base;

  /// Target frame of reference when requesting pose data, compared to the
  /// base. For example, if you want the device's pose data, use
  /// TANGO_COORDINATE_FRAME_DEVICE.
  TangoCoordinateFrameType target;
} TangoCoordinateFramePair;

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
  /// The pair of coordinate frames for this pose. We retrieve a pose for a
  /// target coordinate frame (such as the Tango device) against a base
  /// coordinate frame (such as a learned area).
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
  /// The origin is the focal centre of the color camera.
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
  ///
  /// For more information, see our <a href =
  /// "../../overview/depth-perception#xyzij">
  /// developer overview on depth perception</a>.
  int *ij;
} TangoXYZij;

typedef struct TangoImageBuffer {
  /// The width of the image data.
  uint32_t width;
  /// The height of the image data.
  uint32_t height;
  /// The number of pixels per scanline of image data.
  uint32_t stride;
  /// The timestamp of this image.
  double timestamp;
  /// The frame number of this image.
  int64_t frame_number;
  /// Pixels in HAL_PIXEL_FORMAT_YV12 format.
  uint8_t *data;
} TangoImageBuffer;

/// The TangoIntrinsics struct contains intrinsic parameters for a camera.
/// For image coordinates, the obervations, [u, v]^T in pixels.
/// Normalized image plane coordinates refer to:
///
/// x = (u - cx) / fx
///
/// y = (v - cy) / fy
///
/// Distortion model type is as given by calibration_type.  For example, for the
/// color camera, TANGO_CALIBRATION_POLYNOMIAL_3_PARAMETERS means that the
/// distortion parameters are in distortion[] as {k1, k2 ,k3} where
///
/// x_corr_px = x_px (1 + k1 * r2 + k2 * r4 + k3 * r6)
/// y_corr_px = y_px (1 + k1 * r2 + k2 * r4 + k3 * r6)
///
/// where r2, r4, r6 are the 2nd, 4th, and 6th powers of the r, where r is the
/// distance (normalized image plane coordinates) of (x,y) to (cx,cy), and
/// for a pixel at point (x_px, y_px) in pixel coordinates, the corrected output
/// position would be (x_corr, y_corr).
typedef struct TangoIntrinsics {
  /// ID of the camera which the intrinsics reference.
  TangoCameraId camera_id;
  // Calibration model type that the distortion parameters reference.
  TangoCalibrationType calibration_type;

  /// The width of the image in pixels.
  uint32_t width;
  /// The height of the image in pixels.
  uint32_t height;

  /// Focal length, x axis, in pixels.
  double fx;
  /// Focal length, y axis, in pixels.
  double fy;
  /// Principal point x coordinate on the image, in pixels.
  double cx;
  /// Principal point y coordinate on the image, in pixels.
  double cy;

  /// Distortion coefficients, k1, k2, k3 for color image.
  double distortion[5];
} TangoIntrinsics;

#define UUID_LEN 37

/// The UUID struct contains the unique id associated with a single area
/// description.  Should be 36 characters including dashes and a null
/// terminating character.
typedef struct {
  /// ID for an area description.
  char data[UUID_LEN];
} UUID;

/// The UUID_list struct contains a set of area descriptions, generally
/// the area descriptions available for loading on a particular
/// device.
typedef struct {
  /// Number of UUIDs in this list.
  int count;
  /// Array of area description UUIDs.
  UUID* uuid;
} UUID_list;

/// The Metadata_entry struct contains a single key/value pair associated
/// with a single entry in the area description metadata struct.
typedef struct {
  /// Key size.
  size_t key_size;
  /// Metadata key.
  char* key;
  /// Value size.
  size_t value_size;
  /// Value corresponding to key.
  char* value;
} Metadata_entry;

/// The Metadata_list struct contains a set of Metadata_entry objects
/// associated with a single area description.
typedef struct {
  /// Number of entries in this list.
  int num_entries;
  /// Array of metadata key-value entries.
  Metadata_entry* metadata_entries;
} Metadata_list;

/// @brief The TangoEvent structure signals important sensor and tracking
/// events.
/// Each event comes with a timestamp, a type, and a description which
/// describes the event and pertinent information.
///
/// Possible descriptions are:
/// - "Unknown:"
/// - "FisheyeOverExposed:X" - the fisheye image is over exposed with average
/// pixel value X px.
/// - "FisheyeUnderExposed:X" - the fisheye image is under exposed with average
/// pixel value X px.
/// - "ColorOverExposed:X" - the color image is over exposed with average pixel
/// value X px.
/// - "ColorUnderExposed:X" - the color image is under exposed with average
/// pixel value X px.
/// - "TooFewFeaturesTracked:X" - too few features were tracked in the fisheye
/// image, number of features tracked is X.
/// - "ADFEvent:Relocalized" - a relocalization event occurred.
/// - "ADFEvent:NotRelocalized" - a relocalization event has not yet occurred.
/// - "VIOEvent:Initializing" - the tracking system is initializing.
/// - "VIOEvent:Valid" - the tracking system is initialized and pose estimates
/// are currently valid.
/// - "VIOEvent:Invalid" - the tracking system is in a bad state and pose
/// estimates are currently invalid.
typedef struct TangoEvent {
  /// Timestamp, in seconds, of the event.
  double timestamp;
  /// Type of event.
  TangoEventType type;
  /// Description of the event, as listed above.
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
/// uninitialized TangoConfig object. This TangoConfig object must be
/// configured with at least TangoService_getConfig() to initialize it with
/// default values, and then the various TangoConfig_set functions can be used
/// to set specific configuration values.
/// See @link configparams Configuration Parameters @endlink
TangoConfig* TangoConfig_alloc();

/// Deallocate a TangoConfig object.
/// Destroys the TangoConfig object for the handle specified by the config
/// variable.
void TangoConfig_free(TangoConfig* config);

/// Allocates and return a string with one key=value pair per line of all of
/// the configuration values of Tango Service.  Note many of these are
/// 'read-only', unless otherwise documented.
char *TangoConfig_toString(TangoConfig* config);

/**@} devsitenav Configuration Templates */

/// @defgroup lifecycle Tango Service: Lifecycle Interface
/// @brief Functions for initializing, shutting down, and resetting
/// the Tango Service.
/// @{

/// Initializes the TangoService.  This function must be called first before
/// other Tango functions are called.  A connection to the service is created.
/// To succed, the calling application must have camera permissions enabled.
/// The initialization is invalidated on calling TangoService_disconnect() or
/// if the service is stopped or is brought down while a client is running.
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
/// TANGO_ERROR if config_type is not valid, config is NULL, failure to
/// get the requested configuration.
TangoErrorType TangoService_getConfig(TangoConfigType config_type,
    TangoConfig* config);

/// Lock a new configuration.  This will place the service into the
/// configuration given by config.  This will interrupt other client
/// configurations.  The config object must be initialized with
/// TangoService_getConfig().  Upon locking, the service will run the locked
/// configuration on the next call to TangoService_connect() and that
/// configuration will remain valid and immutable until the next call to
/// TangoService_disconnect() after which all configuration of the service
/// is invalidated.
/// @return Returns TANGO_SUCCESS if the configuration was set successfully.
/// Returns TANGO_INVALID if config is NULL, or if the config object was not
/// initialized with TangoService_getConfig().  Returns TANGO_ERROR if the
/// client could not initialize or use a initialized service connection, or if a
/// configuration was not locked.
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
/// TangoService_initialize().
TangoErrorType TangoService_connect(void *callback_context);

/// Disconnect from the Tango Service. Callbacks will no longer be generated.
/// Applications should always call disconnect when the service is no longer
/// needed.  All previous configuration data is invalidated and the service
/// connection is de-initialized, so TangoService_initialize() must be called
/// if the service is to be used again.
void TangoService_disconnect();

/// Sends a reset to the motion tracking system, designed for use
/// after the system has encountered a fault.  If the system is not in
/// a fault state then this command will be ignored.
void TangoService_resetMotionTracking();

/**@} devsitenav Lifecycle */

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
/// starting position in the base frame of reference
/// For more information, see our page on
/// <a href ="../../overview/frames-of-reference">frames of reference</a>.
///
/// @param count The number of base/target pairs to listen to.
/// @param frames The base/targer pairs to listen to.
/// @param TangoService_onPoseAvailable function pointer to callback function.
TangoErrorType TangoService_connectOnPoseAvailable(
    int count, const TangoCoordinateFramePair *frames,
    void (*TangoService_onPoseAvailable)(void *context,
                                         const TangoPoseData *pose));

/// Get a pose at a given timestamp from the base to the target frame.
///
/// All poses returned are marked as VALID (in the status_code) even if they
/// were marked as INITIALIZING in the callback poses.
/// To determine when initialization is complete, register a callback using
/// TangoService_connectOnPoseAvailable and wait until you receive valid data.
///
/// If no pose can be returned, the status_code of the returned pose will be
/// INVALID.
///
/// @param timestamp Time specified in seconds. If not set to 0.0, getPoseAtTime
/// retrieves the interpolated pose closest to this timestamp.  If set to 0.0,
/// the most
/// recent pose estimate for the target-base pair is returned.  The time
/// of the returned pose is contained in the pose output structure and may
/// differ from the queried timestamp.
/// @param frame A pair of coordinate frames specifying the transformation to be
/// queried for. For example, typical device motion is given by a target frame
/// of TANGO_COORDINATE_FRAME_DEVICE and a base frame of
/// TANGO_COORDINATE_FRAME_START_OF_SERVICE.
/// @param pose The pose of target with respect to base frame of reference.
/// Must be allocated by the caller, and is overwritten upon return.
/// @return Returns TANGO_SUCCESS if a pose was returned successfully.  The
/// pose status field should be checked for vailidity.  Returns TANGO_ERROR
/// if no connection was initialized with TangoService_initialize(). Returns
/// TANGO_INVALID if the base and target frame are the same, or if the base or
/// target frame is not valid, or if timestamp is less than 0, or if the service
/// has not yet begun running (TangoService_connext() has not completed).
TangoErrorType TangoService_getPoseAtTime(double timestamp,
    TangoCoordinateFramePair frame, TangoPoseData* pose);

/**@} devsitenav Pose */

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

/**@} devsitenav Depth */

/// @defgroup event Tango Service: Event Notification Interface
/// @brief Functions for getting event notifications from the device.
/// @{

/// Attach an onTangoEvent callback.  The callback is called each time a
/// Tango event happens.
TangoErrorType TangoService_connectOnTangoEvent(
    void (*TangoService_onTangoEvent)(void* context, const TangoEvent* event));

/**@} devsitenav Event Notification */

/// @defgroup camera Tango Service: Camera Interface
/// @brief Functions for getting input from the device's cameras.  Use either
/// TangoService_connectTextureId or TangoService_connectOnFrameAvailable but
/// not both.
/// @{

/// Connect a Texture ID to a camera. The camera is selected via TangoCameraId.
/// Currently only TANGO_CAMERA_COLOR is supported.  The texture must be the ID
/// of a texture that has been allocated and initialized by the calling
/// application.  Note: The first scan-line of the color image is reserved for
/// metadata instead of image pixels.
/// @param id The ID of the camera to connect this texture to.  Only
/// TANGO_CAMERA_COLOR is supported.
/// @param context The context returned during the onFrameAvailable callback.
/// @param tex The texture ID of the texture to connect the camera to.  Must be
/// a valid texture in the applicaton.
TangoErrorType TangoService_connectTextureId(TangoCameraId id, unsigned int tex,
                                             void *context,
                                             void (*callback)(void *,
                                                              TangoCameraId));

/// Update the texture that has been connected to camera referenced by
/// TangoCameraId. The texture is updated with the latest image from the
/// camera.
/// If timestamp is not NULL, it will be filled with the image timestamp.
/// @param id The ID of the camera to connect this texture to.  Only
/// TANGO_CAMERA_COLOR is supported.
/// @param timestamp Upon return, if not NULL upon calling, timestamp contains
/// the timestamp of the image that has been pushed to the connected texture.
TangoErrorType TangoService_updateTexture(TangoCameraId id, double* timestamp);

/// Connect a callback to a camera for access to the pixels. This is not
/// recommended for display but for applications requiring access to the
/// HAL_PIXEL_FORMAT_YV12 pixel data.  The camera is selected via
/// TangoCameraId.  Currently only TANGO_CAMERA_COLOR is supported.  The
/// onFrameAvailable callback will be called when a new frame of is available
/// from the camera.
/// Note: The first scan-line of the color image is reserved for metadata
/// instead of image pixels.
/// @param id The ID of the camera to connect this texture to.  Only
/// TANGO_CAMERA_COLOR is supported.
/// @param context The context returned during the onFrameAvailable callback.
/// @param onFrameAvailable Function called when a new frame is available
/// from the camera.
TangoErrorType TangoService_connectOnFrameAvailable(
    TangoCameraId id, void *context,
    void (*onFrameAvailable)(void *context, TangoCameraId id,
                             const TangoImageBuffer *buffer));

/// Get the intrinsic calibration parameters for a given camera.  The intrinsics
/// are as specified by the TangoIntrinsics struct.
/// @param camera_id The camera ID to retrieve the calibration intrinsics for.
/// @param intrinsics A TangoIntrinsics struct that must be allocated before
/// calling, and is filled with calibration intrinsics for the camera camera_id
/// upon successful return.
/// @return Returns TANGO_SUCCESS on successfully retrieving calibration
/// intrinsics.  Returns TANGO_ERROR if service could not be connected to or
/// intrinsics could not be found and TANGO_INVALID if inputs are invalid.
TangoErrorType TangoService_getCameraIntrinsics(TangoCameraId camera_id,
    TangoIntrinsics* intrinsics);

/**@} devsitenav Camera */

/// @defgroup adf Tango Service: Area Description Interface
/// @brief Functions for handling Area Descriptions for localization.
/// Note that loading an Area Description is handled by specifying the
/// configuration item config_load_area_description_UUID when the configuration
/// is locked.  This is done using the TangoConfig_setString() function;
/// see @link configparams Configuration Parameters @endlink for more info.
/// @{

/// Saves the area description, returning the unique ID associated
/// with the saved map.  Will only have an effect after connect has
/// occurred, but can be called at any point after that, and can be
/// called repeatedly if desired.
/// @param uuid Upon saving, the generated UUID to refer to this map is
/// returned in uuid.
/// @return Returns TANGO_SUCCESS on success, and TANGO_ERROR if a failure
/// occurred when saving, or if the service needs to be initialized, or
/// TANGO_INVALID if uuid is NULL, or of incorrect length.
TangoErrorType TangoService_saveAreaDescription(UUID* uuid);

/// Deletes an area description with the specified unique ID.  This
/// method can be called after TangoService_initialize(), but should
/// not be called to delete the ADF that is currently loaded.
/// @param uuid The area description to delete.
/// @return Returns TANGO_SUCCESS if area description file with
/// specified unique ID is found and can be removed.  Returns
/// TANGO_ERROR on failure to delete, or if the service needs to be initialized.
TangoErrorType TangoService_deleteAreaDescription(UUID uuid);

/// Gets the full list of unique area description IDs available on a
/// device. Allocates memory which should be destroyed by calling
/// TangoService_destroyAreaDescriptionUuidList.  Can be called either
/// before or after connect.
/// @param uuid_list Upon successful return, uuid_list is filled with a
/// UUID_list structure of available UUIDs.
/// @return Returns TANGO_SUCCESS on success, or TANGO_ERROR on failure to
/// retrieve the list, or if the service needs to be initialized, or
/// TANGO_INVALID if the uuid_list argument was NULL.
TangoErrorType TangoService_getAreaDescriptionUUIDList(UUID_list *uuid_list);

/// Destroys the memory allocated by a call to
/// TangoService_getAreaDescriptionUuidList.
/// @param uuid_list The UUID_list to be deallocated.
/// @return Returns TANGO_SUCCESS if the list was deleted, TANGO_ERROR if the
/// service needs to be initialized, or TANGO_INVALID if the uuid_list was NULL.
TangoErrorType TangoService_destroyAreaDescriptionUUIDList(
    UUID_list *uuid_list);

/// Gets the metadata associated with a single area description unique
/// ID.  Allocates memory which should be destroyed by calling
/// TangoService_destroyAreaDescriptionMetadata.
/// @param uuid The UUID for which to load the metadata.
/// @param metadata_list The metadata for the uuid.
/// @return Returns TANGO_SUCCESS on successful load of metadata, or TANGO_ERROR
/// if the service needs to be initialized or if the metadata could not be
/// loaded, or TANGO_INVALID if metadata_list was NULL.
TangoErrorType TangoService_loadAreaDescriptionMetadata(UUID uuid,
    Metadata_list* metadata_list);

/// Saves the metadata associated with a single area description unique ID.
/// @param uuid The UUID for which to save the meta data of.
/// @param metadata_list The metadata to be saved for the uuid.
/// @return Returns TANGO_SUCCESS on successful save, or TANGO_ERROR on failure,
/// or if the service needs to be initialized, or TANGO_INVALID if metadata_list
/// was NULL.
TangoErrorType TangoService_saveAreaDescriptionMetadata(UUID uuid,
    Metadata_list* metadata_list);

/// Destroys the memory allocated by a call to
/// TangoService_getAreaDescriptionMetadata.
/// @param metadata_list The handle to the metadata to be deallocated.
/// @return Returns TANGO_SUCCESS if the metadata was deleted.
TangoErrorType TangoService_destroyAreaDescriptionMetadata(
    Metadata_list* metadata_list);

/// Import an area description from the source file path to the default
/// area storage location and rename the area with its uuid.
/// @param uuid To store the uuid of the area.
/// @param src_file_path The source file path of the area to be imported.
/// @return Returns TANGO_SUCCESS on successful import, or TANGO_ERROR if the
/// file could not be imported, or TANGO_INVALID if uuid or src_file_path was
/// NULL
TangoErrorType TangoService_importAreaDescription(UUID* uuid,
                                                  char* src_file_path);

/// Export an area with the uuid from the default area storage location to
/// the destination file directory with the uuid as its name.
/// @param uuid the uuid of the area.
/// @param dst_file_dir The destination file directory.
/// @return Returns TANGO_SUCCESS if the file was exported, or TANGO_ERROR if
/// the export failed, or TANGO_INVALID if dst_file_dir was NULL.
TangoErrorType TangoService_exportAreaDescription(UUID uuid,
                                                  char* dst_file_dir);

/// Searches through the metadata list for a key that matches the parameter
/// 'key'.  If such a key is found, returns the value_size and value associated
/// with that key.
/// @param metadata_list The metadata list to search through.
/// @param key The string key value of the parameter to set.
/// @param size The size in bytes of value, as allocated by the
/// caller.  value will be written only up to this size in bytes.
/// @param value value The value to set the key to.  This array must
/// be allocated by the caller.
/// @return Returns TANGO_SUCCESS if the key is found, otherwise returns
/// TANGO_INVALID if the key does not exists, or if any of the arguments is
/// NULL.
TangoErrorType TangoAreaDescriptionMetadata_get(
    Metadata_list* metadata_list, char* key, uint32_t* size, char** value);

/// Sets the value associated with an area description key to a new
/// value.  Returns 0 on success, and otherwise returns non-zero.
/// @param metadata_list The metadata list to set the key-value pair in.
/// @param key The string key value of the parameter to set.
/// @param value_size The size in bytes of value, as allocated by the
/// caller.  value will be written only up to this size in bytes.
/// @param value The value to set the key to.
/// @return Returns TANGO_SUCCESS if the key is set, otherwise returns
/// TANGO_INVALID if any of the arguments is NULL.
TangoErrorType TangoAreaDescriptionMetadata_set(
    Metadata_list* metadata_list, char* key, uint32_t value_size, char* value);

/** @} devsitenav Area Description */

/// @defgroup configparams Configuration Parameters Get and Set Functions
/// @brief Configuration Parameters Get and Set Functions
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
/// -   int32 config_color_iso: ISO value for the color camera.  For example use
///         100, 200, or 400.  Default is  100.
///
/// -   int32 config_color_exp: Exposure value for the color camera, in
///         nanoseconds.  Default is 11100000 (11.1 ms).
///
/// -   boolean config_enable_auto_reset:
///         If tracking is lost, this will automatically reset the tracking and
///         attempt to reinitialize. If an area description is loaded, or if
///         learning mode is enabled, this will also automatically try to
///         localize. Defaults to true.
///
/// -   boolean config_enable_depth:
///         Enables depth output if true.  Defaults to false.
///
/// -   boolean config_enable_learning_mode:
///         Enables learning mode if true.  Defaults to false.
///
/// -   boolean config_enable_motion_tracking:
///         Enables motion tracking if true.  Defaults to true.
///
/// -   boolean config_enable_dataset_recording:
///         Enables recording of a dataset to disk.
///
/// -   string config_load_area_description_UUID:
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
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if config or key
/// is NULL, or key is not found or could not be set.
TangoErrorType TangoConfig_setBool(TangoConfig *config, const char *key,
    bool value);

/// Set an int32 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if config or key
/// is NULL, or key is not found or could not be set.
TangoErrorType TangoConfig_setInt32(TangoConfig *config, const char *key,
    int32_t value);

/// Set an int64 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if config or key
/// is NULL, or key is not found or could not be set.
TangoErrorType TangoConfig_setInt64(TangoConfig *config, const char *key,
    int64_t value);

/// Set a double configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if config or key
/// is NULL, or key is not found or could not be set.
TangoErrorType TangoConfig_setDouble(TangoConfig *config, const char *key,
    double value);

/// Set a character string configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if config or key
/// is NULL, or key is not found or could not be set.
TangoErrorType TangoConfig_setString(TangoConfig *config, const char *key,
    const char *value);

/// Get a boolean configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if the any of the
/// arguments is NULL, or if the key could not be found.
TangoErrorType TangoConfig_getBool(TangoConfig *config, const char *key,
    bool *value);

/// Get an int32 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if the any of the
/// arguments is NULL, or if the key could not be found.
TangoErrorType TangoConfig_getInt32(TangoConfig *config, const char *key,
    int32_t *value);

/// Get an int64 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if the any of the
/// arguments is NULL, or if the key could not be found.
TangoErrorType TangoConfig_getInt64(TangoConfig *config, const char *key,
    int64_t *value);

/// Get a double configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if the any of the
/// arguments is NULL, or if the key could not be found.
TangoErrorType TangoConfig_getDouble(TangoConfig *config, const char *key,
    double *value);

/// Get a character string configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been allocated with TangoConfig_alloc() and should have been
/// initialized with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.  This array must be
/// allocated by the caller.
/// @param size The size in bytes of value, as allocated by the caller.  value
/// will be written only up to this size in bytes.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if the any of the
/// arguments is NULL, or if the key could not be found.
TangoErrorType TangoConfig_getString(TangoConfig *config, const char *key,
    char *value, size_t size);
/**@} devsitenav Config Params */

#ifdef __cplusplus
}
#endif

#endif  // TANGO_SERVICE_LIBRARY_TANGO_CLIENT_API_H_
