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

#include <tango_support.h>

#include "hello_video/hello_video_app.h"

namespace {
constexpr int kTangoCoreMinimumVersion = 9377;
void OnFrameAvailableRouter(void* context, TangoCameraId,
                            const TangoImageBuffer* buffer) {
  hello_video::HelloVideoApp* app =
      static_cast<hello_video::HelloVideoApp*>(context);
  app->OnFrameAvailable(buffer);
}

// We could do this conversion in a fragment shader if all we care about is
// rendering, but we show it here as an example of how people can use RGB data
// on the CPU.
inline void Yuv2Rgb(uint8_t y_value, uint8_t u_value, uint8_t v_value,
                    uint8_t* r, uint8_t* g, uint8_t* b) {
  float float_r = y_value + (1.370705 * (v_value - 128));
  float float_g =
      y_value - (0.698001 * (v_value - 128)) - (0.337633 * (u_value - 128));
  float float_b = y_value + (1.732446 * (u_value - 128));

  float_r = float_r * !(float_r < 0);
  float_g = float_g * !(float_g < 0);
  float_b = float_b * !(float_b < 0);

  *r = float_r * (!(float_r > 255)) + 255 * (float_r > 255);
  *g = float_g * (!(float_g > 255)) + 255 * (float_g > 255);
  *b = float_b * (!(float_b > 255)) + 255 * (float_b > 255);
}
}  // namespace

namespace hello_video {
void HelloVideoApp::OnCreate(JNIEnv* env, jobject caller_activity) {
  // Check the installed version of the TangoCore.  If it is too old, then
  // it will not support the most up to date features.
  int version = 0;
  TangoErrorType err =
      TangoSupport_getTangoVersion(env, caller_activity, &version);
  if (err != TANGO_SUCCESS || version < kTangoCoreMinimumVersion) {
    LOGE("HelloVideoApp::OnCreate, Tango Core version is out of date.");
    std::exit(EXIT_SUCCESS);
  }

  // Initialize variables
  is_yuv_texture_available_ = false;
  swap_buffer_signal_ = false;
  is_service_connected_ = false;
  is_texture_id_set_ = false;
  video_overlay_drawable_ = NULL;
  yuv_drawable_ = NULL;
  is_video_overlay_rotation_set_ = false;
}

void HelloVideoApp::OnTangoServiceConnected(JNIEnv* env, jobject binder) {
  if (TangoService_setBinder(env, binder) != TANGO_SUCCESS) {
    LOGE(
        "HelloVideoApp::OnTangoServiceConnected, TangoService_setBinder error");
    std::exit(EXIT_SUCCESS);
  }

  // Here, we'll configure the service to run in the way we'd want. For this
  // application, we'll start from the default configuration
  // (TANGO_CONFIG_DEFAULT). This enables basic motion tracking capabilities.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE(
        "HelloVideoApp::OnTangoServiceConnected,"
        "Failed to get default config form");
    std::exit(EXIT_SUCCESS);
  }

  // Enable color camera from config.
  int ret =
      TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "HelloVideoApp::OnTangoServiceConnected,"
        "config_enable_color_camera() failed with error code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  ret = TangoService_connectOnFrameAvailable(TANGO_CAMERA_COLOR, this,
                                             OnFrameAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "HelloVideoApp::OnTangoServiceConnected,"
        "Error connecting color frame %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  // Connect to Tango Service, service will start running, and
  // pose can be queried.
  ret = TangoService_connect(this, tango_config_);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "HelloVideoApp::OnTangoServiceConnected,"
        "Failed to connect to the Tango service with error code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  // Initialize TangoSupport context.
  TangoSupport_initialize(TangoService_getPoseAtTime,
                          TangoService_getCameraIntrinsics);

  is_service_connected_ = true;
}

void HelloVideoApp::OnPause() {
  // Free TangoConfig structure
  if (tango_config_ != nullptr) {
    TangoConfig_free(tango_config_);
    tango_config_ = nullptr;
  }

  // Disconnect from the Tango service
  TangoService_disconnect();

  // Free buffer data
  is_yuv_texture_available_ = false;
  swap_buffer_signal_ = false;
  is_service_connected_ = false;
  is_video_overlay_rotation_set_ = false;
  is_texture_id_set_ = false;
  rgb_buffer_.clear();
  yuv_buffer_.clear();
  yuv_temp_buffer_.clear();
  this->DeleteDrawables();
}

void HelloVideoApp::OnFrameAvailable(const TangoImageBuffer* buffer) {
  if (current_texture_method_ != TextureMethod::kYuv) {
    return;
  }

  if (yuv_drawable_->GetTextureId() == 0) {
    LOGE("HelloVideoApp::yuv texture id not valid");
    return;
  }

  if (buffer->format != TANGO_HAL_PIXEL_FORMAT_YCrCb_420_SP) {
    LOGE("HelloVideoApp::yuv texture format is not supported by this app");
    return;
  }

  // The memory needs to be allocated after we get the first frame because we
  // need to know the size of the image.
  if (!is_yuv_texture_available_) {
    yuv_width_ = buffer->width;
    yuv_height_ = buffer->height;
    uv_buffer_offset_ = yuv_width_ * yuv_height_;

    yuv_size_ = yuv_width_ * yuv_height_ + yuv_width_ * yuv_height_ / 2;

    // Reserve and resize the buffer size for RGB and YUV data.
    yuv_buffer_.resize(yuv_size_);
    yuv_temp_buffer_.resize(yuv_size_);
    rgb_buffer_.resize(yuv_width_ * yuv_height_ * 3);

    AllocateTexture(yuv_drawable_->GetTextureId(), yuv_width_, yuv_height_);
    is_yuv_texture_available_ = true;
  }

  std::lock_guard<std::mutex> lock(yuv_buffer_mutex_);
  memcpy(&yuv_temp_buffer_[0], buffer->data, yuv_size_);
  swap_buffer_signal_ = true;
}

