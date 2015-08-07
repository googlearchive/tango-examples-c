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

#include "tango-video-overlay/video_overlay_app.h"

namespace tango_video_overlay {

VideoOverlayApp::VideoOverlayApp() {}

VideoOverlayApp::~VideoOverlayApp() {
  if (tango_config_ != nullptr) {
    TangoConfig_free(tango_config_);
  }
}

int VideoOverlayApp::TangoInitialize(JNIEnv* env, jobject caller_activity) {
  // The first thing we need to do for any Tango enabled application is to
  // initialize the service. We'll do that here, passing on the JNI environment
  // and jobject corresponding to the Android activity that is calling us.
  return TangoService_initialize(env, caller_activity);
}

int VideoOverlayApp::TangoSetupConfig() {
  // Here, we'll configure the service to run in the way we'd want. For this
  // application, we'll start from the default configuration
  // (TANGO_CONFIG_DEFAULT). This enables basic motion tracking capabilities.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE("VideoOverlayApp: Failed to get default config form");
    return TANGO_ERROR;
  }

  // Enable color camera from config.
  int ret =
      TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "VideoOverlayApp: config_enable_color_camera() failed with error"
        "code: %d",
        ret);
    return ret;
  }
  return ret;
}

// Connect to Tango Service, service will start running, and
// pose can be queried.
int VideoOverlayApp::TangoConnect() {
  TangoErrorType ret = TangoService_connect(this, tango_config_);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "VideoOverlayApp: Failed to connect to the Tango service with"
        "error code: %d",
        ret);
    return ret;
  }
  return ret;
}

void VideoOverlayApp::TangoDisconnect() {
  // When disconnecting from the Tango Service, it is important to make sure to
  // free your configuration object. Note that disconnecting from the service,
  // resets all configuration, and disconnects all callbacks. If an application
  // resumes after disconnecting, it must re-register configuration and
  // callbacks with the service.
  TangoConfig_free(tango_config_);
  tango_config_ = nullptr;
  TangoService_disconnect();
}

void VideoOverlayApp::InitializeGLContent() {
  video_overlay_ = new tango_gl::VideoOverlay();
  // Connect color camera texture. TangoService_connectTextureId expects a valid
  // texture id from the caller, so we will need to wait until the GL content is
  // properly allocated.
  int texture_id = static_cast<int>(video_overlay_->GetTextureId());
  TangoErrorType ret = TangoService_connectTextureId(
      TANGO_CAMERA_COLOR, texture_id, nullptr, nullptr);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "VideoOverlayApp: Failed to connect the texture id with error"
        "code: %d",
        ret);
  }
}

void VideoOverlayApp::SetViewPort(int width, int height) {
  glViewport(0, 0, width, height);
}

void VideoOverlayApp::Render() {
  double timestamp;
  // TangoService_updateTexture() updates target camera's
  // texture and timestamp.
  int ret = TangoService_updateTexture(TANGO_CAMERA_COLOR, &timestamp);
  if (ret != TANGO_SUCCESS) {
    LOGE("VideoOverlayApp: Failed to update the texture id with error code: %d",
         ret);
  }
  video_overlay_->Render(glm::mat4(1.0f), glm::mat4(1.0f));
}

void VideoOverlayApp::FreeGLContent() { delete video_overlay_; }

}  // namespace tango_video_overlay
