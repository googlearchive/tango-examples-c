/*
 * Copyright 2014 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CPP_RGB_DEPTH_SYNC_EXAMPLE_RGB_DEPTH_SYNC_RGB_DEPTH_SYNC_APPLICATION_H_
#define CPP_RGB_DEPTH_SYNC_EXAMPLE_RGB_DEPTH_SYNC_RGB_DEPTH_SYNC_APPLICATION_H_

#include <jni.h>
#include <vector>

#include <tango_client_api.h>
#include <tango_support.h>
#include <rgb-depth-sync/color_image.h>
#include <rgb-depth-sync/depth_image.h>
#include <rgb-depth-sync/scene.h>
#include <rgb-depth-sync/util.h>
#include <tango-gl/util.h>

namespace rgb_depth_sync {

// This thread safe class is the main application for Synchronization.
// It can be instantiated in the JNI layer and use to pass information back and
// forth between Java. The class also manages the application's lifecycle and
// interaction with the Tango service. Primarily, this involves registering for
// callbacks and passing on the necessary information to stored objects. It also
// takes care of passing a vector container which has a pointer to the
// latest point cloud buffer that is to used for rendering.
//  To reduce the number of point cloud data copies between callback and render
// threads we use three buffers which are synchronously exchanged between each
// other so that the render loop always contains the latest point cloud data.
// 1. Callback buffer : The buffer to which pointcloud data received from Tango
// Service callback is copied out.
// 2. Shared buffer: This buffer is used to share the data between Service
// callback and Render loop
// 3. Render Buffer: This buffer is used in the renderloop to project point
// cloud data to a 2D image plane which is of the same size as RGB image. We
// also make sure that this buffer contains the latest point cloud data that is
//  received from the call back.
class SynchronizationApplication {
 public:
  SynchronizationApplication();
  ~SynchronizationApplication();

  // OnCreate() callback is called when this Android application's
  // OnCreate function is called from UI thread. In the OnCreate
  // function, we are only checking the Tango Core's version.
  //
  // @param env, java environment parameter OnCreate is being called.
  // @param caller_activity, caller of this function.
  void OnCreate(JNIEnv* env, jobject caller_activity);

  // OnPause() callback is called when this Android application's
  // OnCreate function is called from UI thread. In our application,
  // we disconnect Tango Service and free the Tango configuration
  // file. It is important to disconnect Tango Service and release
  // the coresponding resources in the OnPause() callback from
  // Android, otherwise, this application will hold on to the Tango
  // resources and other application will not be able to connect to
  // Tango Service.
  void OnPause();

  // Called when Tango Service is connected successfully.
  void OnTangoServiceConnected(JNIEnv* env, jobject binder);

  // Inititalizes all the OpenGL resources required to render a Depth Image on
  // Top of an RGB image.
  void OnSurfaceCreated();

  // Setup the view port width and height.
  void OnSurfaceChanged(int width, int height);

  // Main Render loop.
  void OnDrawFrame();

  // Set the transparency of Depth Image.
  void SetDepthAlphaValue(float alpha);

  // Set whether to use GPU or CPU upsampling
  void SetGPUUpsample(bool on);

  // Callback for display change event, we use this function to detect display
  // orientation change.
  //
  // @param display_rotation, the rotation index of the display. Same as the
  // Android display enum value, see here:
  // https://developer.android.com/reference/android/view/Display.html#getRotation()
  // @param color_camera_rotation, the rotation index of color camera
  // orientation.
  // Same as the Android sensor rotation enum value, see here:
  // https://developer.android.com/reference/android/hardware/Camera.CameraInfo.html#orientation
  void OnDisplayChanged(int display_rotation, int color_camera_rotation);

  // Callback for point clouds that come in from the Tango service.
  //
  // @param point_cloud The point cloud returned by the service.
  //
  void OnPointCloudAvailable(const TangoPointCloud* point_cloud);

 private:
  // Setup the configuration file for the Tango Service. .
  void TangoSetupConfig();

  // Associate the texture generated from an Opengl context to which the color
  // image will be updated to.
  bool TangoConnectTexture();

  // Sets the callbacks for OnXYZijAvailable
  void TangoConnectCallbacks();

  // Connect to Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start Depth Sensing callbacks.
  void TangoConnect();

  // Queries and sets the camera transforms between different sensors of
  // Project Tango Device that are required to project Point cloud onto
  // Image plane.
  void TangoSetIntrinsics();

  // RGB image
  ColorImage color_image_;

  // Depth image created by projecting Point Cloud onto RGB image plane.
  DepthImage depth_image_;

  // Main scene which contains all the renderable objects.
  Scene main_scene_;

  // Tango configration file, this object is for configuring Tango Service setup
  // before connect to service. For example, we turn on the depth sensing in
  // this example.
  TangoConfig tango_config_;

  // The point_cloud_manager allows for thread safe reading and
  // writing of the point cloud data.
  TangoSupport_PointCloudManager* point_cloud_manager_;

  bool gpu_upsample_;

  bool is_service_connected_;
  bool is_gl_initialized_;

  TangoSupport_Rotation color_camera_to_display_rotation_;
};
}  // namespace rgb_depth_sync

#endif  // CPP_RGB_DEPTH_SYNC_EXAMPLE_RGB_DEPTH_SYNC_RGB_DEPTH_SYNC_APPLICATION_H_
