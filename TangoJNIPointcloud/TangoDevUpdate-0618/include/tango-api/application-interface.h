// Copyright 2013 Motorola Mobility LLC. Part of the Trailmix project.
// CONFIDENTIAL. AUTHORIZED USE ONLY. DO NOT REDISTRIBUTE.
/*! \file */

/// \mainpage Tango API Documentation
/// \section Overview
/// These pages document the Tango C-API.
///
/// Functions are divided in to groups, called "modules".
///
/// To start, see the modules page for a listing.
#ifndef TANGO_API_APPLICATION_INTERFACE_H_
#define TANGO_API_APPLICATION_INTERFACE_H_
#include <tango-api/public-api.h>

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup application-interface Application Interface Module

/// \brief Initialize the library and construct Application context.
///
/// ApplicationInitialize initializes the library and returns a context
/// pointer.  ApplicationInitialize must be called before other library
/// functions are used. The application context is an general library context,
/// within which it operates.
///
/// The data source argument specifies the input data source which can be either
/// live camera data or a pre-recorded dataset.  To initialize camera input,
/// the string "[Superframes Small-Peanut]" should be passed.  To initialize
/// a pre-recorded dataset as the input, the path to the recorded dataset
/// should be provided.  The datasets must be in the format recorded by the
/// TangoMapper application.
///
/// \return A handle to a newly created application context.
/// \param data_source A string containing the name of the data source.  This
/// can either be the string "[Superframes Small-Peanut]" to use the cameras
///  as a data source, or a path to a pre-recorded dataset.
/// \param request_point_cloud data (Yellowstone Only)
/// \warning The data_source parameter is likely to change in future versions.
/// \ingroup application-interface
PUBLIC_C_API application_handle_t* ApplicationInitialize(
    const char* data_source, int request_point_cloud = 0);

/// \brief Propagates forward engine state, so it is available to an API user.
///
/// ApplicationDoStep interface synchronizes Tango Engine and API context,
/// so the user can access latest Tango engine state.
///
/// In particular, it allows internal image buffers to be updated,
/// for the next set of queries.
/// In between DoStep calls, the latest color and depth images will remain
/// constant.
///
/// \param application The application context handle.
/// \return On failure, an error code is returned.
/// \ingroup application-interface
PUBLIC_C_API CAPIErrorCodes ApplicationDoStep(
    application_handle_t* application);

/// \brief Shuts down the library.
/// \param application The application context handle.
/// \return On success, returns kCAPISuccess.  On failure returns an error code.
/// \ingroup application-interface
PUBLIC_C_API CAPIErrorCodes
    ApplicationShutdown(application_handle_t* application);

#ifdef __cplusplus
}
#endif

#endif  // TANGO_API_APPLICATION_INTERFACE_H_
