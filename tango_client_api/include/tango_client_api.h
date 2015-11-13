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

/// @defgroup Enums Project Tango Enumerated Types
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
  TANGO_CONFIG_DEFAULT = 0, /**< Default, motion tracking only. */
  TANGO_CONFIG_CURRENT,     /**< Current */
  TANGO_CONFIG_MOTION_TRACKING,  /**< Motion tracking */
  TANGO_CONFIG_AREA_LEARNING, /**< Area learning */
  TANGO_CONFIG_RUNTIME,       /**< Runtime settable configuration */
  TANGO_MAX_CONFIG_TYPE       /**< Maximum number allowable.  */
} TangoConfigType;

/// @brief Tango coordinate frame enumerations.
typedef enum {
  /// Coordinate system for the entire Earth.
  /// See WGS84:
  /// <a href ="http://en.wikipedia.org/wiki/World_Geodetic_System">World
  /// Geodetic System</a>
  TANGO_COORDINATE_FRAME_GLOBAL_WGS84 = 0,
  /// Origin within a saved area description
  TANGO_COORDINATE_FRAME_AREA_DESCRIPTION,
  /// Origin when the device started tracking
  TANGO_COORDINATE_FRAME_START_OF_SERVICE,
  /// Immediately previous device pose
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
/// Success is denoted by <code>TANGO_SUCCESS = 0</code>.
typedef enum {
  /// The user has not given permissions to read and write datasets.
  TANGO_NO_DATASET_PERMISSION = -7,
  /// The user has not given permission to export or import ADF files.
  TANGO_NO_IMPORT_EXPORT_PERMISSION = -6,
  /// The user has not given permission to access the device's camera.
  TANGO_NO_CAMERA_PERMISSION = -5,
  /// The user has not given permission to save or change ADF files.
  TANGO_NO_ADF_PERMISSION = -4,
  /// The user has not given permission to use Motion Tracking functionality.
  /// Note: Motion Tracking permission is deprecated, apps are always permitted.
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
  TANGO_EVENT_AREA_LEARNING,    /**< Area Learning Event */
} TangoEventType;

/// Tango Camera Calibration types. See TangoCameraIntrinsics for a detailed
/// description.
typedef enum {
  TANGO_CALIBRATION_UNKNOWN,
  /// The FOV camera model described in
  /// <a href="http://scholar.google.com/scholar?cluster=13508836606423559694">
  /// Straight lines have to be straight</a>.
  TANGO_CALIBRATION_EQUIDISTANT,
  TANGO_CALIBRATION_POLYNOMIAL_2_PARAMETERS,
  /// Tsai's K1, K2, K3 model. See
  /// <a href ="http://scholar.google.com/scholar?cluster=3512800631607394002">A
  /// versatile camera calibration technique for high-accuracy 3D machine
  /// vision metrology using off-the-shelf TV cameras and lenses</a>.
  TANGO_CALIBRATION_POLYNOMIAL_3_PARAMETERS,
  TANGO_CALIBRATION_POLYNOMIAL_5_PARAMETERS,
} TangoCalibrationType;

/// Tango Image Formats
///
/// Equivalent to those found in Android core/system/include/system/graphics.h.
/// See TangoImageBuffer for a format description.
typedef enum {
  TANGO_HAL_PIXEL_FORMAT_RGBA_8888 = 1,      /**< RGBA 8888 */
  TANGO_HAL_PIXEL_FORMAT_YV12 = 0x32315659,  /**< YV12 */
  TANGO_HAL_PIXEL_FORMAT_YCrCb_420_SP = 0x11 /**< NV21 */
} TangoImageFormatType;

// Experimental API only, subject to change.
// Dataset recording mode.
typedef enum {
  /// Unknown dataset recording mode.
  TANGO_RECORDING_MODE_UNKNOWN = -1,
  /// Contains only data required for motion tracking.
  TANGO_RECORDING_MODE_MOTION_TRACKING = 0,
  /// Contains data required for motion tracking and scene reconstruction.
  TANGO_RECORDING_MODE_SCENE_RECONSTRUTCION = 1,
} TangoRecordingMode_Experimental;

/**@} */

/// @defgroup Types Project Tango Defined Types
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

/**@} */

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
///
/// For more information, including which coordinate frame pairs are valid,
/// see our page on
/// <a href ="/project-tango/overview/frames-of-reference">frames of
/// reference</a>.
typedef struct {
  /// Base frame of reference to compare against when requesting pose data.
  /// For example, if you have loaded an area and want to find out where the
  /// device is within it, you would use the
  /// <code>TANGO_COORDINATE_FRAME_AREA_DESCRIPTION</code> frame of reference
  /// as your base.
  TangoCoordinateFrameType base;

  /// Target frame of reference when requesting pose data, compared to the
  /// base. For example, if you want the device's pose data, use
  /// <code>TANGO_COORDINATE_FRAME_DEVICE</code>.
  TangoCoordinateFrameType target;
} TangoCoordinateFramePair;

/// The TangoPoseData struct contains pose information returned from motion
/// tracking.
/// The device pose is given using Android conventions.
/// See the Android
/// <a href ="http://developer.android.com/guide/topics/sensors/sensors_overview.html#sensors-coords">Sensor
/// Overview</a> page for more information.
typedef struct TangoPoseData {
  /// An integer denoting the version of the structure.
  uint32_t version;

  /// Timestamp of the time that this pose estimate corresponds to.
  double timestamp;

  /// Orientation, as a quaternion, of the pose of the target frame
  /// with reference to the base frame.
  /// Specified as (x,y,z,w) where RotationAngle is in radians:
  /// @code
  ///   x = RotationAxis.x * sin(RotationAngle / 2)
  ///   y = RotationAxis.y * sin(RotationAngle / 2)
  ///   z = RotationAxis.z * sin(RotationAngle / 2)
  ///   w = cos(RotationAngle / 2)
  /// @endcode
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
/// image data.  This data is populated by the service when it returns an image.
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
  /// Pixels in the format of this image buffer.
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
  /// For more information, see our
  /// <a href ="/project-tango/overview/depth-perception#xyzij">developer
  /// overview on depth perception</a>.
  uint32_t* ij;

  /// TangoImageBuffer is reserved for future use.
  TangoImageBuffer* color_image;
} TangoXYZij;

/// The TangoCameraIntrinsics struct contains intrinsic parameters for a camera.
///
/// Given a 3D point (X, Y, Z) in camera coordinates, the corresponding
/// pixel coordinates (x, y) are:
///
/// @code
/// x = X / Z * fx * rd / ru + cx
/// y = X / Z * fy * rd / ru + cy
/// @endcode
///
/// The normalized radial distance ru is given by:
///
/// @code
/// ru = sqrt((X^2 + Y^2) / (Z^2))
/// @endcode
///
/// The distorted radial distance rd depends on the distortion model used.
///
/// For <code>TANGO_CALIBRATION_POLYNOMIAL_3_PARAMETERS</code>, rd is a
/// polynomial that depends on the 3 distortion coefficients k1, k2 and k3:
///
/// @code
/// rd = ru + k1 * ru^3 + k2 * ru^5 + k3 * ru^7
/// @endcode
///
/// For <code>TANGO_CALIBRATION_EQUIDISTANT</code>, rd depends on the single
/// distortion coefficient w:
///
/// @code
/// rd = 1 / w * arctan(2 * ru * tan(w / 2))
/// @endcode
///
/// For more information, see our page on
/// <a href ="/project-tango/overview/intrinsics-extrinsics">Camera Intrinsics
/// and Extrinsics</a>.
typedef struct TangoCameraIntrinsics {
  /// ID of the camera which the intrinsics reference.
  TangoCameraId camera_id;
  // The type of distortion model used. This determines the meaning of the
  // distortion coefficients.
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

  /// Distortion coefficients. Number and meaning of these values depends on
  /// the distortion model specified by calibration_type.
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
/// - "FisheyeOverExposed:X" - the fisheye image is over exposed with average
/// pixel value X px.
/// - "FisheyeUnderExposed:X" - the fisheye image is under exposed with average
/// pixel value X px.
/// - "ColorOverExposed:X" - the color image is over exposed with average pixel
/// value X px.
/// - "ColorUnderExposed:X" - the color image is under exposed with average
/// pixel value X px.
/// - "TooFewFeaturesTracked:X" - too few features were tracked in the fisheye
/// image.  The number of features tracked is X.
/// - "AreaDescriptionSaveProgress:X" - ADF saving is X * 100 percent complete.
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

/// @defgroup ConfigTemplates Configuration Templates
/// @brief Functions for setting configurations for connecting to the device.
///
/// A configuration consists of settings that must be set before connecting to
/// the Tango service, and cannot be changed after connecting to the service.
/// @{

/// Free a TangoConfig object.
/// Frees the TangoConfig object for the handle specified by the config
/// variable.
void TangoConfig_free(TangoConfig config);

/// Allocates and returns a string of key-value pairs of all the configuration
/// values of TangoService. The string is separated into lines such that each
/// line is one key-value pair, with format "key=value\n".  Note that many of
/// these config values are read-only, unless otherwise documented.
char* TangoConfig_toString(TangoConfig config);

/**@} */

/// @defgroup Lifecycle Tango Service: Lifecycle Interface
/// @brief Functions for initializing, shutting down, and resetting
/// the Tango Service.
/// @{

/// @deprecated This function is implicitly called when needed.
/// Initialize the Tango Service. To succeed, the calling
/// application must have camera permissions enabled. The initialization is
/// invalidated if the service is stopped or faults while a client is running.
/// TangoService_initialize() uses two parameters, the JNI context, env, and the
/// native activity object, activity, to check the version that the version of
/// Tango Service installed on the device meets the minimum number required by
/// the client library. The parameters <code>env</code> and
/// <code>activity</code> may be retrieved in a native activity for example by:
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
/// @return Returns <code>TANGO_SUCCESS</code> if a the Tango Service version
/// is found to be compatible with this client's library version and the
/// service was initialized successfully. Returns <code>TANGO_INVALID</code>
/// if either the <i>env</i> and/or <i>activity</i> parameter is set to null.
/// Returns <code>TANGO_ERROR</code> if the version check fails, or if the
/// service connection could not be initialized.
TangoErrorType TangoService_initialize(JNIEnv* env, jobject activity);

/// Creates a TangoConfig object with configuration settings from the service.
/// This should be used to initialize a Config
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
/// @param config_type The requested configuration type.
/// @return Returns a handle (TangoConfig*) for a newly allocated TangoConfig
/// object with settings as requested by config_type. Returns NULL if the
/// config_type is not valid, the config could not be allocated, the service
/// could not be initialized, or an internal failure occurred.
TangoConfig TangoService_getConfig(TangoConfigType config_type);

/// Sets the configuration of the Tango Service and starts it running.
/// The service will connect with the specified configuration, or the default
/// configuration if none is provided.
/// TangoService_connect() starts the motion estimation and at that point you
/// can query the Tango Service for data (such as camera, depth, or pose), via
/// direct calls or callbacks.  When the service starts, it uses the TangoConfig
/// specified by config; you can free it after the call completes.
/// @param context Optional. A user defined pointer that is returned in callback
/// functions onPoseAvailable, onXYZijAvailable and onTangoEvent. If set to NULL
/// then an additional argument can be passed to onPoseAvailable,
/// onXYZijAvailable and onTangoEvent to optionally set individual callback
/// contexts.
/// @param config The service will be started with the setting specified by this
/// TangoConfig handle.  If NULL is passed here, then the service will be
/// started in the default configuration.
/// @return Returns <code>TANGO_SUCCESS</code> on successfully starting the
/// configuration. Returns <code>TANGO_ERROR</code> on failure, or if the
/// camera could not be opened which could be due to cameras being opened by
/// other applications or could be caused by a system error which may require
/// a reboot.  Returns <code>TANGO_INVALID</code> if an Area Description UUID
/// was specified but could not be found or accessed by the service, or if the
/// provided combination of config flags is not valid.  Returns <code>
/// TANGO_NO_DATASET_PERMISSION</code> if the config_enable_dataset_recording
/// flag was enabled, but the user has not given permissions to read and write
/// datasets.
TangoErrorType TangoService_connect(void* context, TangoConfig config);

/// Sets configuration parameters at runtime.  Only configuration parameters
/// prefixed config_runtime_ are settable with this and others are ignored.
/// Must be called after TangoService_connect(), on a running system.
/// @param tconfig The service will be configured while running to the newly
/// specified setting.
/// @return Returns <code>TANGO_SUCCESS</code> on successfully reconfiguring the
/// service. Returns <code>TANGO_INVALID</code> if the app has not connected
/// to the service, in which case no changes are made, or if tconfig is NULL.
/// Returns <code>TANGO_ERROR</code> on failure or if the service was not found
/// on the device.
TangoErrorType TangoService_setRuntimeConfig(TangoConfig tconfig);

/// Disconnects from the Tango Service. Callbacks will no longer be generated
/// or provide sensor data. Applications should always call
/// TangoService_disconnect() when the service is no longer needed. When you
/// call this function, all previous configuration data is invalidated.
void TangoService_disconnect();

/// Resets the motion tracking system.  This reinitializes the
/// <code>TANGO_COORDINATE_FRAME_START_OF_SERVICE</code> coordinate frame to
/// where the device is when you call this function; afterwards, if you ask for
/// the pose with relation to start of service, it uses this as the new origin.
/// You can call this function at any time.
///
/// If you are using Area Learning, the
/// <code>TANGO_COORDINATE_FRAME_AREA_DESCRIPTION</code> coordinate frame is
/// not affected by calling this function; however, the device needs to localize
/// again before you can use the area description.
void TangoService_resetMotionTracking();

/**@} */

/// @defgroup Pose Tango Service: Pose Interface
/// @brief Functions for getting the pose of the device.
/// @{

/// Attaches an onPoseAvailable callback.   The callback is called as new pose
/// updates become available for the registered coordinate frame pair.  When
/// registering the callback, specify the the target and base frame of
/// interest, and the callback will be called on each change of the pose of
/// that target with reference to that base frame.  Only some base/target pairs
/// are currently supported; for a list, see our page on
/// <a href ="/project-tango/overview/frames-of-reference">frames of
/// reference</a>.
///
/// For example, <code>TANGO_COORDINATE_FRAME_DEVICE</code> to base
/// <code>TANGO_COORDINATE_FRAME_START_OF_SERVICE</code> is a typical motion
/// tracking pair to track the motion of the device with reference to its
/// starting position in the base frame of reference.
///
/// An optional argument following the callback pointer can be supplied and will
/// be returned in the callback context parameter if TangoService_connect() was
/// called with a null context.
///
/// @param count The number of base/target pairs to listen to.
/// @param frames The base/target pairs to listen to.
/// @param TangoService_onPoseAvailable Function pointer to callback function.
TangoErrorType TangoService_connectOnPoseAvailable(
    uint32_t count, const TangoCoordinateFramePair* frames,
    void (*TangoService_onPoseAvailable)(void* context,
                                         const TangoPoseData* pose),
    ...);

/// Get a pose at a given timestamp from the base to the target frame.
///
/// All poses returned are marked as <code>TANGO_POSE_VALID</code> (in the
/// status_code field on TangoPoseData) even if they were marked as
/// <code>TANGO_POSE_INITIALIZING</code> in the callback poses.
/// To determine when initialization is complete, register a callback using
/// TangoService_connectOnPoseAvailable() and wait until you receive valid data.
///
/// If no pose can be returned, the status_code of the returned pose will be
/// <code>TANGO_POSE_INVALID</code>.
///
/// @param timestamp Time specified in seconds. If not set to 0.0, getPoseAtTime
/// retrieves the interpolated pose closest to this timestamp.  If set to 0.0,
/// the most
/// recent pose estimate for the target-base pair is returned.  The time
/// of the returned pose is contained in the pose output structure and may
/// differ from the queried timestamp.
/// @param frame A pair of coordinate frames specifying the transformation to be
/// queried for. For example, typical device motion is given by a target frame
/// of <code>TANGO_COORDINATE_FRAME_DEVICE</code> and a base frame of
/// <code>TANGO_COORDINATE_FRAME_START_OF_SERVICE</code>.
/// @param pose The pose of target with respect to base frame of reference.
/// Must be allocated by the caller, and is overwritten upon return.
/// @return Returns <code>TANGO_SUCCESS</code> if a pose was returned
/// successfully.  Check the <i>status_code</i> attribute on the returned
/// <i>pose</i> to see if it is valid.  Returns <code>TANGO_INVALID</code>
/// if the base and target frame are the same, or if the base or if the target
/// frame is not valid, or if timestamp is less than 0, or if the service has
/// not yet begun running (TangoService_connect() has not completed).
TangoErrorType TangoService_getPoseAtTime(double timestamp,
    TangoCoordinateFramePair frame, TangoPoseData* pose);

/**@} */

/// @defgroup Depth Tango Service: Depth Interface
/// @brief Functions for getting depth information from the device.
/// @{

/// Attach an onXYZijAvailable callback.  The callback is called each time new
/// depth data is available.  On the Tango tablet, the depth callback occurs at
/// 5 Hz.
///
/// An optional argument following the callback pointer can be supplied and will
/// be returned in the callback context parameter if TangoService_connect() was
/// called with a null context.
/// @param TangoService_onXYZijAvailable Function pointer for the callback
/// function.
/// @return Returns <code>TANGO_ERROR</code> if the callback function pointer
/// is null. Returns <code>TANGO_SUCCESS</code> otherwise.
TangoErrorType TangoService_connectOnXYZijAvailable(
    void (*TangoService_onXYZijAvailable)(void* context,
                                          const TangoXYZij* xyz_ij),
    ...);

/**@} */

/// @defgroup Event Tango Service: Event Notification Interface
/// @brief Functions for getting event notifications from the device.
/// @{

/// Attach an onTangoEvent callback.  The callback is called each time a
/// Tango event happens.
/// An optional argument following the callback pointer can be supplied and will
/// be returned in the callback context parameter if TangoService_connect() was
/// called with a null context.
/// @param TangoService_onTangoEvent Function pointer for the callback function.
TangoErrorType TangoService_connectOnTangoEvent(
    void (*TangoService_onTangoEvent)(void* context, const TangoEvent* event),
    ...);

/**@} Event Notification */

/// @defgroup Camera Tango Service: Camera Interface
/// @brief Functions for getting input from the device's cameras.  Use either
/// TangoService_connectTextureId() or TangoService_connectOnFrameAvailable()
/// but not both.
/// @{

/// Connect a Texture ID to a camera; the camera is selected by specifying a
/// TangoCameraId.  Currently only <code>TANGO_CAMERA_COLOR</code> and
/// <code>TANGO_CAMERA_FISHEYE</code> are supported.
/// The texture must be the ID of a texture that has been allocated and
/// initialized by the calling application.  The TangoConfig flag
/// config_enable_color_camera must be set to true for connectTextureId
/// to succeed after TangoService_connect() is called.
///
/// Note: The first scan-line of the color image is reserved for metadata
/// instead of image pixels.
/// @param id The ID of the camera to connect this texture to.  Only
/// <code>TANGO_CAMERA_COLOR</code> and <code>TANGO_CAMERA_FISHEYE</code>
/// are supported.
/// @param context The context returned during the onFrameAvailable callback.
/// @param tex The texture ID of the texture to connect the camera to.  Must be
/// a valid texture in the applicaton.
/// @return Returns <code>TANGO_INVALID</code> if the camera ID is not valid.
/// Otherwise returns <code>TANGO_ERROR</code> if an internal error occurred.
TangoErrorType TangoService_connectTextureId(TangoCameraId id, unsigned int tex,
                                             void* context,
                                             void (*callback)(void*,
                                                              TangoCameraId));

/// Update the texture that has been connected to camera referenced by
/// TangoCameraId. The texture is updated with the latest image from the
/// camera.
/// If timestamp is not NULL, it will be filled with the image timestamp.
/// @param id The ID of the camera to connect this texture to.  Only
/// <code>TANGO_CAMERA_COLOR</code> and <code>TANGO_CAMERA_FISHEYE</code>
/// are supported.
/// @param timestamp Upon return, if not NULL upon calling, timestamp contains
/// the timestamp of the image that has been pushed to the connected texture.
/// @return Returns <code>TANGO_INVALID</code> if <i>id</i> is out of range or
/// if a texture ID was never associated with the camera.  Otherwise returns
/// <code>TANGO_SUCCESS</code>.
TangoErrorType TangoService_updateTexture(TangoCameraId id, double* timestamp);

/// Connect a callback to a camera for access to the pixels. This is not
/// recommended for display but for applications requiring access to the
/// <code>HAL_PIXEL_FORMAT_YV12</code> pixel data.  The camera is selected via
/// TangoCameraId.  Currently only <code>TANGO_CAMERA_COLOR</code> and
/// <code>TANGO_CAMERA_FISHEYE</code> are supported. The <i>onFrameAvailable</i>
/// callback will be called when a new frame is available from the camera.
/// The TangoConfig flag <code>config_enable_color_camera</code> (see @link
/// ConfigParams Configuration Parameters @endlink) must be set to true for
/// TangoService_connectOnFrameAvailable() to succeed after
/// TangoService_connect() is called.
///
/// Note: The first scan-line of the color image is reserved for metadata
/// instead of image pixels.
/// @param id The ID of the camera to connect this texture to.  Only
/// <code>TANGO_CAMERA_COLOR</code> and <code>TANGO_CAMERA_FISHEYE</code>
/// are supported.
/// @param context The context returned during the onFrameAvailable callback.
/// @param onFrameAvailable Function called when a new frame is available
/// from the camera.
TangoErrorType TangoService_connectOnFrameAvailable(
    TangoCameraId id, void* context,
    void (*onFrameAvailable)(void* context, TangoCameraId id,
                             const TangoImageBuffer* buffer));

/// Disconnect a camera. The camera is selected via TangoCameraId.
/// Currently only <code>TANGO_CAMERA_COLOR</code> and
/// <code>TANGO_CAMERA_FISHEYE</code> are supported.
/// This call will disconnect callbacks that have been registered with
/// TangoService_connectTextureId() or TangoService_connectOnFrameAvailable().
TangoErrorType TangoService_disconnectCamera(TangoCameraId id);

/// Get the intrinsic calibration parameters for a given camera.  The intrinsics
/// are as specified by the TangoCameraIntrinsics struct.  Intrinsics are read
/// from the on-device intrinsics file (typically
/// <code>/sdcard/config/calibration.xml</code>, but to ensure compatibility
/// applications should only access these parameters via the API), or default
/// internal model parameters corresponding to the device are used if the
/// calibration.xml file is not found.
/// @param camera_id The camera ID to retrieve the calibration intrinsics for.
/// @param intrinsics A TangoCameraIntrinsics struct that must be allocated
/// before calling, and is filled with calibration intrinsics for the camera
/// <i>camera_id</i> upon successful return.
/// @return Returns <code>TANGO_SUCCESS</code> on successfully retrieving
/// calibration intrinsics.  Returns <code>TANGO_INVALID</code> if the camera_id
/// is out of range or if intrinsics argument was null.  Returns
/// <code>TANGO_ERROR</code> if an internal error occurs while getting
/// intrinsics.
TangoErrorType TangoService_getCameraIntrinsics(TangoCameraId camera_id,
    TangoCameraIntrinsics* intrinsics);

/**@} */

/// @defgroup AreaDescription Tango Service: Area Description Interface
/// @brief Functions for handling Area Descriptions for localization.
/// Note that there is no direct function for loading an area description.  To
/// load an Area Description, use the TangoConfig_setString() function to set
/// the configuration item <code>config_load_area_description_UUID</code> to the
/// UUID of the Area Description you want to load, and the Tango Service loads
/// that area when you call TangoService_connect().
///
/// See the following pages for more info:
/// - Doxygen: @link ConfigParams Configuration Parameters @endlink
/// - Tutorial: <a href ="/project-tango/apis/c/c-lifecycle">API Lifecycle</a>
/// - Tutorial: <a href ="/project-tango/apis/c/c-area-learning">Area
/// Learning</a>
///
/// To save area descriptions, your application must have
/// <code><a href="http://developer.android.com/reference/android/Manifest.permission.html#WRITE_EXTERNAL_STORAGE">WRITE_EXTERNAL_STORAGE</a></code>
/// permissions enabled in its Manifest.
/// @{

/// Optimizes and saves the area description, returning the unique ID associated
/// with the saved ADF.
///
/// You can only save an area description while connected to the Tango Service
/// (after calling TangoService_connect() but before calling
/// TangoService_disconnect()), and if you have enabled Area Learning mode by
/// setting config_enable_learning_mode to true in the TangoConfig when
/// connecting.
///
/// If you enabled Area Learning mode and you also loaded an ADF when connecting
/// (specified using <code>config_load_area_description_UUID</code>) then
/// calling this method appends any new learned areas to the loaded areas and
/// returns a new UUID for the new ADF (the original ADF is not modified).
///
/// This method may be long-running (do not call it on the UI thread). Since
/// the Tango Service locks internally, other API calls (such as
/// TangoService_getPoseAtTime()) will block while this method is running.
/// Feedback about the progress of this operation is provided via a TangoEvent
/// of type TANGO_EVENT_AREA_LEARNING with key "AreaDescriptionSaveProgress".
///
/// Calling this method will permanently stop motion tracking and area learning.
/// After this method completes you may use TangoService_getPoseAtTime()
/// repeatedly to query for the optimized pose estimate for any pose from the
/// past using its timestamp. To re-start motion tracking or localize on the
/// learned area description you must call TangoService_disconnect() and then
/// connect again as normal.
///
/// @param uuid Upon saving, the TangoUUID to refer to this ADF is
/// returned in <i>uuid</i>.
/// @return Returns <code>TANGO_SUCCESS</code> on success, and
/// <code>TANGO_ERROR</code> if a failure occurred when saving, or if the
/// service needs to be initialized, or <code>TANGO_INVALID</code> if uuid is
/// NULL, or of incorrect length, or if Area Learning Mode was not set (see
/// logcat for details).
TangoErrorType TangoService_saveAreaDescription(TangoUUID* uuid);

/// Deletes an area description with the specified unique ID.  This
/// method should not be called to delete the ADF that is currently loaded.
/// @param uuid The area description to delete.
/// @return Returns <code>TANGO_SUCCESS</code> if area description file with
/// specified unique ID is found and can be removed.  Returns
/// <code>TANGO_ERROR</code> on failure to delete, or if the service needs to
/// be initialized.
TangoErrorType TangoService_deleteAreaDescription(const TangoUUID uuid);

/// Gets the full list of unique area description IDs available on a
/// device as a comma-separated list of TangoUUIDs.  Memory should not
/// be deallocated outside the API.
/// @param uuid_list Upon successful return, <i>uuid_list</i> will contain a
/// comma separated list of available UUIDs.
/// @return Returns <code>TANGO_SUCCESS</code> on success, or
/// <code>TANGO_ERROR</code> on failure to retrieve the list, or if the service
/// needs to be initialized, or <code>TANGO_INVALID</code> if the
/// <i>uuid_list</i> argument was NULL.
TangoErrorType TangoService_getAreaDescriptionUUIDList(char** uuid_list);

/// Gets the metadata handle associated with a single area description unique
/// ID.  Allocates memory which should be freed by calling
/// TangoAreaDescriptionMetadata_free().
/// @param uuid The TangoUUID for which to load the metadata.
/// @param metadata The metadata handle associated with the uuid.
/// @return Returns <code>TANGO_SUCCESS</code> on successful load of metadata,
/// or <code>TANGO_ERROR</code> if the service needs to be initialized or if
/// the metadata could not be loaded, or <code>TANGO_INVALID</code> if metadata
/// was NULL.
TangoErrorType TangoService_getAreaDescriptionMetadata(
    const TangoUUID uuid, TangoAreaDescriptionMetadata* metadata);

/// Saves the metadata associated with a single area description unique ID.
/// @param uuid The TangoUUID associated with the metadata.
/// @param metadata The metadata to be saved.
/// @return Returns <code>TANGO_SUCCESS</code> on successful save, or
/// <code>TANGO_ERROR</code> on failure, or if the service needs to be
/// initialized, or <code>TANGO_INVALID</code> if metadata was NULL.
TangoErrorType TangoService_saveAreaDescriptionMetadata(
    const TangoUUID uuid, TangoAreaDescriptionMetadata metadata);

/// Frees the memory allocated by a call to
/// TangoService_getAreaDescriptionMetadata().
/// @param metadata The handle to the metadata to be deallocated.
/// @return Returns <code>TANGO_SUCCESS</code> if the metadata was deleted.
TangoErrorType TangoAreaDescriptionMetadata_free(
    TangoAreaDescriptionMetadata metadata);

/// Import an area description from the source file path to the default
/// area storage location.
/// @param src_file_path The source file path of the area to be imported.
/// @param uuid Populated with the UUID of the new file. This will be the same
/// as the source file's UUID.
/// @return Returns <code>TANGO_SUCCESS</code> on successful import, or
/// <code>TANGO_ERROR</code> if the file could not be imported, or
/// <code>TANGO_INVALID</code> if <i>uuid</i> or <i>src_file_path</i> was NULL.
TangoErrorType TangoService_importAreaDescription(const char* src_file_path,
                                                  TangoUUID* uuid);

/// Export an area with the UUID from the default area storage location to
/// the destination file directory with the UUID as its name.
/// @param uuid the UUID of the area.
/// @param dst_file_dir The destination file directory.
/// @return Returns <code>TANGO_SUCCESS</code> if the file was exported, or
/// <code>TANGO_ERROR</code> if the export failed, or
/// <code>TANGO_INVALID</code> if <i>dst_file_dir</i> was NULL.
TangoErrorType TangoService_exportAreaDescription(
    const TangoUUID uuid, const char* dst_file_dir);

/// Searches through the metadata list for a key that matches the parameter
/// <i>key</i>.  If such a key is found, returns the <i>value_size</i> and
/// <i>value</i> associated with that key.  If the key has not been initialized
/// in the map the <i>value_size</i> will be 0 and the <i>value</i> will be
/// NULL.
///
/// The supported keys are:
///
/// - <code>"id"</code>: The UUID of the ADF in a null-terminated char
/// array. Setting this value on the client side using
/// TangoAreaDescriptionMetadata_set() will have no effect on the data stored by
/// the server.  Also, when you call TangoService_saveAreaDescriptionMetadata()
/// it ignores the value and uses the supplied UUID.
///
/// - <code>"name"</code>: The name of the ADF in a null-terminated char array.
/// Default value for a new map is <code>"unnamed"</code>. The value can be set
/// by calling TangoAreaDescriptionMetadata_set().
///
/// - <code>"date_ms_since_epoch"</code>: The creation date of the ADF measured
/// in milliseconds since Unix epoch as a 64-Bit unsigned integer.  Setting this
/// value on the client side using TangoAreaDescriptionMetadata_set() will have
/// no effect on the data stored by the server. Also, when you call
/// TangoService_saveAreaDescriptionMetadata() it ignores the value.
///
/// - <code>"transformation"</code>: The transformation of the ADF's global
/// coordinate system.
///
/// For the <code>transformation</code> key, the transformation data consists
/// of 7 double precision elements:
///
/// - <code>x, y, z</code> :ECEF (earth centered earth fixed) Cartesian frame of
/// reference at the center of the earth which rotates with the earth).
/// - <code>qx, qy, qz, qw</code> : Hamilton Quaternion.
///
/// The default corresponding values are:
/// <code>(x, y, z, qx, qy, qz, qw) = (0, 0, 0, 0, 0, 0, 1)</code>.
///
/// @param metadata The metadata list to search through.
/// @param key The string key value of the parameter to set.
/// @param value_size The size in bytes of value, as allocated by the
/// caller.  <i>value</i> will be written only up to this size in bytes.
/// @param value The value of the data with the corresponding key stored in the
/// metadata. The value will be returned as a binary data blob (The endianness
/// of the binary block is platform dependent).
/// The array memory should not be allocated by the caller, and will go out of
/// scope after a call to TangoAreaDescriptionMetadata_free().
/// The value will be NULL if the key does not exist or has not been set yet.
/// @return Returns <code>TANGO_SUCCESS</code> if the key is found.  If the key
/// is valid but does not have a valid value, size will be set to 0 and value
/// will contain a NULL.  Returns <code>TANGO_INVALID</code> if any of the
/// arguments are NULL or the key is not found.
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
/// @param value_size The size in bytes of <i>value</i>, as allocated by the
/// caller. <i>value</i> will be written only up to this size in bytes.
/// @param value The value to which to set the key.
/// @return Returns <code>TANGO_SUCCESS</code> if the key is set, otherwise
/// returns <code>TANGO_INVALID</code> if the key is not found in the metadata
/// or any of the arguments is NULL.
TangoErrorType TangoAreaDescriptionMetadata_set(
    TangoAreaDescriptionMetadata metadata, const char* key, size_t value_size,
    const char* value);

/// Returns a comma separated list of all keys in the metadata.  Memory should
/// not be deallocated outside the API.
/// @param metadata The metadata from which to read the keys.
/// @param key_list Place to store the comma separated list
/// @return Returns <code>TANGO_SUCCESS</code> on success,
/// <code>TANGO_INVALID</code> if the metadata was not valid or key_list is
/// NULL.
TangoErrorType TangoAreaDescriptionMetadata_listKeys(
    TangoAreaDescriptionMetadata metadata, char** key_list);

/** @} */

/// @defgroup ConfigParams Configuration Parameters Get and Set Functions
/// @brief Configuration Parameters Get and Set Functions
///
/// For an allocated TangoConfig handle, these functions get and set parameters
/// of that TangoConfig handle.  You can use the handle to query the current
/// state, or you can create a new handle and alter its contents to set the
/// service settings on TangoService_connect().  For each type of configuration
/// parameter (bool, double, string, etc) you call the corresponding get/set
/// function, such as TangoConfig_getBool() for a boolean.
///
/// The supported configuration parameters that can be set are:
///
/// <table>
/// <tr><td>boolean config_color_mode_auto</td><td>
/// Use auto-exposure/auto-whitebalance with the color camera.  Defaults to
/// true, and
/// if true, the values for config_color_iso and config_color_exp are ignored.
/// </td></tr>
///
/// <tr><td>int32 config_color_iso</td><td>
///         ISO value for the color camera.
///         One of 100, 200, 400 or 800.  Default is 100.  Only applied if
///         config_color_mode_auto is set to false.</td></tr>
///
/// <tr><td>int32 config_color_exp</td><td>
///         Exposure value for the color camera, in nanoseconds.  Default is
///         11100000 (11.1 ms).  Valid from 0 to 30000000.  Only applied if
///         config_color_mode_auto is set to false.</td></tr>
///
/// <tr><td>boolean config_enable_auto_recovery</td><td>
///         Automatically recovers when motion tracking becomes invalid, by
///         returning immediately to the initializing state in the pose
///         lifecycle. This will use the last valid pose as the starting pose
///         after recovery. If an area description is loaded, or if learning
///         mode is enabled, this will also automatically try to localize.
///         Defaults to true.</td></tr>
///
/// <tr><td>boolean config_enable_color_camera</td><td>
///         Enables the color camera if true.  Defaults to false.  Must be
///         set to true if connecting color camera or texture callbacks after
///         TangoService_connect() is called.  When connecting a callback before
///         calling TangoService_connect() is called, config_enable_color_camera
///         is set to true internally.  Due to potential lifecycle race
///         conditions it is recommended to always set to true if the color
///         camera is to be used.</td></tr>
///
/// <tr><td>boolean config_enable_depth</td><td>
///         Enables depth output if true.  Defaults to false.</td></tr>
///
/// <tr><td>boolean config_enable_low_latency_imu_integration</td><td>
///         When the latest pose is requested or returned via callback, this
///         enables aggressive integration of the latest inertial measurements
///         to provide lower latency pose estimates, leaving the update
///         frequency unaffected.  The accuracy may be slightly lower because
///         the inertial data is not yet integrated with a new visual feature
///         update.</td></tr>
///
/// <tr><td>boolean config_enable_learning_mode</td><td>
///         Enables learning mode if true.  Defaults to false.</td></tr>
///
/// <tr><td>boolean config_enable_motion_tracking</td><td>
///         Enables motion tracking if true.  Defaults to true.</td></tr>
///
/// <tr><td>boolean config_experimental_high_accuracy_small_scale_adf</td><td>
///         EXPERIMENTAL Toggles between high-accuracy-small-scale and
///         normal-accuracy-large-scale ADFs when learning mode is enabled.
///         The higher accuracy is only supported in environments the size of a
///         small home. A Tango device can contain a mixture of ADFs with normal
///         and with high accuracy. When relocalizing against an ADF, the API
///         automatically selects the correct algorithms based on the type of
///         ADF. Note that it is not possible yet to learn additional parts of
///         an environment into the same high accuracy ADF; all learning has to
///         happen in one single session.</td></tr>
///
/// <tr><td>boolean config_high_rate_pose</td><td>
///         This flag enables 100Hz pose updates in callback mode. If disabled,
///         pose updates are provided at 33Hz in callback mode. Default value is
///         true.</td></tr>
///
/// <tr><td>boolean config_smooth_pose</td><td>
///         This flag enables pose smoothing using a spatial smoothing method.
///         Small pose corrections are hidden while moving, large pose
///         corrections are applied instantly. Default value is true.</td></tr>
///
/// <tr><td>string config_load_area_description_UUID</td><td>
///         Loads the given Area Description with given UUID and attempts to
///         localize against that Area Description.  Empty string will disable
///         localization.  Defaults to empty.</td></tr>
///
/// <tr><td>boolean config_experimental_enable_dataset_recording</td><td>
///         EXPERIMENTAL Enables recording of a dataset to disk. Note that this
///         API is subject to change.</td></tr>
///
/// <tr><td>boolean config_experimental_enable_scene_reconstruction</td><td>
///         EXPERIMENTAL This flag enables the experimental scene reconstruction
///         APIs which can be used to construct a mesh of an environment. Note
///         that this API is subject to change.</td></tr>
/// </table>
///
/// The supported configuration parameters that can be queried are:
///
/// <table>
/// <tr><td>char* tango_service_library_version</td><td>
///         The version of the Tango Service Library that implements the Tango
///         functionality.  The version is returned as
///         YYMMDD-{git hash}-{ARCHITECTURE}.</td></tr>
///
/// <tr><td>double depth_period_in_seconds</td><td>
///         Nominal time between successive frames of depth data.  Use the depth
///         data structure fields to get more accurate depth frame times.
/// </td></tr>
///
/// <tr><td>int32 max_point_cloud_elements</td><td>
///         Maximum number of points returned in depth point clouds.  For a
///         tablet device, this is 60000.  Typically no more than to 15000
///         are returned.</td></tr>
/// </table>
///
/// The supported configuration parameters that can be queried and set at
/// runtime are:
///
/// <table>
/// <tr><td>int32 config_runtime_depth_framerate</td><td>
///         Sets the framerate, in frames per second, at which depth is acquired
///         and returned via the depth callback.  Setting this parameter to 0
///         disables acquisition of depth data and the callback will not be
///         called and will resume if set to greater than 0.
///         config_enable_depth must have been set to true, or the depth
///         callback must be connected before TangoService_connect() in order
///         for depth to be active so that this flag has effect. The Project
///         Tango Tablet Development Kit supports depth framerates of 0, 1, 2, 3
///         and 5.</td></tr>
/// </table>
///
/// @{

/// Set a boolean configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been created with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns <code>TANGO_SUCCESS</code> on success or
/// <code>TANGO_INVALID</code> if <i>config</i> or key is NULL, or key is not
/// found or could not be set.
TangoErrorType TangoConfig_setBool(TangoConfig config, const char* key,
                                   bool value);

/// Set an int32 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been created with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns <code>TANGO_SUCCESS</code> on success or
/// <code>TANGO_INVALID</code> if <i>config</i> or <i>key</i> is NULL, or
/// <i>key</i> is not found or could not be set.
TangoErrorType TangoConfig_setInt32(TangoConfig config, const char* key,
                                    int32_t value);

/// Set an int64 configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been created with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns <code>TANGO_SUCCESS</code> on success or
/// <code>TANGO_INVALID</code> if <i>config</i> or <i>key</i> is NULL, or
/// <i>key</i> is not found or could not be set.
TangoErrorType TangoConfig_setInt64(TangoConfig config, const char* key,
                                    int64_t value);

/// Set a double configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been created with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns <code>TANGO_SUCCESS</code> on success or
/// <code>TANGO_INVALID</code> if <i>config</i> or <i>key</i> is NULL, or
/// <i>key</i> is not found or could not be set.
TangoErrorType TangoConfig_setDouble(TangoConfig config, const char* key,
                                     double value);

/// Set a character string configuration parameter.
/// @param config The configuration object to set the parameter on.  config must
/// have been created with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns <code>TANGO_SUCCESS</code> on success or
/// <code>TANGO_INVALID</code> if <i>config</i> or <i>key</i> is NULL, or
/// <i>key</i> is not found or could not be set.
TangoErrorType TangoConfig_setString(TangoConfig config, const char* key,
                                     const char* value);

/// Get a boolean configuration parameter.
/// @param config The configuration object to get the parameter from. config
/// must have been created with TangoConfig_getConfig().
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns <code>TANGO_SUCCESS</code> on success or
/// <code>TANGO_INVALID</code> if the any of the arguments is NULL, or if the
/// key could not be found.
TangoErrorType TangoConfig_getBool(TangoConfig config, const char* key,
                                   bool* value);

/// Get a uint32_t configuration parameter.
/// @param config The configuration object to get the parameter from. config
/// must have been created with TangoConfig.
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns <code>TANGO_SUCCESS</code> on success or
/// <code>TANGO_INVALID</code> if the any of the arguments is NULL, or if the
/// key could not be found.
TangoErrorType TangoConfig_getInt32(TangoConfig config, const char* key,
                                    int32_t* value);

/// Get an uint64_t configuration parameter.
/// @param config The configuration object to get the parameter from. config
/// must have been created with TangoConfig.
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns <code>TANGO_SUCCESS</code> on success or
/// <code>TANGO_INVALID</code> if the any of the arguments is NULL, or if the
/// key could not be found.
TangoErrorType TangoConfig_getInt64(TangoConfig config, const char* key,
                                    int64_t* value);

/// Get a double configuration parameter.
/// @param config The configuration object to get the parameter from. config
/// must have been created with TangoConfig,
/// @param key The string key value of the configuration parameter to set.
/// @param value The value to set the configuration key to.
/// @return Returns <code>TANGO_SUCCESS</code> on success or
/// <code>TANGO_INVALID</code> if the any of the arguments is NULL, or if the
/// key could not be found.
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
/// @return Returns <code>TANGO_SUCCESS</code> on success or
/// <code>TANGO_INVALID</code> if the any of the arguments is NULL, or if the
/// key could not be found.
TangoErrorType TangoConfig_getString(TangoConfig config, const char* key,
                                     char* value, size_t size);
/**@} */

// Experimental API only, subject to change.  Connect a Texture IDs to a camera.
// The camera is selected via TangoCameraId.
// Currently only TANGO_CAMERA_COLOR is supported.
// The texture handles will be regenerated by the API on startup after which
// the application can use them, and will be packed RGBA8888 data containing
// bytes of the image (so a single RGBA8888 will pack 4 neighbouring pixels).
// If the config flag experimental_image_pixel_format is set to
// HAL_PIXEL_FORMAT_YCrCb_420_SP, texture_y will pack 1280x720 pixels into
// a 320x720 RGBA8888 texture.
// texture_Cb and texture_Cr will contain copies of the 2x2 downsampled
// interleaved UV planes packed similarly.
// If experimental_image_pixel_format is set to HAL_PIXEL_FORMAT_YV12
// then texture_y will have a stride of 1536 containing 1280 columns of data,
// packed similarly in a RGBA8888 texture.
// texture_Cb and texture_Cr will be 2x2 downsampled versions of the same.
// See YV12 and NV21 formats for details.
//
// Note: The first scan-line of the color image is reserved for metadata
// instead of image pixels.
// @param id The ID of the camera to connect this texture to.  Only
// TANGO_CAMERA_COLOR and TANGO_CAMERA_FISHEYE are supported.
// @param context The context returned during the onFrameAvailable callback.
// @param texture_y The texture ID to use for the Y-plane.
// @param texture_Cb The texture ID to use for a chroma plane.
// @param texture_Cr The texture ID to use for a chroma plane.
// @return Returns <code>TANGO_INVALID</code> if the camera ID is not valid.
// Otherwise returns <code>TANGO_ERROR</code> if an internal error occurred.
TangoErrorType TangoService_Experimental_connectTextureIdUnity(
    TangoCameraId id, unsigned int texture_y, unsigned int texture_Cb,
    unsigned int texture_Cr, void* context,
    void (*callback)(void*, TangoCameraId));

// Experimental API only, subject to change.
// A mesh, described by vertices and face indices, with optional per-vertex
// normals and colors.
typedef struct TangoMesh_Experimental {
  // Index into a three-dimensional fixed grid.
  int32_t index[3];

  // Array of vertices. Each vertex is an {x, y, z} coordinate triplet, in
  // meters.
  float (*vertices)[3];

  // Array of faces. Each face is an index triplet into the vertices array.
  uint32_t (*faces)[3];

  // Array of per-vertex normals. Each normal is a normalized {x, y, z} vector.
  float (*normals)[3];

  // Array of per-vertex colors. Each color is a 4-tuple of 8-bit {R, G, B, A}
  // values.
  uint8_t (*colors)[4];

  // Number of vertices, describing the size of the vertices array.
  uint32_t num_vertices;

  // Number of faces, describing the size of the faces array.
  uint32_t num_faces;

  // If true, each vertex will have an associated normal. In that case, the
  // size of the normals array will be equal to num_vertices. Otherwise, the
  // size of the normals array will be 0.
  bool has_normals;

  // If true, each vertex will have an associated color. In that case, the size
  // of the colors array will be equal to num_vertices. Otherwise, the size of
  // the colors array will be 0.
  bool has_colors;
} TangoMesh_Experimental;

// Experimental API only, subject to change.
// Metadata from the scene reconstruction.
typedef struct TangoReconstructionMetadata_Experimental {
  // Number of volumes allocated.
  int64_t num_volumes_allocated;

  // The allocated memory by the volumes in bytes.
  int64_t volumes_memory_size;

  // The volumes dimension in meters.
  float volume_size;

  // The entire grid bounding box lower boundary in meters.
  float bbx_min[3];

  // The entire grid bounding box upper boundary in meters.
  float bbx_max[3];
} TangoReconstructionMetadata_Experimental;

// Experimental API only, subject to change.
// Metadata describing a uniform three-dimensional grid.
typedef struct TangoGridMetadata_Experimental {
  // The grid cell size, in meters.
  float cell_size;

  // The lower bounding index (inclusive) of the grid. The index is a
  // {ix, iy, iz} triplet describing the position on the grid.
  int32_t minimum_index[3];

  // The upper bounding index (inclusive) of the grid. The index is a
  // {ix, iy, iz} triplet describing the position on the grid.
  int32_t maximum_index[3];

  // Number of grid cells that have been instantiated.
  uint32_t num_active_cells;
} TangoGridMetadata_Experimental;

// Experimental API only, subject to change.
// Metadata describing a dense reconstruction.
typedef struct TangoDenseReconstructionMetadata_Experimental {
  // Describes the grid used for dense reconstruction.
  TangoGridMetadata_Experimental grid_metadata;
} TangoDenseReconstructionMetadata_Experimental;

// Experimental API only, subject to change.
// Attach a TangoService_onMeshVectorAvailableCallback callback. The callback is
// is invoked when a new array of mesh segments is available. Typically, a new
// array of meshes will be sent every time the scene reconstruction receives new
// depth input.
// @param TangoService_onMeshVectorAvailableCallback Function called when a
// array of meshes is available.
// @return Returns <code>TANGO_SUCCESS</code> if the callback can be
// attached. Returns <code>TANGO_ERROR</code> if a connection cannot be
// initialized.
TangoErrorType TangoService_Experimental_connectOnMeshVectorAvailable(
    void (*TangoService_onMeshVectorAvailableCallback)(
        void* context, const int num_meshes,
        const TangoMesh_Experimental* mesh_segments), ...);

// Experimental API only, subject to change.
// Starts the scene reconstruction. After this function is called, the scene
// reconstruction server will begin receiving depth data updates and publishing
// the reconstruction meshes.
// @return Returns <code>TANGO_SUCCESS</code> if the reconstruction can be
// started. Returns <code>TANGO_ERROR</code> if a connection was not
// initialized with TangoService_initialize(). Returns
// <code>TANGO_INVALID</code> if the scene reconstruction server was not set up
// during initialization, which can happen if the
// config_experimental_enable_scene_reconstruction flag was not enabled.
TangoErrorType TangoService_Experimental_startSceneReconstruction();

// Experimental API only, subject to change.
// Stops the scene reconstruction. After this function is called, the scene
// reconstruction server will stop receiving depth data updates and publishing
// the reconstruction meshes. The reconstruction itself will not be cleared.
// @return Returns <code>TANGO_SUCCESS</code> if the reconstruction can be
// stopped. Returns <code>TANGO_ERROR</code> if a connection was not
// initialized with TangoService_initialize(). Returns
// <code>TANGO_INVALID</code> if the scene reconstruction server was not set up
// during initialization, which can happen if the
// config_experimental_enable_scene_reconstruction flag was not enabled.
TangoErrorType TangoService_Experimental_stopSceneReconstruction();

// Experimental API only, subject to change.
// Resets the scene reconstruction by clearing all the stored data. Can be
// called both when the scene reconstruction running and stopped.
// @return Returns <code>TANGO_SUCCESS</code> if the reconstruction can be
// reset. Returns <code>TANGO_ERROR</code> if a connection was not initialized
// with TangoService_initialize(). Returns <code>TANGO_INVALID</code> if the
// scene reconstruction server was not set up during initialization, which can
// happen if the config_experimental_enable_scene_reconstruction flag was not
// enabled.
TangoErrorType TangoService_Experimental_resetSceneReconstruction();

// Experimental API only, subject to change.
// Extracts the full mesh from the scene reconstruction.
// @param mesh A pointer to the variable to store the output mesh to.
// @return Returns <code>TANGO_SUCCESS</code> if mesh was successfully
// extracted. Returns <code>TANGO_ERROR</code> if a connection was not
// initialized with TangoService_initialize(). Returns
// <code>TANGO_INVALID</code> if the scene reconstruction server was not set up
// during initialization, which can happen if the
// config_experimental_enable_scene_reconstruction flag was not enabled.
TangoErrorType TangoService_Experimental_extractMesh(
    TangoMesh_Experimental* mesh);

// Experimental API only, subject to change.
// Gets the metadata from the scene reconstruction.
// @param metadata A pointer to the variable to store the output metadata.
// @return Returns <code>TANGO_SUCCESS</code> if metadata was successfully
// extracted. Returns <code>TANGO_ERROR</code> if a connection was not
// initialized with TangoService_initialize(). Returns
// <code>TANGO_INVALID</code> if the scene reconstruction server was not set up
// during initialization, which can happen if the
// config_experimental_enable_scene_reconstruction flag was not enabled.
TangoErrorType TangoService_Experimental_getReconstructionMetadata(
    TangoReconstructionMetadata_Experimental* metadata);

// The 3D position of a point relative to an arbitrary reference frame.
typedef struct TangoPositionData_Experimental {
  /// 3D position ordered x, y, z.
  double position[3];
} TangoPositionData_Experimental;

/// Experimental API only, subject to change.
/// Returns a trajectory from the current device position to the specified goal
/// position. This will only work when successfully relocalized against an ADF.
/// The input position and the output trajectory are relative to base_frame;
/// allowed frames are TANGO_COORDINATE_FRAME_AREA_DESCRIPTION (always) and
/// TANGO_COORDINATE_FRAME_GLOBAL_WGS84 (only if the ADF's"transformation"
/// metadata field populated).
/// @param goal_position_in_base_frame The 3D coordinate of the goal position.
/// @param base_frame Coordinate frame for the goal and output trajectory.
/// @param[out] trajectory_size The length of the returned trajectory.
/// @param[out] trajectory The planned trajectory.
/// @return Returns <code>TANGO_SUCCESS</code> if a trajectory from the current
/// position to goal position was found. <code>TANGO_INVALID</code> can occur if
/// the your current position in the loaded ADF is not known, if no ADF is
/// loaded, if base_frame is set to an unsupported value, or if the trajectory
/// planner failed to initialize. Returns <code>TANGO_ERROR</code> if
/// communication fails or if the service needs to be initialized.
TangoErrorType TangoService_Experimental_getTrajectoryToGoal(
    const TangoPositionData_Experimental goal_position_in_base_frame,
    const TangoCoordinateFrameType base_frame, size_t* trajectory_size,
    TangoPositionData_Experimental** trajectory);

/// Experimental API only, subject to change.
/// Returns a trajectory between the specified start and goal positions.
/// This will only work when successfully relocalized against an ADF.
/// The input positions and the output trajectory are relative to base_frame;
/// allowed frames are TANGO_COORDINATE_FRAME_AREA_DESCRIPTION (always) and
/// TANGO_COORDINATE_FRAME_GLOBAL_WGS84 (only if the ADF's"transformation"
/// metadata field populated).
/// @param start_position_in_base_frame The 3D coordinate of the start position.
/// @param goal_position_in_base_frame The 3D coordinate of the goal position.
/// @param base_frame Coordinate frame for the goal and output trajectory.
/// @param[out] trajectory_size The length of the returned trajectory.
/// @param[out] trajectory The planned trajectory.
/// @return Returns <code>TANGO_SUCCESS</code> if a trajectory from the current
/// position to goal position was found. <code>TANGO_INVALID</code> can occur if
/// the your current position in the loaded ADF is not known, if no ADF is
/// loaded, if base_frame is set to an unsupported value, or if the trajectory
/// planner failed to initialize. Returns <code>TANGO_ERROR</code> if
/// communication fails or if the service needs to be initialized.
TangoErrorType TangoService_Experimental_getTrajectoryFromStartToGoal(
    const TangoPositionData_Experimental start_position_in_base_frame,
    const TangoPositionData_Experimental goal_position_in_base_frame,
    const TangoCoordinateFrameType base_frame, size_t* trajectory_size,
    TangoPositionData_Experimental** trajectory);

/// Free a trajectory created by a call to
/// TangoService_Experimental_getTrajectoryToGoal or to
/// TangoService_Experimental_getTrajectoryFromStartToGoal.
/// A trajectory should only be freed if the returned trajectory_size > 0.
TangoErrorType TangoService_Experimental_freeTrajectory(
    TangoPositionData_Experimental** trajectory);

/// Experimental API only, subject to change.
/// Loads an area description with the specified unique ID. This allows an
/// application to load an ADF for relocalization after connecting to the
/// service. It should only be called after calling TangoService_connect(), and
/// then only if the connect configuration did not specify an ADF to load, and
/// did not enable learning mode.
/// @param uuid The unique identifier for the ADF to load.
/// @return Returns <code>TANGO_SUCCESS</code> if the ADF is successfully loaded
/// for relocalization. Returns <code>TANGO_INVALID</code> if the UUID is
/// invalid, or an ADF is already being learned or has already been
/// loaded. Returns <code>TANGO_ERROR</code> if communication fails or if the
/// service needs to be initialized.
TangoErrorType TangoService_Experimental_loadAreaDescription(
    const TangoUUID uuid);

/// Experimental API only, subject to change.
/// Loads an area description with the specified file path. This allows an
/// application to load an ADF for relocalization after connecting to the
/// service. It should only be called after calling TangoService_connect(), and
/// then only if the connect configuration did not specify an ADF to load, and
/// did not enable learning mode.
/// @param file_path The file path for the ADF to load.
/// @return Returns <code>TANGO_SUCCESS</code> if the ADF is successfully loaded
/// for relocalization. Returns <code>TANGO_INVALID</code> if the file path is
/// invalid, or an ADF is already being learned. Returns
/// <code>TANGO_ERROR</code> if communication fails or if the service needs to
/// be initialized.
TangoErrorType TangoService_Experimental_loadAreaDescriptionFromFile(
    const char* file_path);

/// Experimental API only, subject to change.
/// Returns a list of Tango dataset UUIDs, containing a UUID for each valid
/// dataset recorded to the Tango Service application directory. Memory will be
/// dynamically allocated by this call, and must be deallocated using
/// TangoService_Experimental_ReleaseDatasetUUIDs. The service does not
/// need to be connected in order to call this function.
/// @param dataset_uuids a dynamic array of TangoUUIDs. Will be dynamically
/// allocated by this call.
/// @param num_dataset_uuids An output parameter for the number of dataset
/// UUIDs returned.
/// @return Returns <code>TANGO_SUCCESS</code> if the dataset UUIDs were
/// successfully retrieved. Returns <code>TANGO_ERROR</code> if communication
/// failed or the datasets could not be enumerated. Returns <code>
/// TANGO_NO_DATASET_PERMISSION</code> if user has not given permissions to read
/// and write datasets. Returns <code>TANGO_INVALID</code> if any of the
/// arguments passed were NULL.
TangoErrorType TangoService_Experimental_getDatasetUUIDs(
    TangoUUID** dataset_uuids, int* num_dataset_uuids);

/// Experimental API only, subject to change
/// Frees up memory allocated by TangoService_Experimental_GetDatasetUUIDs
/// @param dataset_uuids a dynamic array of TangoUUIDs. Will be deallocated by
/// this call.
/// @returns Returns <code>TANGO_INVALID</code> if any of the arguments passed
/// were NULL, and <code>TANGO_SUCCESS</code> otherwise.
TangoErrorType TangoService_Experimental_releaseDatasetUUIDs(
    TangoUUID** dataset_uuids);

/// Experimental API only, subject to change.
/// Deletes a dataset.
/// @param dataset_uuid The ID of the dataset to be deleted.
/// @return Returns <code>TANGO_SUCCESS</code> if the dataset was successfully
/// deleted. Returns <code>TANGO_ERROR</code> if communication failed or the
/// datasets could not be found. Returns
/// <code>TANGO_NO_DATASET_PERMISSION</code> if user has not given permissions
/// to read and write datasets.
TangoErrorType TangoService_Experimental_deleteDataset(
    const TangoUUID dataset_uuid);

/// Experimental API only, subject to change.
/// Gets the current dataset UUID.
/// @param dataset_uuid The current dataset UUID.
/// @return Returns <code>TANGO_SUCCESS</code> if the id was successfully
/// retrieved. Returns <code>TANGO_ERROR</code> if communication failed.
TangoErrorType TangoService_Experimental_getCurrentDatasetUUID(
    TangoUUID* dataset_uuid);
#ifdef __cplusplus
}
#endif

#endif  // TANGO_CLIENT_API_H_
