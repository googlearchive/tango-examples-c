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

#include <atomic>
#include <jni.h>
#include <memory>
#include <string>

#include <tango_client_api.h>  // NOLINT
#include <tango-gl/util.h>

#include <tango-augmented-reality/scene.h>
#include <tango-augmented-reality/tango_event_data.h>

namespace tango_augmented_reality {

// AugmentedRealityApp handles the application lifecycle and resources.
class AugmentedRealityApp {
 public:
  // OnCreate() callback is called when this Android application's
  // OnCreate function is called from UI thread. In the OnCreate
  // function, we are only checking the Tango Core's version.
  //
  // @param env, java environment parameter OnCreate is being called.
  // @param caller_activity, caller of this function.
  // @param display_rotation, orienation param for the current display.
  void OnCreate(JNIEnv* env, jobject caller_activity, int display_rotation);

  // OnPause() callback is called when this Android application's
  // OnCreate function is called from UI thread. In our application,
  // we disconnect Tango Service and free the Tango configuration
  // file. It is important to disconnect Tango Service and release
  // the coresponding resources in the OnPause() callback from
  // Android, otherwise, this application will hold on to the Tango
  // resources and other application will not be able to connect to
  // Tango Service.
  void OnPause();

  // Call when Tango Service is connected successfully.
  void OnTangoServiceConnected(JNIEnv* env, jobject iBinder);

  // When the Android activity is destroyed signal the JNI layer to
  // remove references to the activity. This should be called from the
  // onDestroy() callback of the parent activity lifecycle.
  void OnDestroy();

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
  void OnSurfaceCreated(AAssetManager* aasset_manager);

  // Setup the view port width and height.
  void OnSurfaceChanged(int width, int height);

  // Main render loop.
  void OnDrawFrame();

  // Return transform debug string.
  std::string GetTransformString();

  // Retrun Tango event debug string.
  std::string GetEventString();

  // Retrun Tango Service version string.
  std::string GetVersionString();

  // Cache the Java VM
  //
  // @JavaVM java_vm: the Java VM is using from the Java layer.
  void SetJavaVM(JavaVM* java_vm) { java_vm_ = java_vm; }

  // Called when the device orientation changed
  //
  // @JavaVM display_rotation: orientation of current display.
  void OnDeviceRotationChanged(int display_rotation);

 private:
  // Request the render function from Java layer.
  void RequestRender();

  // Update current transform and previous transform.
  //
  // @param transform: transform data of current frame.
  // @param timestamp: timestamp of the current transform.
  void UpdateTransform(const double transform[16], double timestamp);

  // Format debug string with current and last transforms information.
  void FormatTransformString();

  void UpdateViewportAndProjectionMatrix();

  // Setup the configuration file for the Tango Service. We'll also see whether
  // we'd like auto-recover enabled.
  void TangoSetupConfig();

  // Connect the OnTextureAvailable and OnTangoEvent callbacks.
  void TangoConnectCallbacks();

  // Connect to Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start Motion Tracking.
  void TangoConnect();

  // Disconnect from Tango Service, release all the resources that the app is
  // holding from Tango Service.
  void TangoDisconnect();

  // Release all non-OpenGL resources that allocate from the program.
  void DeleteResources();

  // Current position of the Color Camera with respect to Start of Service.
  glm::mat4 cur_start_service_T_camera_;
  // prev_start_service_T_camera_, transform_counter_ and transform_string_ are
  // used for
  // composing the debug string to display the useful information on screen.
  glm::mat4 prev_start_service_T_camera_;

  // Debug transform string.
  std::string transform_string_;

  // Timestamps of the current and last transforms.
  double cur_timestamp_;
  double prev_timestamp_;

  // Pose counter for debug purpose.
  size_t transform_counter_;

  // Mutex for protecting the transform data. The transform data is shared
  // between render thread and TangoService callback thread.
  std::mutex transform_mutex_;

  // tango_event_data_ handles all Tango event callbacks,
  // onTangoEventAvailable() in this object will be routed to tango_event_data_
  // to handle.
  TangoEventData tango_event_data_;

  // tango_event_data_ is share between the UI thread we start for updating
  // debug texts and the TangoService event callback thread. We keep
  // event_mutex_ to
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

  std::atomic<bool> is_service_connected_;
  std::atomic<bool> is_gl_initialized_;
  std::atomic<bool> is_video_overlay_rotation_set_;

  int viewport_width_;
  int viewport_height_;

  int display_rotation_;
};
}  // namespace tango_augmented_reality

#endif  // TANGO_AUGMENTED_REALITY_AUGMENTED_REALITY_APP_H_
