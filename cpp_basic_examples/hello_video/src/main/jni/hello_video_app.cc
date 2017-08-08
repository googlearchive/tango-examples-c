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

void OnFrameAvailableRouter(void* context, TangoCameraId camera_id,
                            const TangoImageBuffer* buffer) {
  hello_video::HelloVideoApp* app =
      static_cast<hello_video::HelloVideoApp*>(context);
  app->OnFrameAvailable(camera_id, buffer);
}

void OnImageAvailableRouter(void* context, TangoCameraId camera_id,
                            const TangoImage* image,
                            const TangoCameraMetadata* metadata) {
  hello_video::HelloVideoApp* app =
      static_cast<hello_video::HelloVideoApp*>(context);
  app->OnImageAvailable(camera_id, image, metadata);
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

  ret = TangoService_connectOnFrameAvailable(TANGO_CAMERA_FISHEYE, this,
                                               OnFrameAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "HelloVideoApp::OnTangoServiceConnected,"
        "Error connecting fisheye %d with onFrameAvailable",
        ret);
  }

  /*ret = TangoService_connectOnFrameAvailable(TANGO_CAMERA_COLOR, this,
                                             OnFrameAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "HelloVideoApp::OnTangoServiceConnected,"
        "Error connecting color %d with onFrameAvailable",
        ret);
  }*/

  /*ret = TangoService_connectOnImageAvailable(TANGO_CAMERA_COLOR, this,
                                             OnImageAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "HelloVideoApp::OnTangoServiceConnected,"
        "Error connecting color %d with onImageAvailable",
        ret);
  }*/


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
}

void HelloVideoApp::OnPause() {
  // Free TangoConfig structure
  if (tango_config_ != nullptr) {
    TangoConfig_free(tango_config_);
    tango_config_ = nullptr;
  }

  // Disconnect from the Tango service
  TangoService_disconnect();
}

void HelloVideoApp::OnFrameAvailable(TangoCameraId camera_id, const TangoImageBuffer* buffer) {
  if (camera_id == TangoCameraId::TANGO_CAMERA_FISHEYE) {
    LOGE("HelloVideoApp::OnFrameAvailable: new fisheye frame available");
  } else if (camera_id == TangoCameraId::TANGO_CAMERA_COLOR) {
    LOGE("HelloVideoApp::OnFrameAvailable: new color frame available");
  } else {
    LOGE("HelloVideoApp::OnFrameAvailable: new frame available with unknown id");
  }
}

void HelloVideoApp::OnImageAvailable(TangoCameraId camera_id,
                      const TangoImage* image,
                      const TangoCameraMetadata* metadata) {
  if (camera_id == TangoCameraId::TANGO_CAMERA_FISHEYE) {
    LOGE("HelloVideoApp::OnImageAvailable: new fisheye image available");
  } else if (camera_id == TangoCameraId::TANGO_CAMERA_COLOR) {
    LOGE("HelloVideoApp::OnImageAvailable: new color image available");
  } else {
    LOGE("HelloVideoApp::OnImageAvailable: new image available with unknown id");
  }
}

}  // namespace hello_video
