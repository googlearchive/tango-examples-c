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

#ifndef TANGO_AUGMENTED_REALITY_AUGMENTED_REALITY_APP_H_
#define TANGO_AUGMENTED_REALITY_AUGMENTED_REALITY_APP_H_

#include <jni.h>
#include <memory>

#include <tango_client_api.h>  // NOLINT
#include <tango-gl/util.h>

#include <tango-augmented-reality/pose_data.h>
#include <tango-augmented-reality/scene.h>
#include <tango-augmented-reality/tango_event_data.h>


namespace tango_augmented_reality {

// AugmentedRealityApp handles the application lifecycle and resources.
class AugmentedRealityApp {
 public:
  // Constructor and deconstructor.
  AugmentedRealityApp();
  ~AugmentedRealityApp();

  // Initialize Tango Service, this function starts the communication
  // between the application and Tango Service.  This also records a
  // reference to the caller activity to request rendering when new
  // texture data is available.  The activity object is also used to
  // check if the API version is outdated.
  int TangoInitialize(JNIEnv* env, jobject caller_activity);

  // When the Android activity is destroyed signal the JNI layer to
  // remove references to the activity. This should be called from the
  // onDestroy() callback of the parent activity lifecycle.
  void ActivityDestroyed();

  // Setup the configuration file for the Tango Service. We'll also see whether
  // we'd like auto-recover enabled.
  int TangoSetupConfig();

  // Connect the onPoseAvailable callback.
  int TangoConnectCallbacks();

  // Connect to Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start Motion Tracking.
  int TangoConnect();

  // Disconnect from Tango Service, release all the resources that the app is
  // holding from Tango Service.
  void TangoDisconnect();

  // Explicitly reset motion tracking and restart the pipeline.
  // Note that this will cause motion tracking to re-initialize.
  void TangoResetMotionTracking();

  // Tango service event callback function for pose data. Called when new events
  // are available from the Tango Service.
  //
  // @param event: Tango event, caller allocated.
  void onTangoEventAvailable(const TangoEvent* event);

  // Tango service texture callback. Called when the texture is updated.
  //
  // @param id: camera Id of the updated camera.
  void onTextureAvailable(TangoCameraId id);

  // Allocate OpenGL resources for rendering, mainly initializing the Scene.
  void InitializeGLContent();

  // Setup the view port width and height.
  void SetViewPort(int width, int height);

  // Main render loop.
  void Render();

  // Release all non-OpenGL resources that allocate from the program.
  void DeleteResources();

  // Retrun pose debug string.
  std::string GetPoseString();

  // Retrun Tango event debug string.
  std::string GetEventString();

  // Retrun Tango Service version string.
  std::string GetVersionString();

  // Set render camera's viewing angle, first person, third person or top down.
  //
  // @param: camera_type, camera type includes first person, third person and
  //         top down
  void SetCameraType(tango_gl::GestureCamera::CameraType camera_type);

  // Touch event passed from android activity. This function only supports two
  // touches.
  //
  // @param: touch_count, total count for touches.
  // @param: event, touch event of current touch.
  // @param: x0, normalized touch location for touch 0 on x axis.
  // @param: y0, normalized touch location for touch 0 on y axis.
  // @param: x1, normalized touch location for touch 1 on x axis.
  // @param: y1, normalized touch location for touch 1 on y axis.
  void OnTouchEvent(int touch_count, tango_gl::GestureCamera::TouchEvent event,
                    float x0, float y0, float x1, float y1);

  // Cache the Java VM
  //
  // @JavaVM java_vm: the Java VM is using from the Java layer.
  void SetJavaVM(JavaVM* java_vm) { java_vm_ = java_vm; }

 private:
  // Get a pose in matrix format with extrinsics in OpenGl space.
  //
  // @param: timstamp, timestamp of the target pose.
  //
  // @return: pose in matrix format.
  glm::mat4 GetPoseMatrixAtTimestamp(double timstamp);

  // Query sensor/camera extrinsic from the Tango Service, the extrinsic is only
  // available after the service is connected.
  //
  // @return: error code.
  TangoErrorType UpdateExtrinsics();

  // Request the render function from Java layer.
  void RequestRender();

  // pose_data_ handles all pose onPoseAvailable callbacks, onPoseAvailable()
  // in this object will be routed to pose_data_ to handle.
  PoseData pose_data_;

  // Mutex for protecting the pose data. The pose data is shared between render
  // thread and TangoService callback thread.
  std::mutex pose_mutex_;

  // tango_event_data_ handles all Tango event callbacks,
  // onTangoEventAvailable() in this object will be routed to tango_event_data_
  // to handle.
  TangoEventData tango_event_data_;

  // tango_event_data_ is share between the UI thread we start for updating
  // debug
  // texts and the TangoService event callback thread. We keep event_mutex_ to
  // protect tango_event_data_.
  std::mutex tango_event_mutex_;

  // main_scene_ includes all drawable object for visualizing Tango device's
  // movement.
  Scene main_scene_;

  // Tango configration file, this object is for configuring Tango Service setup
  // before connect to service. For example, we set the flag
  // config_enable_auto_recovery based user's input and then start Tango.
  TangoConfig tango_config_;

  // Device color camera intrinsics, these intrinsics value is used for
  // calculate the camera frustum and image aspect ratio. In the AR view, we
  // want to match the virtual camera's intrinsics to the actual physical camera
  // as close as possible.
  TangoCameraIntrinsics color_camera_intrinsics_;

  // Tango service version string.
  std::string tango_core_version_string_;

  // Cached Java VM, caller activity object and the request render method. These
  // variables are used for on demand render request from the onTextureAvailable
  // callback.
  JavaVM* java_vm_;
  jobject calling_activity_obj_;
  jmethodID on_demand_render_;
};
}  // namespace tango_augmented_reality

#endif  // TANGO_AUGMENTED_REALITY_AUGMENTED_REALITY_APP_H_
