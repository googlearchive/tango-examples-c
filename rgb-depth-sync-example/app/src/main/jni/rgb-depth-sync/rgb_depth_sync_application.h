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

#ifndef RGB_DEPTH_SYNC_APPLICATION_H_
#define RGB_DEPTH_SYNC_APPLICATION_H_

#include <jni.h>

#include <tango_client_api.h>
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

  // Initialize the Tango Service, this function starts the communication
  // between the application and the Tango Service.
  // The activity object is used for checking if the API version is outdated
  int TangoInitialize(JNIEnv* env, jobject caller_activity);

  // Setup the configuration file for the Tango Service. .
  int TangoSetupConfig();

  // Associate the texture generated from an Opengl context to which the color
  // image will be updated to.
  int TangoConnectTexture();

  // Sets the callbacks for OnXYZijAvailable
  int TangoConnectCallbacks();

  // Connect to Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start Depth Sensing callbacks.
  int TangoConnect();

  // Queries and sets the camera transforms between different sensors of
  // Project Tango Device that are required to project Point cloud onto
  // Image plane.
  int TangoSetIntrinsicsAndExtrinsics();

  // Disconnect from Tango Service.
  void TangoDisconnect();

  // Inititalizes all the OpenGL resources required to render a Depth Image on
  // Top of an RGB image.
  void InitializeGLContent();

  // Setup the view port width and height.
  void SetViewPort(int width, int height);

  // Main Render loop.
  void Render();

  // Set the transparency of Depth Image.
  void SetDepthAlphaValue(float alpha);

  // Set whether to use GPU or CPU upsampling
  void SetGPUUpsample(bool on);

  // Callback for point clouds that come in from the Tango service.
  //
  // @param xyz_ij The point cloud returned by the service.
  //
  void OnXYZijAvailable(const TangoXYZij* xyz_ij);

 private:
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

  // Extrinsic transformation of color frame wrt device frame.
  glm::mat4 device_T_color_;

  // Extrinsic transformation of depth frame wrt device frame.
  glm::mat4 device_T_depth_;

  // OpenGL to Start of Service
  glm::mat4 OW_T_SS_;
  float screen_width_;
  float screen_height_;

  // This is the buffer to which point cloud data from TangoService callback
  // gets copied out to.
  // The data is an array of packed coordinate triplets, x,y,z as floating point
  // values. With the unit in landscape orientation, screen facing the user:
  // +Z points in the direction of the camera's optical axis, and is measured
  // perpendicular to the plane of the camera.
  // +X points toward the user's right, and +Y points toward the bottom of
  // the screen.
  // The origin is the focal centre of the color camera.
  // The output is in units of metres.
  std::vector<float> callback_point_cloud_buffer_;

  // The buffer of point cloud data which is shared between TangoService
  // callback and render loop.
  std::vector<float> shared_point_cloud_buffer_;

  // This buffer is used in the render loop to project point cloud data to
  // a 2D image plane which is of the same size as RGB image.
  std::vector<float> render_point_cloud_buffer_;

  // Time of capture of the current depth data (in seconds).
  double depth_timestamp_;

  // Mutex for protecting the point cloud data. The point cloud data is shared
  // between update call which is called from render loop and
  // TangoService callback thread.
  std::mutex point_cloud_mutex_;

  // This signal is used to notify update call if there is a new
  // point cloud buffer available and swap the shared and render buffers
  // accordingly.
  bool swap_signal;

  bool gpu_upsample_;
};
}  // namespace rgb_depth_sync

#endif  // RGB_DEPTH_SYNC_APPLICATION_H_
