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

#ifndef TANGO_VIDEO_OVERLAY_VIDEO_OVERLAY_APP_H_
#define TANGO_VIDEO_OVERLAY_VIDEO_OVERLAY_APP_H_

#include <atomic>
#include <jni.h>
#include <memory>
#include <mutex>

#include <tango_client_api.h>  // NOLINT
#include <tango-gl/util.h>
#include <tango-video-overlay/yuv_drawable.h>
#include <tango-gl/video_overlay.h>

namespace tango_video_overlay {

// VideoOverlayApp handles the application lifecycle and resources.
class VideoOverlayApp {
 public:
  // Constructor and deconstructor.
  VideoOverlayApp();
  ~VideoOverlayApp();

  enum TextureMethod {
    kYUV,
    kTextureId
  };

  // YUV data callback.
  void OnFrameAvailable(const TangoImageBuffer* buffer);

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

  // Initializing the Scene.
  void InitializeGLContent();

  // Setup the view port width and height.
  void SetViewPort(int width, int height);

  // Main render loop.
  void Render();

  // Release all OpenGL resources that allocate from the program.
  void FreeBufferData();

  // Set texture method.
  void SetTextureMethod(int method) {
    current_texture_method_ = static_cast<TextureMethod>(method);
  }

 private:
  // Tango configration file, this object is for configuring Tango Service setup
  // before connect to service. For example, we set the flag
  // config_enable_auto_recovery based user's input and then start Tango.
  TangoConfig tango_config_;

  // video_overlay_ render the camera video feedback onto the screen.
  tango_gl::VideoOverlay* video_overlay_drawable_;
  YUVDrawable* yuv_drawable_;

  TextureMethod current_texture_method_;

  std::vector<uint8_t> yuv_buffer_;
  std::vector<uint8_t> yuv_temp_buffer_;
  std::vector<GLubyte> rgb_buffer_;

  std::atomic<bool> is_yuv_texture_available_;
  std::atomic<bool> swap_buffer_signal_;
  std::mutex yuv_buffer_mutex_;

  size_t yuv_width_;
  size_t yuv_height_;
  size_t yuv_size_;
  size_t uv_buffer_offset_;

  void AllocateTexture(GLuint texture_id, int width, int height);
  void RenderYUV();
  void RenderTextureId();
  void DeleteDrawables();
};
}  // namespace tango_video_overlay

#endif  // TANGO_VIDEO_OVERLAY_VIDEO_OVERLAY_APP_H_
