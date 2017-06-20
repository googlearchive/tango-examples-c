/*
 * Copyright 2016 Google Inc. All Rights Reserved.
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

#ifndef CPP_BASIC_EXAMPLES_HELLO_MOTION_TRACKING_TANGO_HANDLER_H_
#define CPP_BASIC_EXAMPLES_HELLO_MOTION_TRACKING_TANGO_HANDLER_H_

#include <android/log.h>
#include <jni.h>

#include <tango_client_api.h>  // NOLINT
#include <tango_support.h>     // NOLINT

#define LOG_TAG "cpp_hello_motion_tracking"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace hello_motion_tracking {
// TangoHandler provides functionality to communicate with the Tango Service.
class TangoHandler {
 public:
  TangoHandler() : tango_config_(nullptr) {}

  TangoHandler(const TangoHandler& other) = delete;

  TangoHandler& operator=(const TangoHandler& other) = delete;

  ~TangoHandler() {
    if (tango_config_ != nullptr) {
      TangoConfig_free(tango_config_);
      tango_config_ = nullptr;
    }
  }

  // Check if the Tango Core version is compatible with this app.
  // If not, the applicaiton will exit.
  //
  // @param env, java environment parameter CheckVersion is being called.
  // @param caller_activity, caller of this function.
  void OnCreate(JNIEnv* env, jobject caller_activity);

  // Called when the Tango service is connect. We set the binder object to Tango
  // Service in this function.
  //
  // @param env, java environment parameter.
  // @param iBinder, the native binder object.
  void OnTangoServiceConnected(JNIEnv* env, jobject iBinder);

  // Disconnect and stop Tango service.
  void OnPause();

 private:
  TangoConfig tango_config_;
};
}  // namespace hello_motion_tracking

#endif  // CPP_BASIC_EXAMPLES_HELLO_MOTION_TRACKING_TANGO_HANDLER_H_
