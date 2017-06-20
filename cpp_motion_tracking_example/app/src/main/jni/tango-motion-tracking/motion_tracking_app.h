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

#ifndef CPP_MOTION_TRACKING_EXAMPLE_TANGO_MOTION_TRACKING_MOTION_TRACKING_APP_H_
#define CPP_MOTION_TRACKING_EXAMPLE_TANGO_MOTION_TRACKING_MOTION_TRACKING_APP_H_

#include <android/asset_manager.h>
#include <jni.h>
#include <memory>
#include <mutex>
#include <string>

#include <tango_client_api.h>  // NOLINT
#include <tango-gl/util.h>

#include <tango-motion-tracking/scene.h>

namespace tango_motion_tracking {

// MotionTrackingApp handles the application lifecycle and resources.
class MotionTrackingApp {
 public:
  // Constructor and deconstructor.
  MotionTrackingApp();
  ~MotionTrackingApp();

  // OnCreate() callback is called when this Android application's
  // OnCreate function is called from UI thread. In the OnCreate
  // function, we are only checking the Tango Core's version.
  //
  // @param env, java environment parameter OnCreate is being called.
  // @param caller_activity, caller of this function.
  void OnCreate(JNIEnv* env, jobject caller_activity);

  // OnPause() callback is called when this Android application's
  // OnPause function is called from UI thread. In our application,
  // we disconnect Tango Service and free the Tango configuration
  // file. It is important to disconnect Tango Service and release
  // the corresponding resources in the OnPause() callback from
  // Android, otherwise, this application will hold on to the Tango
  // resources and other applications will not be able to connect to
  // Tango Service.
  void OnPause();

  // Call when Tango Service is connected successfully.
  void OnTangoServiceConnected(JNIEnv* env, jobject iBinder);

  // Allocate OpenGL resources for rendering, mainly initializing the Scene.
  void OnSurfaceCreated(AAssetManager* aasset_manager);

  // Set up the viewport width and height.
  void OnSurfaceChanged(int width, int height);

  // Main render loop.
  void OnDrawFrame();

  // Set screen rotation index.
  //
  // @param screen_roatation: the screen rotation index,
  //    the index is following Android screen rotation enum.
  //    see Android documentation for detail:
  //    http://developer.android.com/reference/android/view/Surface.html#TANGO_SUPPORT_ROTATION_0
  void SetScreenRotation(int screen_rotation);

 private:
  // Setup the configuration file for the Tango Service
  void TangoSetupConfig();

  // Connect to Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start Motion Tracking.
  void TangoConnect();

  // Disconnect from Tango Service, release all the resources that the app is
  // holding from Tango Service.
  void TangoDisconnect();

  // Release all resources that allocate from the program.
  void DeleteResources();

  // main_scene_ includes all drawable object for visualizing Tango device's
  // movement.
  Scene main_scene_;

  // Tango configration file, this object is for configuring Tango Service setup
  // before connect to service. For example, we set the flag
  // config_enable_auto_recovery based user's input and then start Tango.
  TangoConfig tango_config_;

  // Screen rotation index.
  int screen_rotation_;

  // Flag indicating when the Tango service can be used.
  bool is_service_connected_;
};
}  // namespace tango_motion_tracking

#endif  // CPP_MOTION_TRACKING_EXAMPLE_TANGO_MOTION_TRACKING_MOTION_TRACKING_APP_H_
