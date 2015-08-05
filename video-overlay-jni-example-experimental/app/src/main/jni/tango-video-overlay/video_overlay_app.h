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
#include <tango-gl/video_overlay.h>

namespace tango_video_overlay {

// VideoOverlayApp handles the application lifecycle and resources.
class VideoOverlayApp {
 public:
  // Constructor and deconstructor.
  VideoOverlayApp();
  ~VideoOverlayApp();

  // Initialize Tango Service, this function starts the communication
  // between the application and Tango Service.
  // The activity object is used for checking if the API version is outdated.
  int TangoInitialize(JNIEnv* env, jobject caller_activity);

  // Setup the configuration file for the Tango Service. We'll also se whether
  // we'd like auto-recover enabled.
  int TangoSetupConfig();

  // Connect to the Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start the video overlay update.
  int TangoConnect();

  // Disconnect from Tango Service, release all the resources that the app is
  // holding from Tango Service.
  void TangoDisconnect();

  // Allocate OpenGL resources for rendering, mainly initializing the Scene.
  void InitializeGLContent();

  // Setup the view port width and height.
  void SetViewPort(int width, int height);

  // Main render loop.
  void Render();

  // Release all OpenGL resources that allocate from the program.
  void FreeGLContent();

 private:
  // Tango configration file, this object is for configuring Tango Service setup
  // before connect to service. For example, we set the flag
  // config_enable_auto_recovery based user's input and then start Tango.
  TangoConfig tango_config_;

  // video_overlay_ render the camera video feedback onto the screen.
  tango_gl::VideoOverlay* video_overlay_;
};
}  // namespace tango_video_overlay

#endif  // TANGO_AUGMENTED_REALITY_AUGMENTED_REALITY_APP_H_
