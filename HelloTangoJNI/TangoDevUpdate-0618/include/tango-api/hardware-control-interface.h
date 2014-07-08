// Copyright 2013 Motorola Mobility LLC. Part of the Trailmix project.
// CONFIDENTIAL. AUTHORIZED USE ONLY. DO NOT REDISTRIBUTE.
/*! \file */

#ifndef TANGO_API_HARDWARE_CONTROL_INTERFACE_H_
#define TANGO_API_HARDWARE_CONTROL_INTERFACE_H_

#include <tango-api/public-api.h>

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup hardware-control-interface Hardware Control Interface Module.

/// \brief Get the horizontal field of view of the RGB/IR camera, in degrees.
///
/// This is a helper function that returns RGB/IR camera horizontal FOV,
/// cropped to visible_height.
///
///
/// \param visible_height Cropped frame height.
/// \return camera FOV.
/// \ingroup hardware-controll-interface
PUBLIC_C_API float HardwareCameraHorizontalFieldOfView(
    int visible_height);

/// \brief Get the vertical field of view of the RGB/IR camera, in degrees.
///
/// This is a helper function that returns RGB/IR camera vertical FOV,
/// cropped to visible_width.
///
/// \param visible_width Cropped frame width.
/// \return camera FOV.
/// \ingroup hardware-controll-interface
PUBLIC_C_API float HardwareCameraVerticalFieldOfView(
    int visible_width);

/// \brief Request camera hardware to increment frame exposure.
///
/// Request camera hardware to increment frame exposure.
///
/// \param application The application context handle.
/// \return On success, returns kCAPISuccess.  On failure returns an error code.
/// \ingroup hardware-controll-interface
PUBLIC_C_API CAPIErrorCodes HardwareCameraIncrementExposure(
    application_handle_t* application);

/// \brief Request camera hardware to decrement frame exposure.
///
/// Request camera hardware to decrement frame exposure.
///
/// \param application The application context handle.
/// \return On success, returns kCAPISuccess.  On failure returns an error code.
/// \ingroup hardware-controll-interface
PUBLIC_C_API CAPIErrorCodes HardwareCameraDecrementExposure(
    application_handle_t* application);

// Video mode query functions.
struct VideoModeAttributes {
  int width;
  int height;
  float framerate;
};

PUBLIC_C_API int HardwareCameraSizeofVideoMode();
PUBLIC_C_API int HardwareCameraNumberVideoModes();
PUBLIC_C_API CAPIErrorCodes HardwareCameraGetVideoMode(int video_mode_ID,
    void* video_mode);

// Depth functions.
PUBLIC_C_API CAPIErrorCodes HardwareDepthSetConfidenceLevel(
    int confidence_level);
PUBLIC_C_API CAPIErrorCodes HardwareDepthSetNoiseLevel(int noise_level);
PUBLIC_C_API CAPIErrorCodes HardwareDepthConfidenceLevel(
    int* confidence_level);
PUBLIC_C_API CAPIErrorCodes HardwareDepthNoiseLevel(int* noise_level);
PUBLIC_C_API CAPIErrorCodes HardwareDepthGetMode(int depth_mode_ID,
    void* video_mode);
PUBLIC_C_API CAPIErrorCodes HardwareDepthSetMode(int depth_mode_ID);
#ifdef __cplusplus
}
#endif

#endif  // TANGO_API_HARDWARE_CONTROL_INTERFACE_H_
