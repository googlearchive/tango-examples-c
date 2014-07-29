// Copyright 2013 Motorola Mobility LLC. Part of the Trailmix project.
// CONFIDENTIAL. AUTHORIZED USE ONLY. DO NOT REDISTRIBUTE.

#ifndef TANGO_API_PUBLIC_API_H_
#define TANGO_API_PUBLIC_API_H_

#ifndef PUBLIC_C_API
#  if __GNUC__ >= 4
#    define PUBLIC_C_API __attribute__((visibility("default")))
#  else
#    define PUBLIC_C_API
#  endif
#endif

struct application_handle_s;
typedef struct application_handle_s application_handle_t;

struct StatisticsData;
typedef StatisticsData StatisticsDataStructType;

struct video_overlay_handle_s;
typedef struct video_overlay_handle_s video_overlay_handle_t;

struct vio_handle_s;
typedef struct vio_handle_s vio_handle_t;

enum CAPIErrorCodes {
  kCAPISuccess = 0,
  kCAPIFail = -1,
  kCAPINotImplemented = -2,
  kCAPIAlreadyInstantiated = 100,
  kCAPINotInitialized = 101,
  kCAPIDataNotAvailable = 102,
  kCAPIWrongInputData = 103,
  kCAPIOperationFailed = 104
};

const int kAPIVersionMajor = 1;
const int kAPIVersionMinor = 3;

#endif  // TANGO_API_PUBLIC_API_H_
