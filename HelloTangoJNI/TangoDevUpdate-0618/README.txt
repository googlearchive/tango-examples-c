06-17 - 06-18 Changes
==============
- Removed gflags header dependency
- Additional function to get maximum size of point cloud (Experimental).  The
Calling application should allocate at least this much memory to send to
DepthGetPointCloud functions).

PUBLIC_C_API CAPIErrorCodes DepthGetPointCloudMaxSize(int* size);

- New libCUDAFeatureTracker.so.1.5.0.  See below for installation instructions,
following those for version 1.4.0 but using the 1.5.0 instead.

- Library is compatible with DVT1-2 and DVT2
- Color Camera ISO set to 100, Raw IR camera ISO set to 100
- Depth at 3Hz, typical 10,000 points on a cooperatie scene, binning mode
- Bug fixes:
   - keyframe-detector.cc bug sensor data coverage bug fixes
   - DepthThread crash bug fixes


06-16 Release:
==============
- Headers updated wtih new function documentation.
- Depth buffer support on Yellowstone.
- Depth Point Cloud support on Yellowstone (Experimental)

- Use additional initialization flag to request depth to be enabled (both point
  cloud and depth map require this). 
  
PUBLIC_C_API application_handle_t* ApplicationInitialize(
    const char* data_source, int request_point_cloud = 0);
    
- Point Clouds can be accessed by the following:

PUBLIC_C_API CAPIErrorCodes DepthGetPointCloud(
    application_handle_t* application,
    double* timestamp, double max_delta,
    float* depth_data_buffer,
    int* buffer_size_in_elements);
PUBLIC_C_API CAPIErrorCodes DepthGetPointCloudUnity(
    application_handle_t* application,
    double* timestamp, double max_delta,
    float* depth_data_buffer,
    int* buffer_size);

- New experimental ability to get color buffers.

PUBLIC_C_API CAPIErrorCodes VideoOverlayCopyLatestFrameExperimental(
    application_handle_t* application_handle, double* actual_timestamp,
    int frame_buffer_size_in_bytes, unsigned char *frame_buffer);
    
- New experimental Area Description handling interface
    
- Updated libCUDAFeatureTracker.so, more robust tracking under fast motions.
- Updated libMantis-arm.so and libStInfra-arm.so depth processing. 


Package Contents (06-17): 
=========================

├── app
│   ├── AndroidPlayer.apk
│   ├── AndroidVioService.apk
│   ├── PointcloudViewer.apk
│   ├── Tango-Dashboard.apk
│   ├── TangoMapper.apk
│   └── TangoService.apk
├── config
│   └── config_YS_binning_DVT1.2.cfg
├── include
│   └── tango-api
│       ├── application-interface.h
│       ├── depth-interface.h
│       ├── hardware-control-interface.h
│       ├── map-daytime-filter.h
│       ├── public-api.h
│       ├── util-interface.h
│       ├── video-overlay-interface.h
│       └── vio-interface.h
├── libs
│   ├── libCUDAFeatureTracker.so.1.4.0
│   ├── libCUDAFeatureTracker.so.1.5.0
│   ├── libMantis-arm.so.1.5.0
│   ├── libStInfra-arm.so.1.5.0
│   └── libtango_api.so
├── README.txt
├── sensor_hub_2.0.7_update.zip
└── TangoSDK.unitypackage

1.  Please check that your sensor hub version is 2.0.7.  This firmware version
modifies hardware settings required by the newer Tango software APKs and
libraries.  The version can be checked as shown below: 

jfung@tm2:~/Desktop$ adb root
jfung@tm2:~/Desktop$ adb shell
root@yellowstone:/ # stop 
root@yellowstone:/ # stop sensorhubd
root@yellowstone:/ # sh_updater -q
connecting to sensor hub
Bootloader version 3.1 detected.
Reading: |
Current firmware version 2.0.7
root@yellowstone:/ # start sensorhubd
root@yellowstone:/ # start

2. For depth install the new configuration file included in /config

adb push config/config_YS_binning_DVT1.2.cfg /persist/mantis/config

2.  (Recommended) Install and symlink a new version of libCUDAFeatureTracker.so

adb root
adb remount
adb push libs/libCUDAFeatureTracker.so.1.4.0 /system/lib/
adb shell rm /system/lib/libCUDAFeatureTracker.so
adb shell ln -s /system/lib/libCUDAFeatureTracker.so.1.4.0 /system/lib/libCUDAFeatureTracker.so

lrwxrwxrwx root     root              2000-01-07 14:34 libCUDAFeatureTracker.so -> libCUDAFeatureTracker.so.1.4.0
-rw-r--r-- root     root      2018748 2014-06-05 14:39 libCUDAFeatureTracker.so.1.0.0
-rw-r--r-- root     root      2018748 2014-06-05 14:39 libCUDAFeatureTracker.so.1.1.0
-rw-r--r-- root     root      2018772 2014-06-05 14:39 libCUDAFeatureTracker.so.1.2.0
-rw-r--r-- root     root      2018856 2014-06-05 14:39 libCUDAFeatureTracker.so.1.3.0
-rwxrwxrwx root     root      2037448 2014-06-06 02:10 libCUDAFeatureTracker.so.1.4.0

3.  (Recommended) Install and symlink a new version of libMantis-arm.so and libStInfra-arm.so
Use commands similar to above to create symlinks: 

lrwxrwxrwx root     root              2000-01-08 09:12 libMantis-arm.so -> libMantis-arm.so.1.5.0
lrwxrwxrwx root     root              2000-01-08 09:12 libStInfra-arm.so -> libStInfra-arm.so.1.5.0

4.  Install and use:

DevUpdate/
├── app
│   ├── AndroidVioService.apk
│   ├── TangoMapper.apk
│   └── TangoService.apk

For applications that use the C-API, the shared library is here:

├── libs
│   └── libtango_api.so

Future updates to system files will be handled via OTA. 



