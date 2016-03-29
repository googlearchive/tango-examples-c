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

#ifndef HELLO_MOTION_TRACKING_TANGO_HANDLER_H_
#define HELLO_MOTION_TRACKING_TANGO_HANDLER_H_

#include <android/log.h>

#include "tango_client_api.h"   // NOLINT
#include "tango_support_api.h"  // NOLINT

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

  // OnCreate() callback is called when this Android application's
  // OnCreate function is called from UI thread. In the OnCreate
  // function, we are only checking the Tango Core's version.
  //
  // @param env, java environment parameter OnCreate is being called.
  // @param caller_activity, caller of this function.
  void OnCreate(JNIEnv* env, jobject caller_activity);

  // onResume() callback is called when this Android application's
  // onResume function is called from UI thread. In our applicaiton,
  // we queries Tango configuration and start Tango Service in the
  // onResume function.
  void OnResume();

  // OnPause() callback is called when this Android application's
  // OnCreate function is called from UI thread. In our application,
  // we disconnect Tango Service and free the Tango configuration
  // file. It is important to disconnect Tango Service and release
  // the coresponding resources in the OnPause() callback from
  // Android, otherwise, this application will hold on to the Tango
  // resources and other application will not be able to connect to
  // Tango Service.
  void OnPause();

 private:
  TangoConfig tango_config_;
};
}  // namespace hello_motion_tracking

#endif  // HELLO_MOTION_TRACKING_TANGO_HANDLER_H_
