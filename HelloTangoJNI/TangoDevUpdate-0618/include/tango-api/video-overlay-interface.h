// Copyright 2013 Motorola Mobility LLC. Part of the Trailmix project.
// CONFIDENTIAL. AUTHORIZED USE ONLY. DO NOT REDISTRIBUTE.

#ifndef TANGO_API_VIDEO_OVERLAY_INTERFACE_H_
#define TANGO_API_VIDEO_OVERLAY_INTERFACE_H_

#include <tango-api/public-api.h>

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup video-overlay-interface Interface Module

/// \brief Initialize and start video overlay engine.
///
/// Initialize and start video overlay engine.
/// This function can be performed only within valid OpenGLES2.0 context.
///
/// \param application_handle The application context handle.
/// \return On success, returns kCAPISuccess.  On failure returns an error code.
/// \ingroup video-overlay-interface
PUBLIC_C_API CAPIErrorCodes VideoOverlayInitialize(
    application_handle_t* application_handle);

/// \brief Render latest RGB frame into user texture.
///
/// Call this function to get latest RGB frame rendered to OpenGL texture.
///
/// This function can be performed only within valid OpenGLES2.0 context.
///
/// \param application_handle The application context handle.
/// \param texture_ID Target texture id.
/// \param width Target texture width.
/// \param height Target texture height.
/// \param actual_timestamp Rendered frame timestamp.
/// \return On success, returns kCAPISuccess.  On failure returns an error code.
/// \ingroup video-overlay-interface
PUBLIC_C_API CAPIErrorCodes VideoOverlayRenderLatestFrame(
    application_handle_t* application_handle, int texture_ID, int width,
    int height, double* actual_timestamp);

/// \brief Shutdown video overlay engine.
///
/// This function can be performed only within valid OpenGLES2.0 context.
///
/// \param application_handle The application context handle.
/// \return On success, returns kCAPISuccess.  On failure returns an error code.
/// \ingroup video-overlay-interface
PUBLIC_C_API CAPIErrorCodes VideoOverlayShutdown(
    application_handle_t* application_handle);


/// \brief Get a YUV420SP color image from the narrow FOV color camera.
/// (Experimental)
///
/// Caller must allocation width x height x 3 / 2 bytes of memory.
/// \param actual_timestamp Pointer to valid memory for storting a timestamp,
///    Must be set to 0.0 which retrieves the latest frame.
/// \param frame_buffer_size_in_bytes the size of the frame_buffer allocated.
/// \param fraem_buffer Caller allocated memory filled in by the call upon
///    successful return.
PUBLIC_C_API CAPIErrorCodes VideoOverlayCopyLatestFrame(
    application_handle_t* application_handle, double* actual_timestamp,
    int frame_buffer_size_in_bytes, unsigned char *frame_buffer);


#ifdef __cplusplus
}
#endif

#endif  // TANGO_API_VIDEO_OVERLAY_INTERFACE_H_
