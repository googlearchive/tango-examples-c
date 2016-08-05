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

#ifndef HELLO_VIDEO_HELLO_VIDEO_APP_H_
#define HELLO_VIDEO_HELLO_VIDEO_APP_H_

#include <atomic>
#include <jni.h>
#include <memory>
#include <mutex>
#include <vector>

#include <tango_client_api.h>  // NOLINT
#include <tango-gl/util.h>
#include <tango-gl/video_overlay.h>

namespace hello_video {

// HelloVideoApp handles the application lifecycle and resources.
class HelloVideoApp {
 public:
  enum TextureMethod {
    kYuv,
    kTextureId
  };

  // OnCreate() callback is called when this Android application's
  // OnCreate function is called from UI thread. In the OnCreate
  // function, we are only checking the Tango Core's version.
  //
  // @param env, java environment parameter OnCreate is being called.
  // @param caller_activity, caller of this function.
  // @param env, orienation param for the activity.
  // @param caller_activity, orientation param for the color camera sensor.
  void OnCreate(JNIEnv* env, jobject caller_activity, int activity_rotation,
                int sensor_rotation);

  // Called when the Tango service is connect. We set the binder object to Tango
  // Service in this function.
  //
  // @param env, java environment parameter.
  // @param binder, the native binder object.
  void OnTangoServiceConnected(JNIEnv* env, jobject binder);

  // OnPause() callback is called when this Android application's
  // OnCreate function is called from UI thread. In our application,
  // we disconnect Tango Service and free the Tango configuration
  // file. It is important to disconnect Tango Service and release
  // the coresponding resources in the OnPause() callback from
  // Android, otherwise, this application will hold on to the Tango
  // resources and other application will not be able to connect to
  // Tango Service.
  void OnPause();

  // Initializing the Scene.
  void OnSurfaceCreated();

  // Setup the view port width and height.
  void OnSurfaceChanged(int width, int height);

  // Main render loop.
  void OnDrawFrame();

  // Set texture method.
  void SetTextureMethod(TextureMethod method) {
    current_texture_method_ = method;
  }

  // YUV data callback.
  void OnFrameAvailable(const TangoImageBuffer* buffer);

 private:
  // Tango configration file, this object is for configuring Tango Service setup
  // before connect to service. For example, we set the flag
  // config_enable_auto_recovery based user's input and then start Tango.
  TangoConfig tango_config_;

  // video_overlay_ Render the camera video feedback onto the screen.
  tango_gl::VideoOverlay* video_overlay_drawable_;
  tango_gl::VideoOverlay* yuv_drawable_;

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

  bool is_service_connected_;
  bool is_texture_id_set_;

  int activity_rotation_;
  int sensor_rotation_;

  void AllocateTexture(GLuint texture_id, int width, int height);
  void RenderYuv();
  void RenderTextureId();
  void DeleteDrawables();
};
}  // namespace hello_video

#endif  // HELLO_VIDEO_HELLO_VIDEO_APP_H_
