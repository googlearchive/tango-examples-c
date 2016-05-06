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

#ifndef TANGO_MOTION_TRACKING_MOTION_TRACKING_APP_H_
#define TANGO_MOTION_TRACKING_MOTION_TRACKING_APP_H_

#include <jni.h>
#include <memory>
#include <mutex>
#include <string>

#include <tango_client_api.h>  // NOLINT
#include <tango-gl/util.h>

#include <tango-motion-tracking/scene.h>

namespace tango_motion_tracking {

// MotiongTrackingApp handles the application lifecycle and resources.
class MotiongTrackingApp {
 public:
  // Constructor and deconstructor.
  MotiongTrackingApp();
  ~MotiongTrackingApp();

  // Check that the installed version of the Tango API is up to date.
  //
  // @return returns true if the application version is compatible with the
  //         Tango Core version.
  bool CheckTangoVersion(JNIEnv* env, jobject activity, int min_tango_version);

  // Setup the configuration file for the Tango Service
  int TangoSetupConfig();

  // Connect to Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start Motion Tracking.
  bool TangoConnect();

  // Disconnect from Tango Service, release all the resources that the app is
  // holding from Tango Service.
  void TangoDisconnect();

  // Allocate OpenGL resources for rendering, mainly initializing the Scene.
  void InitializeGLContent();

  // Setup the view port width and height.
  void SetViewPort(int width, int height);

  // Main render loop.
  void Render();

  // Release all resources that allocate from the program.
  void DeleteResources();

  // Set screen rotation index.
  //
  // @param screen_roatation: the screen rotation index,
  //    the index is following Android screen rotation enum.
  //    see Android documentation for detail:
  //    http://developer.android.com/reference/android/view/Surface.html#ROTATION_0
  void SetScreenRotation(int screen_roatation);

  // Call when Tango Service is connected successfully.
  void OnTangoServiceConnected(JNIEnv* env, jobject iBinder);

 private:
  // main_scene_ includes all drawable object for visualizing Tango device's
  // movement.
  Scene main_scene_;

  // Tango configration file, this object is for configuring Tango Service setup
  // before connect to service. For example, we set the flag
  // config_enable_auto_recovery based user's input and then start Tango.
  TangoConfig tango_config_;

  // Screen rotation index.
  int screen_rotation_;
};
}  // namespace tango_motion_tracking

#endif  // TANGO_MOTION_TRACKING_MOTION_TRACKING_APP_H_
