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

#ifndef CPP_VIDEO_STABILIZATION_EXPERIMENT_TANGO_VIDEO_STABILIZATION_VIDEO_STABILIZATION_APP_H_
#define CPP_VIDEO_STABILIZATION_EXPERIMENT_TANGO_VIDEO_STABILIZATION_VIDEO_STABILIZATION_APP_H_

#include <jni.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#include <tango_client_api.h>  // NOLINT
#include <tango-gl/util.h>
#include <tango_support.h>

#include <tango-video-stabilization/scene.h>

namespace tango_video_stabilization {

// VideoStabilizationApp handles the application lifecycle and resources.
class VideoStabilizationApp {
 public:
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

  // Call when Tango Service is connected successfully.
  bool OnTangoServiceConnected(JNIEnv* env, jobject iBinder);

  // When the Android activity is destroyed signal the JNI layer to
  // remove references to the activity. This should be called from the
  // onDestroy() callback of the parent activity lifecycle.
  void ActivityDestroyed();

  // Setup the configuration file for the Tango Service. We'll also see whether
  // we'd like auto-recover enabled.
  int TangoSetupConfig();

  // Connect to Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start Motion Tracking.
  bool TangoConnect();

  // Disconnect from Tango Service, release all the resources that the app is
  // holding from Tango Service.
  void TangoDisconnect();

  // Tango service texture callback. Called when the texture is updated.
  //
  // @param id: camera Id of the updated camera.
  void OnTextureAvailable(TangoCameraId id);

  // Allocate OpenGL resources for rendering, mainly initializing the Scene.
  void InitializeGLContent();

  // Setup the view port width and height.
  void SetViewPort(int width, int height);

  // Main render loop.
  void Render();

  // Sets the control for video stabilization.
  void SetEnableVideoStabilization(bool enable_video_stabilization);

  // Locks or unlocks the camera.
  void SetCameraLocked(bool camera_locked);

  // Release all non-OpenGL resources that allocate from the program.
  void DeleteResources();

  // Cache the Java VM
  //
  // @JavaVM java_vm: the Java VM is using from the Java layer.
  void SetJavaVM(JavaVM* java_vm) { java_vm_ = java_vm; }

 private:
  // Request the render function from Java layer.
  void RequestRender();

  // Update current transform and previous transform.
  //
  // @param transform: transform data of current frame.
  void UpdatePose(const TangoPoseData& pose);

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

  // Connect the OnTextureAvailable and OnTangoEvent callbacks.
  void TangoConnectCallbacks();

  void UpdateViewportAndProjectionMatrix();

  // Cached Java VM, caller activity object and the request render method. These
  // variables are used for on demand render request from the onTextureAvailable
  // callback.
  JavaVM* java_vm_;
  jobject calling_activity_obj_;
  jmethodID on_demand_render_;

  double last_gpu_timestamp_ = 0.0;

  TangoSupport_PointCloudManager* point_cloud_manager_;
  TangoXYZij* front_cloud_;

  bool is_service_connected_;
  bool is_gl_initialized_;

  int viewport_width_;
  int viewport_height_;
};
}  // namespace tango_video_stabilization

#endif  // CPP_VIDEO_STABILIZATION_EXPERIMENT_TANGO_VIDEO_STABILIZATION_VIDEO_STABILIZATION_APP_H_
