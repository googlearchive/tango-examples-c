// Copyright 2013 Motorola Mobility LLC. Part of the Trailmix project.
// CONFIDENTIAL. AUTHORIZED USE ONLY. DO NOT REDISTRIBUTE.

#ifndef TANGO_API_UTIL_INTERFACE_H_
#define TANGO_API_UTIL_INTERFACE_H_

#include <tango-api/public-api.h>

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup util-interface TangoAPI Utilities Interface Module

#pragma pack(push, 4)
struct StatisticsData {
  int frames_tested;
  int tears;
  int jumps;
  int stutters;
  unsigned int write_queue_backlog;
  unsigned int number_of_written_frames;
  unsigned int validation_result;
  int exposure;
};
#pragma pack(pop)

/// \brief Return size of StatisticsData structure in bytes.
///
/// \return Size of StatisticsData structure in bytes.
PUBLIC_C_API int UtilStatisticsSizeofStatsData();

/// \brief Debug interface to retrieve camera health statistics.
///
/// \param data camera stack health statistics.
/// \return On success, returns kCAPISuccess.  On failure returns an error code.
PUBLIC_C_API CAPIErrorCodes UtilGetStatistics(StatisticsDataStructType* data);

/// \brief Return TangoAPI version string.
///
/// \param version_buffer Null terminated string that describes TangoAPI
/// version.
/// \param version_buffer Version string buffer size.
/// \return On success, returns kCAPISuccess.  On failure returns an error code.
PUBLIC_C_API void UtilAPIVersionString(char* version_buffer, int max_size);

/// \brief Convert a pose from Estimator convention to Unity convention.
///
/// Converts a pose from Estimator convention to Unity convention.  Internally
/// the Tango libraries use the "Estimator" convention, which must be converted
/// into Unity convention for use in Unity.
/// \param estimator_format_rotation Pointer to a quaternion in Estimator Format
///   of the input pose.
/// \param estimator_format_translation Pointer to a translation vector in
///   Estimator Format of the input pose.
/// \param unity_format_rotation Pointer to a quaternion to be
///   filled upon return with the converted rotation in Unity convention.
/// \param unity_format_translation Pointer to a translation vector to be filled
///   upon return with the converted position, in Unity convention.
/// \ingroup util-interface
/// \return On success, returns kCAPISuccess.  On failure returns an error code.
PUBLIC_C_API CAPIErrorCodes UtilConvertPoseToUnityFormat(
    const float* estimator_format_rotation,
    const float* estimator_format_translation, float* unity_format_rotation,
    float* unity_format_translation);

/// \brief Convert a pose from Estimator convention to Android convention.
/// Same as UtilConvertPoseToUnityFormat, except it returns pose in Android
/// format.
PUBLIC_C_API CAPIErrorCodes UtilConvertPoseToAndroidFormat(
    const float* estimator_format_rotation,
    const float* estimator_format_translation, float* android_format_rotation,
    float* android_format_translation);

/// \brief Convert a pose from Estimator convention to OpenGL convention.
/// Same as UtilConvertPoseToUnityFormat, except it returns pose in OpenGL
/// format.
PUBLIC_C_API CAPIErrorCodes UtilConvertPoseToOpenGLFormat(
    const float* estimator_format_rotation,
    const float* estimator_format_translation, float* opengl_format_rotation,
    float* opengl_format_translation);

#ifdef __cplusplus
}
#endif

#endif  // TANGO_API_UTIL_INTERFACE_H_
