/*
 * Copyright 2017 Google Inc. All Rights Reserved.
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

#ifndef CPP_MARKER_DETECTION_EXAMPLE_TANGO_MARKER_DETECTION_MARKER_DETECTION_APP_H_
#define CPP_MARKER_DETECTION_EXAMPLE_TANGO_MARKER_DETECTION_MARKER_DETECTION_APP_H_

#include <atomic>
#include <jni.h>
#include <memory>
#include <string>

#include <tango_client_api.h>
#include <tango-gl/util.h>

#include <tango-marker-detection/scene.h>

namespace tango_marker_detection {

// MarkerDetectionApp handles the application lifecycle and resources.
class MarkerDetectionApp {
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

  void OnFrameAvailable(const TangoImageBuffer* buffer);

  // Allocate OpenGL resources for rendering, mainly initializing the Scene.
  void OnSurfaceCreated(AAssetManager* aasset_manager);

  // Setup the view port width and height.
  void OnSurfaceChanged(int width, int height);

  // Main render loop.
  void OnDrawFrame();

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

  // Update OpenGL viewport and projection matrix.
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

  // Get color camera intrinsics.
  TangoErrorType SetupIntrinsics();

  // Setup image buffer manager.
  TangoErrorType SetupImageBufferManager();

  // Get the transformation matrix of the virtual AR camera
  // @param timestamp the timestamp.
  // @param display_rotation the Android display rotation.
  // @param camera_transformation_matrix the transformation from camera to
  //    world in OpenGL frame.
  TangoErrorType GetARCameraTransformationMatrix(
      double timestamp, TangoSupport_Rotation display_rotation,
      glm::mat4* ar_camera_transformation_matrix);

  // Get the extrinsics of the color camera.
  // @param timestamp the timestamp.
  // @param camera_extrinsics_matrix the transformation from camera to
  //    world in Tango frame.
  TangoErrorType GetColorCameraExtrinsics(double timestamp,
                                          glm::mat4* camera_extrinsics_matrix);

  // Detect marker in a secondary thread.
  // @param timestamp timestamp of the current video overlay image.
  void DetectMarkers(double timestamp);

  // A manger to keep track of the camera image.
  TangoSupport_ImageBufferManager* image_buffer_manager_;

  // main_scene_ includes all drawable objects.
  Scene main_scene_;

  // AR camera projection matrix.
  glm::mat4 ar_camera_projection_matrix_;

  // Tango configration file, this object is for configuring Tango Service setup
  // before connect to service. For example, we set the flag
  // config_enable_auto_recovery based user's input and then start Tango.
  TangoConfig tango_config_;

  // Device color camera intrinsics, these intrinsics value is used for
  // calculate the camera frustum and image aspect ratio. In the AR view, we
  // want to match the virtual camera's intrinsics to the actual physical camera
  // as close as possible.
  TangoCameraIntrinsics color_camera_intrinsics_;

  // Cached Java VM, caller activity object and the request render method. These
  // variables are used for on demand render request from the onTextureAvailable
  // callback.
  JavaVM* java_vm_;
  jobject calling_activity_obj_;
  jmethodID on_demand_render_;

  // Flags to indicate current status.
  std::atomic<bool> is_service_connected_;
  std::atomic<bool> is_gl_initialized_;
  std::atomic<bool> is_video_overlay_rotation_dirty_;

  // timestamp of last marker detection.
  double prev_marker_detection_timestamp_;

  // Mutex to prevent re-entry of marker detection thread.
  std::mutex marker_detection_thread_mutex_;

  // The width and height of the viewport.
  int viewport_width_;
  int viewport_height_;

  // The Android display rotation.
  TangoSupport_Rotation display_rotation_;
};
}  // namespace tango_marker_detection

#endif  // CPP_MARKER_DETECTION_EXAMPLE_TANGO_MARKER_DETECTION_MARKER_DETECTION_APP_H_