void HelloVideoApp::DeleteDrawables() {
  delete video_overlay_drawable_;
  delete yuv_drawable_;
  video_overlay_drawable_ = NULL;
  yuv_drawable_ = NULL;
}

void HelloVideoApp::OnSurfaceCreated() {
  if (video_overlay_drawable_ != NULL || yuv_drawable_ != NULL) {
    this->DeleteDrawables();
  }

  video_overlay_drawable_ =
      new tango_gl::VideoOverlay(GL_TEXTURE_EXTERNAL_OES, display_rotation_);
  yuv_drawable_ = new tango_gl::VideoOverlay(GL_TEXTURE_2D, display_rotation_);
}

void HelloVideoApp::OnSurfaceChanged(int width, int height) {
  glViewport(0, 0, width, height);
}

void HelloVideoApp::OnDrawFrame() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  if (!is_service_connected_) {
    return;
  }

  if (!is_texture_id_set_) {
    is_texture_id_set_ = true;
    // Connect color camera texture. TangoService_connectTextureId expects a
    // valid texture id from the caller, so we will need to wait until the GL
    // content is properly allocated.
    int texture_id = static_cast<int>(video_overlay_drawable_->GetTextureId());
    TangoErrorType ret = TangoService_connectTextureId(
        TANGO_CAMERA_COLOR, texture_id, nullptr, nullptr);
    if (ret != TANGO_SUCCESS) {
      LOGE(
          "HelloVideoApp: Failed to connect the texture id with error"
          "code: %d",
          ret);
    }
  }

  if (!is_video_overlay_rotation_set_) {
    video_overlay_drawable_->SetDisplayRotation(display_rotation_);
    yuv_drawable_->SetDisplayRotation(display_rotation_);
    is_video_overlay_rotation_set_ = true;
  }

  switch (current_texture_method_) {
    case TextureMethod::kYuv:
      RenderYuv();
      break;
    case TextureMethod::kTextureId:
      RenderTextureId();
      break;
  }
}

void HelloVideoApp::AllocateTexture(GLuint texture_id, int width, int height) {
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, rgb_buffer_.data());
}

void HelloVideoApp::RenderYuv() {
  if (!is_yuv_texture_available_) {
    return;
  }
  {
    std::lock_guard<std::mutex> lock(yuv_buffer_mutex_);
    if (swap_buffer_signal_) {
      std::swap(yuv_buffer_, yuv_temp_buffer_);
      swap_buffer_signal_ = false;
    }
  }

  for (size_t i = 0; i < yuv_height_; ++i) {
    for (size_t j = 0; j < yuv_width_; ++j) {
      size_t x_index = j;
      if (j % 2 != 0) {
        x_index = j - 1;
      }

      size_t rgb_index = (i * yuv_width_ + j) * 3;

      // The YUV texture format is NV21,
      // yuv_buffer_ buffer layout:
      //   [y0, y1, y2, ..., yn, v0, u0, v1, u1, ..., v(n/4), u(n/4)]
      Yuv2Rgb(
          yuv_buffer_[i * yuv_width_ + j],
          yuv_buffer_[uv_buffer_offset_ + (i / 2) * yuv_width_ + x_index + 1],
          yuv_buffer_[uv_buffer_offset_ + (i / 2) * yuv_width_ + x_index],
          &rgb_buffer_[rgb_index], &rgb_buffer_[rgb_index + 1],
          &rgb_buffer_[rgb_index + 2]);
    }
  }

  glBindTexture(GL_TEXTURE_2D, yuv_drawable_->GetTextureId());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, yuv_width_, yuv_height_, 0, GL_RGB,
               GL_UNSIGNED_BYTE, rgb_buffer_.data());

  yuv_drawable_->Render(glm::mat4(1.0f), glm::mat4(1.0f));
}

void HelloVideoApp::RenderTextureId() {
  double timestamp;
  // TangoService_updateTexture() updates target camera's
  // texture and timestamp.
  int ret = TangoService_updateTexture(TANGO_CAMERA_COLOR, &timestamp);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "HelloVideoApp: Failed to update the texture id with error code: "
        "%d",
        ret);
  }
  video_overlay_drawable_->Render(glm::mat4(1.0f), glm::mat4(1.0f));
}

void HelloVideoApp::OnDisplayChanged(int display_rotation) {
  display_rotation_ = static_cast<TangoSupport_Rotation>(display_rotation);
  is_video_overlay_rotation_set_ = false;
}

}  // namespace hello_video
