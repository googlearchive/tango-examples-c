// Copyright 2014 Google Inc. All Rights Reserved.
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
#ifndef TANGO_CLIENT_API_H_
#define TANGO_CLIENT_API_H_

#include <jni.h>
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
  TANGO_CONFIG_DEFAULT = 0,     /**< Default, motion tracking only. */
  TANGO_CONFIG_CURRENT,         /**< Current */
  TANGO_CONFIG_MOTION_TRACKING, /**< Motion tracking */
  TANGO_CONFIG_AREA_LEARNING,   /**< Area learning */
  TANGO_MAX_CONFIG_TYPE         /**< Maximum number allowable.  */
} TangoConfigType;

/// @brief Tango coordinate frame enumerations.
typedef enum {
  /** Coordinate system for the entire Earth.
   *  See WGS84: http://en.wikipedia.org/wiki/World_Geodetic_System
   */
  TANGO_COORDINATE_FRAME_GLOBAL_WGS84 = 0,
  /** Origin within a saved area description */
  TANGO_COORDINATE_FRAME_AREA_DESCRIPTION,
  /** Origin when the device started tracking */
  TANGO_COORDINATE_FRAME_START_OF_SERVICE,
  /** Immediately previous device pose */
  TANGO_COORDINATE_FRAME_PREVIOUS_DEVICE_POSE,
  TANGO_COORDINATE_FRAME_DEVICE,         /**< Device coordinate frame */
  TANGO_COORDINATE_FRAME_IMU,            /**< Inertial Measurement Unit */
  TANGO_COORDINATE_FRAME_DISPLAY,        /**< Display */
  TANGO_COORDINATE_FRAME_CAMERA_COLOR,   /**< Color camera */
  TANGO_COORDINATE_FRAME_CAMERA_DEPTH,   /**< Depth camera */
  TANGO_COORDINATE_FRAME_CAMERA_FISHEYE, /**< Fisheye camera */
  TANGO_COORDINATE_FRAME_INVALID,
  TANGO_MAX_COORDINATE_FRAME_TYPE /**< Maximum allowed */
} TangoCoordinateFrameType;

/// @brief Tango Error types.
/// Errors less then 0 should be dealt with by the program.
/// Success is denoted by TANGO_SUCCESS = 0.
typedef enum {
  /// The user has not given permission to export or import ADF files.
  TANGO_NO_IMPORT_EXPORT_PERMISSION = -6,
  /// The user has not given permission to access the device's camera.
  TANGO_NO_CAMERA_PERMISSION = -5,
  /// The user has not given permission to save or change ADF files.
  TANGO_NO_ADF_PERMISSION = -4,
  /// The user has not given permission to use Motion Tracking functionality.
  TANGO_NO_MOTION_TRACKING_PERMISSION = -3,
  /// The input argument is invalid.
  TANGO_INVALID = -2,
  /// This error code denotes some sort of hard error occurred.
  TANGO_ERROR = -1,
  /// This code indicates success.
  TANGO_SUCCESS = 0,
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
  TANGO_EVENT_UNKNOWN = 0, /**< Unclassified Event Type */
  TANGO_EVENT_GENERAL,     /**< General callbacks not otherwise categorized */
  TANGO_EVENT_FISHEYE_CAMERA,   /**< Fisheye Camera Event */
  TANGO_EVENT_COLOR_CAMERA,     /**< Color Camera Event */
  TANGO_EVENT_IMU,              /**< IMU Event */
  TANGO_EVENT_FEATURE_TRACKING, /**< Feature Tracking Event */
} TangoEventType;

/// Tango Camera Calibration types.
typedef enum {
  TANGO_CALIBRATION_UNKNOWN,
  /** f-theta fisheye lens model. See
       http://scholar.google.com/scholar?cluster=13508836606423559694&hl=en&as_sdt=2005&sciodt=0,5
    */
  TANGO_CALIBRATION_EQUIDISTANT,
  TANGO_CALIBRATION_POLYNOMIAL_2_PARAMETERS,
  /** Tsai's k1,k2,k3 Model. See
       http://scholar.google.com/scholar?cluster=3512800631607394002&hl=en&as_sdt=0,5&sciodt=0,5
     */
  TANGO_CALIBRATION_POLYNOMIAL_3_PARAMETERS,
  TANGO_CALIBRATION_POLYNOMIAL_5_PARAMETERS,
} TangoCalibrationType;

