#ifndef __TANGO_CLIENT_API_H__
#define __TANGO_CLIENT_API_H__

#include <stdint.h>
#include <stdbool.h>

// Tango Camera enumerations.
typedef enum {
  TANGO_CAMERA_COLOR,
  TANGO_CAMERA_RGBIR,
  TANGO_CAMERA_FISHEYE,
  TANGO_CAMERA_DEPTH
} TangoCameraId;

// Tango runtime configuration enumerations.
typedef enum {
  TANGO_CONFIG_DEFAULT,
  TANGO_CONFIG_CURRENT,
  TANGO_CONFIG_MOTION_TRACKING,
  TANGO_CONFIG_AREA_DESCRIPTION
} TangoConfigType;

// Tango coordinate frame enumerations.
typedef enum {
  TANGO_COORDINATE_FRAME_GLOBAL,
  TANGO_COORDINATE_FRAME_AREA_DESCRIPTION,
  TANGO_COORDINATE_FRAME_START_OF_SERVICE,
  TANGO_COORDINATE_FRAME_PREVIOUS_DEVICE_POSE,
  TANGO_COORDINATE_FRAME_DEVICE,
  TANGO_COORDINATE_FRAME_IMU,
  TANGO_COORDINATE_FRAME_DISPLAY,
  TANGO_COORDINATE_FRAME_CAMERA_COLOR,
  TANGO_COORDINATE_FRAME_CAMERA_DEPTH,
  TANGO_COORDINATE_FRAME_CAMERA_FISHEYE
} TangoCoordinateFrameType;

struct TangoConfig;
typedef struct TangoConfig TangoConfig;

typedef struct TangoPoseData {
  int version;
  double timestamp;
  double orientation[4];
  double translation[3];
} TangoPoseData;

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

// Initialize the TangoService.
int TangoService_initialize();

// Get a configuration template.
int TangoService_getConfig(TangoConfigType config_type, TangoConfig *config);

// Lock a new configuration.
int TangoService_lockConfig(TangoConfig *config);

// Unlock the tango service.
int TangoService_unlockConfig();

// Connect to the Tango Service.
int TangoService_connect();

// Disconnect from the Tango Service.
void TangoService_disconnect();

// Attach an onPoseAvailable callback.
int TangoService_connectOnPoseAvailable(
  void (*TangoService_onPoseAvailable)(TangoPoseData *pose));

// Attach an onXYZijAvailable callback.
int TangoService_connectOnXYZijAvailable(
    void (*TangoService_onXYZijAvailable)(TangoXYZij *xyz_ij));

// Connect a Texture ID to a camera.
int TangoService_connectTextureId(TangoCameraId, int tex);

// Connect a Texture ID to a camera.
int TangoService_updateTexture(TangoCameraId);

// Get a pose at a given timestamp from the reference to the target frame.
int TangoService_getPoseAtTime(double timestamp,
                               TangoCoordinateFrameType target_frame,
                               TangoCoordinateFrameType reference_frame,
                               TangoPoseData *pose, int pose_size);

// Allocate a TangoConfig object.
TangoConfig *TangoConfig_alloc();

// Deallocate a TangoConfig object.
void TangoConfig_free(TangoConfig *config);

// Allocate and return a string with one key=value pair per line.
char *TangoConfig_toString(TangoConfig *config);

int TangoConfig_setBool(TangoConfig *config, const char *key, bool value);

int TangoConfig_setInt32(TangoConfig *config, const char *key, int32_t value);

int TangoConfig_setInt64(TangoConfig *config, const char *key, int64_t value);

int TangoConfig_setDouble(TangoConfig *config, const char *key, double value);

int TangoConfig_setString(TangoConfig *config, const char *key,
                          const char *value);

int TangoConfig_getBool(TangoConfig *config, const char *key, bool *value);

int TangoConfig_getInt32(TangoConfig *config, const char *key, int32_t *value);

int TangoConfig_getInt64(TangoConfig *config, const char *key, int64_t *value);

int TangoConfig_getDouble(TangoConfig *config, const char *key, double *value);

int TangoConfig_getString(TangoConfig *config, const char *key, char *value,
                          size_t size);

#ifdef __cplusplus
}
#endif

#endif
