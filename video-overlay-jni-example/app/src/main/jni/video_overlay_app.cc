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

namespace {
void OnFrameAvailableRouter(void* context, TangoCameraId,
                            const TangoImageBuffer* buffer) {
  using namespace tango_video_overlay;
  VideoOverlayApp* app = static_cast<VideoOverlayApp*>(context);
  app->OnFrameAvailable(buffer);
}

// We could do this conversion in a fragment shader if all we care about is
// rendering, but we show it here as an example of how people can use RGB data
// on the CPU.
inline void Yuv2Rgb(uint8_t yValue, uint8_t uValue, uint8_t vValue, uint8_t* r,
                    uint8_t* g, uint8_t* b) {
  *r = yValue + (1.370705 * (vValue - 128));
  *g = yValue - (0.698001 * (vValue - 128)) - (0.337633 * (uValue - 128));
  *b = yValue + (1.732446 * (uValue - 128));
}
}

namespace tango_video_overlay {

VideoOverlayApp::VideoOverlayApp() {
  is_yuv_texture_available_ = false;
  swap_buffer_signal_ = false;
  video_overlay_drawable_ = NULL;
  yuv_drawable_ = NULL;
}

VideoOverlayApp::~VideoOverlayApp() {
  if (tango_config_ != nullptr) {
    TangoConfig_free(tango_config_);
  }
}

void VideoOverlayApp::OnFrameAvailable(const TangoImageBuffer* buffer) {
  if (current_texture_method_ != TextureMethod::kYUV) {
    return;
  }

  if (yuv_drawable_->GetTextureId() == 0) {
    LOGE("VideoOverlayApp::yuv texture id not valid");
    return;
  }

  if (buffer->format != TANGO_HAL_PIXEL_FORMAT_YCrCb_420_SP) {
    LOGE("VideoOverlayApp::yuv texture format is not supported by this app");
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

  ret = TangoService_connectOnFrameAvailable(TANGO_CAMERA_COLOR, this,
                                             OnFrameAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE("VideoOverlayApp: Error connecting color frame %d", ret);
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

void VideoOverlayApp::DeleteDrawables() {
  delete video_overlay_drawable_;
  delete yuv_drawable_;
  video_overlay_drawable_ = NULL;
  yuv_drawable_ = NULL;
}

void VideoOverlayApp::InitializeGLContent() {
  if (video_overlay_drawable_ != NULL || yuv_drawable_ != NULL) {
    this->DeleteDrawables();
  }
  video_overlay_drawable_ = new tango_gl::VideoOverlay();
  yuv_drawable_ = new YUVDrawable();

  // Connect color camera texture. TangoService_connectTextureId expects a valid
  // texture id from the caller, so we will need to wait until the GL content is
  // properly allocated.
  int texture_id = static_cast<int>(video_overlay_drawable_->GetTextureId());
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
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  switch (current_texture_method_) {
    case TextureMethod::kYUV:
      RenderYUV();
      break;
    case TextureMethod::kTextureId:
      RenderTextureId();
      break;
  }
}

void VideoOverlayApp::FreeBufferData() {
  is_yuv_texture_available_ = false;
  swap_buffer_signal_ = false;
  rgb_buffer_.clear();
  yuv_buffer_.clear();
  yuv_temp_buffer_.clear();
  this->DeleteDrawables();
}

void VideoOverlayApp::AllocateTexture(GLuint texture_id, int width,
                                      int height) {

  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, rgb_buffer_.data());
}

void VideoOverlayApp::RenderYUV() {
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

void VideoOverlayApp::RenderTextureId() {
  double timestamp;
  // TangoService_updateTexture() updates target camera's
  // texture and timestamp.
  int ret = TangoService_updateTexture(TANGO_CAMERA_COLOR, &timestamp);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "VideoOverlayApp: Failed to update the texture id with error code: "
        "%d",
        ret);
  }
  video_overlay_drawable_->Render(glm::mat4(1.0f), glm::mat4(1.0f));
}

}  // namespace tango_video_overlay