/// Tango Image Formats
///
/// Equivalent to those found in Android core/system/include/system/graphics.h.
typedef enum {
  TANGO_HAL_PIXEL_FORMAT_YV12 = 0x32315659  // YCrCb 4:2:0 Planar
} TangoImageFormatType;

/**@} devsitenav Enumerations */

/// @defgroup types Project Tango Defined Types
/// @brief Basic types being used in the Project Tango API.
/// @{

/// This defines a handle to TangoConfig; key/value pairs can only be accessed
/// through API calls.
typedef void* TangoConfig;

#define TANGO_UUID_LEN 37

/// The unique id associated with a single area description.  Should
/// be 36 characters including dashes and a null terminating
/// character.
typedef char TangoUUID[TANGO_UUID_LEN];

/// This defines a handle to TangoAreaDescriptionMetadata; key/value pairs
/// can only be accessed through API calls.
typedef void* TangoAreaDescriptionMetadata;

/**@} devsitenav Typedefs */

/// @brief The TangoCoordinateFramePair struct contains a pair of coordinate
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
  uint32_t version;

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
  uint32_t confidence;   // Unused.  Integer levels are determined by service.
  float accuracy;        // Unused.  Reserved for metric accuracy.
} TangoPoseData;

/// The TangoImageBuffer contains information about a byte buffer holding
/// image data.
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
  /// Pixel format of data.
  TangoImageFormatType format;
  /// Pixels in HAL_PIXEL_FORMAT_YV12 format. Y samples of width x height are
  /// first, followed by V samples, with half the stride and half the lines of
  /// the Y data, followed by a U samples with the same dimensions as the V
  /// sample.
  uint8_t* data;
} TangoImageBuffer;

/// The TangoXYZij struct contains information returned from the depth sensor.
typedef struct TangoXYZij {
  /// An integer denoting the version of the structure.
  uint32_t version;

  /// Time of capture of the depth data for this struct (in seconds).
  double timestamp;

  /// The number of points in depth_data_buffer populated successfully
  /// is variable with each call to the function, and is returned in
  /// (x,y,z) triplets populated (e.g. 2 points populated returned means 6
  /// floats, or 6*4 bytes used).
  uint32_t xyz_count;

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
  uint32_t ij_rows;
  /// The dimensions of the ij index buffer.
  uint32_t ij_cols;

  /// A 2D buffer, of size ij_rows x ij_cols in raster ordering that contains
  /// the index of a point in the xyz array that was generated at this "ij"
  /// location.  A value of -1 denotes there was no corresponding point
  /// generated at that position. This buffer can be used to find neighbouring
  /// points in the point cloud.
  ///
  /// For more information, see our <a href =
  /// "../../overview/depth-perception#xyzij">
  /// developer overview on depth perception</a>.
  uint32_t* ij;

  /// TangoImageBuffer is reserved for future use.
  TangoImageBuffer* color_image;
} TangoXYZij;

/// The TangoCameraIntrinsics struct contains intrinsic parameters for a camera.
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
typedef struct TangoCameraIntrinsics {
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
} TangoCameraIntrinsics;

/// @brief The TangoEvent structure signals important sensor and tracking
/// events.
/// Each event comes with a timestamp, a type, and a key-value pair describing
/// the event.  The type is an enumeration which generally classifies the event
/// type. The key is a text string describing the event.  The description holds
/// parameters specific to the event.
///
/// Possible descriptions (as "key:value") are:
/// - "TangoServiceException:X" - The service has encountered an exception, and
/// a text description is given in X.
/// - "FisheyeOverExposed" - the fisheye image is over exposed with average
/// pixel value X px.
/// - "FisheyeUnderExposed:X" - the fisheye image is under exposed with average
/// pixel value X px.
/// - "ColorOverExposed:X" - the color image is over exposed with average pixel
/// value X px.
/// - "ColorUnderExposed:X" - the color image is under exposed with average
/// pixel value X px.
/// - "TooFewFeaturesTracked:X" - too few features were tracked in the fisheye
/// image.  The number of features tracked is X.
/// - "Unknown"
typedef struct TangoEvent {
  /// Timestamp, in seconds, of the event.
  double timestamp;
  /// Type of event.
  TangoEventType type;
  /// Description of the event key.
  const char* event_key;
  /// Description of the event value.
  const char* event_value;
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

/// Free a TangoConfig object.
/// Frees the TangoConfig object for the handle specified by the config
/// variable.
void TangoConfig_free(TangoConfig config);

/// Allocates and return a string with one key=value pair per line of all of
/// the configuration values of Tango Service.  Note many of these are
/// 'read-only', unless otherwise documented.
char* TangoConfig_toString(TangoConfig config);

/**@} devsitenav Configuration Templates */

/// @defgroup lifecycle Tango Service: Lifecycle Interface
/// @brief Functions for initializing, shutting down, and resetting
/// the Tango Service.
/// @{

/// Initializes the TangoService. This function must be called first before
/// other Tango functions are called. To succeed, the calling application must
/// have camera permissions enabled. The initialization is invalidated on
/// calling TangoService_disconnect() or if the service is stopped or faults
/// while a client is running.  TangoService_initialize() uses two parameters,
/// the JNI context, env, and the native activity object, activity, to check the
/// version that the version of Tango Service installed on the device meets the
/// minimum number required by the client library. The parameters env and
/// activity may be retrieved in a native activity for example by:
/// @code
/// void android_main(struct android_app* state) {
///   JNIEnv* env;
///   JavaVM* m_vm = state->activity->vm;
///   (*m_vm)->AttachCurrentThread(m_vm, &env, NULL);
///   jobject activity = state->activity->clazz;
///   TangoService_initialize(env, activity);
/// }
/// @endcode
/// @param env A pointer to the JNI Context of the native activity.
/// @param activity The native activity object handle of the calling native
/// activity.
/// @return Returns TANGO_SUCCESS if a the Tango Service version is found to be
/// compatible with this client's library version and the service was
/// initialized successfully. Returns TANGO_INVALID if env and/or activity is
/// set to null and TANGO_ERROR if the version check fails, or if the service
/// connection could not be initialized.
TangoErrorType TangoService_initialize(JNIEnv* env, jobject activity);

/// TangoService_getConfig() creates a TangoConfig object with configuration
/// settings from the service.  This should be used to initialize a Config
/// object for setting the configuration that TangoService should be run in. The
/// Config handle is passed to TangoService_connect() which starts the service
/// running with the parameters set at that time in that TangoConfig handle.
/// This function can be used to find the current configuration of the service
/// (i.e. what would be run if no config is specified on
/// TangoService_connect()), or to create one of a few "template" TangoConfigs.
/// The returned TangoConfig can be further modified by TangoConfig_set function
/// calls.  The handle should be freed with TangoConfig_free().  The handle
/// is needed only at the time of TangoService_connect() where it is used to
/// configure the service, and can safely be freed after it has been used in
/// TangoService_connect().
/// @param config_type The requested configuration type.  For convenience a
/// enumerated set of TangoConfigType is specified as "templates".
/// See @link configparams Configuration Parameters @endlink
/// @return Returns a handle (TangoConfig*) for a newly allocated TangoConfig
/// object with settings as requested by config_type. Returns NULL if the
/// config_type is not valid, the config could not be allocated, the service
/// could not be initialized, or an internal failure occurred.
TangoConfig TangoService_getConfig(TangoConfigType config_type);

/// Connect sets the configuration of the Tango Service and starts it running.
/// The service will run with the current configuration, in either its default
/// configuration with the configuration specified in config.
/// TangoService_connect() starts the motion estimation and data (camera, depth,
/// callbacks etc.) will become available, and pose can be queried etc..   The
/// parameter config is used to set the configuration at this time, and can be
/// safely freed after.
/// @param context Optional. A user defined pointer that is returned in callback
/// functions onPoseAvailable, onXYZijAvailable and onTangoEvent. Can be safely
/// set to NULL if not used.
/// @param config The service will be started with the setting specified by this
/// TangoConfig handle.  If NULL is passed here, then the service will be
/// started in the default configuration.
/// @return Returns TANGO_SUCCESS on successfully starting the configuration.
/// Returns TANGO_ERROR on failure, or if a connection was not initialized with
/// TangoService_initialize(), or if the camera could not be opened, which could
/// be due to cameras being opened by other applications or a system error which
/// may require a reboot.  Returns TANGO_INVALID if an Area Description UUID
/// was specified but could not be found or accessed by the service.
TangoErrorType TangoService_connect(void* context, TangoConfig config);

/// Disconnect from the Tango Service. Callbacks will no longer be generated.
/// Applications should always call disconnect when the service is no longer
/// needed.  All previous configuration data is invalidated.
void TangoService_disconnect();

/// Sends a reset to the motion tracking system.  This will cause pose to
/// reinitialize and start from origin again.  This call can be done at any
/// time.  ADF state is not affected however the device will be starting from
/// origin until it localizes again.
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
/// @param frames The base/target pairs to listen to.
/// @param TangoService_onPoseAvailable function pointer to callback function.
TangoErrorType TangoService_connectOnPoseAvailable(
    uint32_t count, const TangoCoordinateFramePair* frames,
    void (*TangoService_onPoseAvailable)(void* context,
                                         const TangoPoseData* pose));

/// Get a pose at a given timestamp from the base to the target frame.
///
/// All poses returned are marked as VALID (in the status_code) even if they
/// were marked as INITIALIZING in the callback poses.
/// To determine when initialization is complete, register a callback using
/// TangoService_connectOnPoseAvailable() and wait until you receive valid data.
///
/// If no pose can be returned, the status_code of the returned pose will be
/// TANGO_INVALID.
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
/// pose status field should be checked for vailidity.  Returns TANGO_INVALID
/// if no connection was initialized with TangoService_initialize(), or if the
/// base and target frame are the same, or if the base or if the target frame
/// is not valid, or if timestamp is less than 0, or if the service has not yet
/// begun running (TangoService_connect() has not completed).
TangoErrorType TangoService_getPoseAtTime(double timestamp,
    TangoCoordinateFramePair frame, TangoPoseData* pose);

/**@} devsitenav Pose */

/// @defgroup depth Tango Service: Depth Interface
/// @brief Functions for getting depth information from the device.
/// @{

/// Attach an onXYZijAvailable callback.  The callback is called each time new
/// depth data is available, at an approximate nominal period given by the
/// double key depth_period_in_seconds, queryable by TangoConfig_getDouble().
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
/// TangoService_connectTextureId() or TangoService_connectOnFrameAvailable()
/// but not both.
/// @{

/// Connect a Texture ID to a camera. The camera is selected via TangoCameraId.
/// Currently only TANGO_CAMERA_COLOR and TANGO_CAMERA_FISHEYE are supported.
/// The texture must be the ID of a texture that has been allocated and
/// initialized by the calling application.
///
/// Note: The first scan-line of the color image is reserved for metadata
/// instead of image pixels.
/// @param id The ID of the camera to connect this texture to.  Only
/// TANGO_CAMERA_COLOR and TANGO_CAMERA_FISHEYE are supported.
/// @param context The context returned during the onFrameAvailable callback.
/// @param tex The texture ID of the texture to connect the camera to.  Must be
/// a valid texture in the applicaton.
/// @return Returns TANGO_INVALID if the camera ID is not valid.  Otherwise
/// returns TANGO_ERROR if an internal error occurred.
TangoErrorType TangoService_connectTextureId(TangoCameraId id, unsigned int tex,
                                             void* context,
                                             void (*callback)(void*,
                                                              TangoCameraId));

/// Update the texture that has been connected to camera referenced by
/// TangoCameraId. The texture is updated with the latest image from the
/// camera.
/// If timestamp is not NULL, it will be filled with the image timestamp.
/// @param id The ID of the camera to connect this texture to.  Only
/// TANGO_CAMERA_COLOR and TANGO_CAMERA_FISHEYE are supported.
/// @param timestamp Upon return, if not NULL upon calling, timestamp contains
/// the timestamp of the image that has been pushed to the connected texture.
/// @return Returns TANGO_INVALID if id is out of range, if
/// TangoService_initialize() must be called first, or if a texture ID was never
/// associated with the camera.  Otherwise   returns TANGO_SUCCESS.
TangoErrorType TangoService_updateTexture(TangoCameraId id, double* timestamp);

/// Connect a callback to a camera for access to the pixels. This is not
/// recommended for display but for applications requiring access to the
/// HAL_PIXEL_FORMAT_YV12 pixel data.  The camera is selected via
/// TangoCameraId.  Currently only TANGO_CAMERA_COLOR and TANGO_CAMERA_FISHEYE
/// are supported.  The onFrameAvailable callback will be called when a new frame
///  is available from the camera.
///
/// Note: The first scan-line of the color image is reserved for metadata
/// instead of image pixels.
/// @param id The ID of the camera to connect this texture to.  Only
/// TANGO_CAMERA_COLOR and TANGO_CAMERA_FISHEYE are supported.
/// @param context The context returned during the onFrameAvailable callback.
/// @param onFrameAvailable Function called when a new frame is available
/// from the camera.
TangoErrorType TangoService_connectOnFrameAvailable(
    TangoCameraId id, void* context,
    void (*onFrameAvailable)(void* context, TangoCameraId id,
                             const TangoImageBuffer* buffer));

/// Get the intrinsic calibration parameters for a given camera.  The intrinsics
/// are as specified by the TangoCameraIntrinsics struct.  Intrinsics are read
/// from the on-device intrinsics file (typically
/// /sdcard/config/calibration.xml, but to ensure compatibility applications
/// should only access these parameters via the API), or default internal model
/// parameters corresponding to the device are used if the calibration.xml is
/// not found.
/// @param camera_id The camera ID to retrieve the calibration intrinsics for.
/// @param intrinsics A TangoCameraIntrinsics struct that must be allocated
/// before calling, and is filled with calibration intrinsics for the camera
/// camera_id upon successful return.
/// @return Returns TANGO_SUCCESS on successfully retrieving calibration
/// intrinsics.  Returns TANGO_INVALID if the camera_id is out of range, or if
/// intrinsics argument was null, or if the service must be initialized with
/// TangoService_initialize().  Returns TANGO_ERROR if an internal error occurs
/// while getting intrinsics.
TangoErrorType TangoService_getCameraIntrinsics(TangoCameraId camera_id,
    TangoCameraIntrinsics* intrinsics);

/**@} devsitenav Camera */

/// @defgroup adf Tango Service: Area Description Interface
/// @brief Functions for handling Area Descriptions for localization.
/// Note that there is no direct function for loading an area description.
/// To load an Area Description, use the TangoConfig_setString() function to
/// set the configuration item config_load_area_description_UUID to the UUID
/// of the Area Description you want to load, and the Tango Service loads that
/// area when you call TangoService_connect().
///
/// See the following pages for more info:
/// - Doxygen: @link configparams Configuration Parameters @endlink
/// - Tutorial: <a href ="c-lifecycle">API Lifecycle</a>
/// - Tutorial: <a href ="c-area-learning">Area Learning</a>
///
/// To save area descriptions, your application must have
/// <a href
/// ="http://developer.android.com/reference/android/Manifest.permission.html#WRITE_EXTERNAL_STORAGE">WRITE_EXTERNAL_STORAGE</a>
/// permissions enabled in its Manifest.
/// @{

/// Saves the area description, returning the unique ID associated
/// with the saved map.
///
/// You can only save an area description after TangoService_connect()
/// has been called, but you can save at any time after that, and
/// save repeatedly if desired.
/// @param uuid Upon saving, the generated TangoUUID to refer to this map is
/// returned in uuid.
/// @return Returns TANGO_SUCCESS on success, and TANGO_ERROR if a failure
/// occurred when saving, or if the service needs to be initialized, or
/// TANGO_INVALID if uuid is NULL, or of incorrect length.
TangoErrorType TangoService_saveAreaDescription(TangoUUID* uuid);

/// Deletes an area description with the specified unique ID.  This
/// method can be called after TangoService_initialize(), but should
/// not be called to delete the ADF that is currently loaded.
/// @param uuid The area description to delete.
/// @return Returns TANGO_SUCCESS if area description file with
/// specified unique ID is found and can be removed.  Returns
/// TANGO_ERROR on failure to delete, or if the service needs to be initialized.
TangoErrorType TangoService_deleteAreaDescription(const TangoUUID uuid);

/// Gets the full list of unique area description IDs available on a
/// device as a comma-separated list of TangoUUIDs.  Memory should not
/// be deallocated outside the API.   Can be called any time after
/// calling TangoService_initialize().
/// @param uuid_list Upon successful return, uuid_list is set to a pointer to a
/// string (char array) that is a comma separated list of UUIDs.
/// @return Returns TANGO_SUCCESS on success, or TANGO_ERROR on failure to
/// retrieve the list, or if the service needs to be initialized, or
/// TANGO_INVALID if the uuid_list argument was NULL.
TangoErrorType TangoService_getAreaDescriptionUUIDList(char** uuid_list);

/// Gets the metadata handle associated with a single area description unique
/// ID.  Allocates memory which should be freed by calling
/// TangoAreaDescriptionMetadata_free().
/// @param uuid The TangoUUID for which to load the metadata.
/// @param metadata The metadata handle associated with the uuid.
/// @return Returns TANGO_SUCCESS on successful load of metadata, or TANGO_ERROR
/// if the service needs to be initialized or if the metadata could not be
/// loaded, or TANGO_INVALID if metadata was NULL.
TangoErrorType TangoService_getAreaDescriptionMetadata(
    const TangoUUID uuid, TangoAreaDescriptionMetadata* metadata);

/// Saves the metadata associated with a single area description unique ID.
/// @param uuid The TangoUUID associated with the metadata.
/// @param metadata The metadata to be saved.
/// @return Returns TANGO_SUCCESS on successful save, or TANGO_ERROR on failure,
/// or if the service needs to be initialized, or TANGO_INVALID if metadata
/// was NULL.
TangoErrorType TangoService_saveAreaDescriptionMetadata(
    const TangoUUID uuid, TangoAreaDescriptionMetadata metadata);

/// Frees the memory allocated by a call to
/// TangoService_getAreaDescriptionMetadata().
/// @param metadata The handle to the metadata to be deallocated.
/// @return Returns TANGO_SUCCESS if the metadata was deleted.
TangoErrorType TangoAreaDescriptionMetadata_free(
    TangoAreaDescriptionMetadata metadata);

/// Import an area description from the source file path to the default
/// area storage location and rename the area with its UUID.
/// @param src_file_path The source file path of the area to be imported.
/// @param uuid To store the UUID of the area.
/// @return Returns TANGO_SUCCESS on successful import, or TANGO_ERROR if the
/// file could not be imported, or TANGO_INVALID if uuid or src_file_path was
/// NULL
TangoErrorType TangoService_importAreaDescription(const char* src_file_path,
                                                  TangoUUID* uuid);

/// Export an area with the UUID from the default area storage location to
/// the destination file directory with the UUID as its name.
/// @param uuid the UUID of the area.
/// @param dst_file_dir The destination file directory.
/// @return Returns TANGO_SUCCESS if the file was exported, or TANGO_ERROR if
/// the export failed, or TANGO_INVALID if dst_file_dir was NULL.
TangoErrorType TangoService_exportAreaDescription(
    const TangoUUID uuid, const char* dst_file_dir);

/// Searches through the metadata list for a key that matches the parameter
/// 'key'.  If such a key is found, returns the value_size and value associated
/// with that key.  If the key has not been initialized in the map the
/// value_size will be 0 and the value will be NULL.
///
/// The supported keys are:
///
/// "id": The UUID of the ADF in a null-terminated char array. Setting this
/// value on the client side using TangoAreaDescriptionMetadata_set() will have
/// no effect on the data stored by the server.  Also, when you call
/// TangoService_saveAreaDescriptionMetadata() it ignores the value and uses
/// the supplied UUID.
///
/// "name": The name of the ADF in a null-terminated char array.
/// Default value for a new map is "unamed". The value can be set by calling
/// TangoAreaDescriptionMetadata_set().
///
/// "date_ms_since_epoch": The creation date of the ADF measured in milliseconds
/// since Unix epoch as a 64-Bit unsigned integer.
/// Setting this value on the client side using
/// TangoAreaDescriptionMetadata_set() will have no effect on the data stored
/// by the server. Also, when you call
/// TangoService_saveAreaDescriptionMetadata() it ignores the value.
///
/// "transformation": The transformation of the ADF's global coordinate.
/// The data consists of 7 double precision elements:
/// x, y, z :ECEF (earth centered earth fixed) Cartesian frame of reference at
/// the center of the earth which rotates with the earth).
/// qx, qy, qz, qw : Hamilton Quaternion.
/// The default corresponding values are:
/// (x, y, z, qx, qy, qz, qw) = (0, 0, 0, 0, 0, 0, 1).
///
/// @param metadata The metadata list to search through.
/// @param key The string key value of the parameter to set.
/// @param value_size The size in bytes of value, as allocated by the API.
/// @param value The value of the data with the corresponding key stored in the
/// metadata. The value will be returned as a binary data blob (The endianness
/// of the binary block is platform dependent).
/// The array memory should not be allocated by the caller, and will go out of
/// scope after a call to TangoAreaDescriptionMetadata_free.
/// The value will be NULL if the key does not exist or has not been set yet.
/// @return Returns TANGO_SUCCESS if the key is found.  If the key is valid but
/// does not have a valid value, size will be set to 0 and value will contain a
/// NULL.  Returns TANGO_INVALID if any of the arguments are NULL or the key is
/// not found.
TangoErrorType TangoAreaDescriptionMetadata_get(
    TangoAreaDescriptionMetadata metadata, const char* key, size_t* value_size,
    char** value);

/// Sets the value associated with an area description key to a new
/// value.
///
/// For a list of supported keys, see TangoAreaDescriptionMetadata_get().
///
/// @param metadata The metadata for which to set the key-value pair.
/// @param key The string key value of the parameter to set.
/// @param value_size The size in bytes of value, as allocated by the caller.
/// value will be written only up to this size in bytes.
/// @param value The value to which to set the key.
/// @return Returns TANGO_SUCCESS if the key is set, otherwise returns
/// TANGO_INVALID if the key is not found in the metadata or any of the
/// arguments is NULL.
TangoErrorType TangoAreaDescriptionMetadata_set(
    TangoAreaDescriptionMetadata metadata, const char* key, size_t value_size,
    const char* value);

/// Returns a comma separated list of all keys in the metadata.  Memory should
/// not be deallocated outside the API.
/// @param metadata The metadata from which to read the keys.
/// @param key_list Place to store the comma separated list
/// @return Returns TANGO_SUCCESS on success, TANGO_INVALID if the metadata was
/// not valid or key_list is NULL.
TangoErrorType TangoAreaDescriptionMetadata_listKeys(
    TangoAreaDescriptionMetadata metadata, char** key_list);

/** @} devsitenav Area Description */

/// @defgroup configparams Configuration Parameters Get and Set Functions
/// @brief Configuration Parameters Get and Set Functions
///
/// For an allocated TangoConfig handle, these functions get and set parameters
/// of that TangoConfig handle.  You can use the handle to query the current
/// state, or you can create a new handle and alter its contents to set the
/// service settings on connect().  For each type of configuration parameter
/// (bool, double, string, etc) you call the corresponding get/set function,
/// such as getBool for a boolean.
///
/// The supported configuration parameters that can be set are:
///
/// <table>
/// <tr><td class="indexkey">int32 config_color_iso</td><td
/// class="indexvalue">ISO value for the color camera.
/// For example, values like 100, 200, or 400.  Default is 100.</td></tr>
///
/// <tr><td class="indexkey">int32 config_color_exp</td><td class="indexvalue">Exposure value for the color camera, in
/// nanoseconds.  Default is 11100000 (11.1 ms).</td></tr>
///
/// <tr><td class="indexkey">boolean config_enable_auto_recovery</td><td class="indexvalue">
///         Automatically recovers when motion tracking becomes invalid, by
///         returning immediately to the initializing state in the pose
///         lifecycle. This will use the last valid pose as the starting pose
///         after recovery. If an area description is loaded, or if learning
///         mode is enabled, this will also automatically try to localize.
///         Defaults to true.</td></tr>
///
/// <tr><td class="indexkey">boolean config_enable_depth</td><td class="indexvalue">
///         Enables depth output if true.  Defaults to false.</td></tr>
///
/// <tr><td class="indexkey">boolean config_enable_low_latency_imu_integration</td><td class="indexvalue">
///         When the latest pose is requested or returned via callback, this
///         enables aggressive integration of the latest inertial measurements
///         to provide lower latency pose estimates, leaving the update
///         frequency unaffected.  The accuracy may be slightly lower because
///         the inertial data is not yet integrated with a new visual feature
///         update.</td></tr>
///
/// <tr><td class="indexkey">boolean config_enable_learning_mode</td><td class="indexvalue">
///         Enables learning mode if true.  Defaults to false.</td></tr>
///
/// <tr><td class="indexkey">boolean config_enable_motion_tracking</td><td class="indexvalue">
///         Enables motion tracking if true.  Defaults to true.</td></tr>
///
/// <tr><td class="indexkey">boolean config_enable_dataset_recording</td><td class="indexvalue">
///         Enables recording of a dataset to disk.</td></tr>
///
/// <tr><td class="indexkey">string config_load_area_description_UUID</td><td class="indexvalue">
///         Loads the given Area Description with given UUID and attempts to
///         localize against that Area Description.  Empty string will disable
///         localization.  Defaults to empty.</td></tr>
/// </table>
///
/// The supported configuration parameters that can be queried are:
///
/// <table>
/// <tr><td class = "indexkey">char* tango_service_library_version</td><td class ="indexvalue">
///         The version of the Tango Service Library that implements the Tango
///         functionality.  The version is returned as YYMMDD-{git hash}-{ARCHITECTURE}.</td></tr>
///
/// <tr><td class = "indexkey">double depth_period_in_seconds</td><td class ="indexvalue">
///         Nominal time between successive frames of depth data.  Use the depth data
///         structure fields to get more accurate depth frame times.</td></tr>
///
/// <tr><td class = "indexkey">int32 max_point_cloud_elements</td><td class ="indexvalue">
///         Maximum number of points returned in depth point clouds.  For a tablet device,
///         this is 60000.  Typically no more than to 15000 are returned.</td></tr>
/// </table>
///
/// @{

/// Set a boolean configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been created with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if config or key
/// is NULL, or key is not found or could not be set.
TangoErrorType TangoConfig_setBool(TangoConfig config, const char* key,
                                   bool value);

/// Set an int32 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been created with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if config or key
/// is NULL, or key is not found or could not be set.
TangoErrorType TangoConfig_setInt32(TangoConfig config, const char* key,
                                    int32_t value);

/// Set an int64 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been created with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if config or key
/// is NULL, or key is not found or could not be set.
TangoErrorType TangoConfig_setInt64(TangoConfig config, const char* key,
                                    int64_t value);

/// Set a double configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been created with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if config or key
/// is NULL, or key is not found or could not be set.
TangoErrorType TangoConfig_setDouble(TangoConfig config, const char* key,
                                     double value);

/// Set a character string configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been created with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if config or key
/// is NULL, or key is not found or could not be set.
TangoErrorType TangoConfig_setString(TangoConfig config, const char* key,
                                     const char* value);

/// Get a boolean configuration parameter.
/// @param config The configuration object to get the parameter from. config
/// must have been created with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if the any of the
/// arguments is NULL, or if the key could not be found.
TangoErrorType TangoConfig_getBool(TangoConfig config, const char* key,
                                   bool* value);

/// Get a uint32_t configuration parameter.
/// @param config The configuration object to get the parameter from. config
/// must have been created with TangoConfig.
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if the any of the
/// arguments is NULL, or if the key could not be found.
TangoErrorType TangoConfig_getInt32(TangoConfig config, const char* key,
                                    int32_t* value);

/// Get an uint64_t configuration parameter.
/// @param config The configuration object to get the parameter from. config
/// must have been created with TangoConfig.
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if the any of the
/// arguments is NULL, or if the key could not be found.
TangoErrorType TangoConfig_getInt64(TangoConfig config, const char* key,
                                    int64_t* value);

/// Get a double configuration parameter.
/// @param config The configuration object to get the parameter from. config
/// must have been created with TangoConfig,
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if the any of the
/// arguments is NULL, or if the key could not be found.
TangoErrorType TangoConfig_getDouble(TangoConfig config, const char* key,
                                     double* value);

/// Get a character string configuration parameter.
/// @param config The configuration object to get the parameter from. config
/// must have been created with TangoConfig.
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.  This array must be
/// allocated by the caller.
/// @param size The size in bytes of value, as allocated by the caller.  value
/// will be written only up to this size in bytes.
/// @return Returns TANGO_SUCCESS on success or TANGO_INVALID if the any of the
/// arguments is NULL, or if the key could not be found.
TangoErrorType TangoConfig_getString(TangoConfig config, const char* key,
                                     char* value, size_t size);
/**@} devsitenav Config Params */

#ifdef __cplusplus
}
#endif

#endif  // TANGO_CLIENT_API_H_
