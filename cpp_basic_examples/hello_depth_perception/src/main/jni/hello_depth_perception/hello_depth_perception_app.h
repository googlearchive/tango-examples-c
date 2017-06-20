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

#ifndef CPP_BASIC_EXAMPLES_HELLO_DEPTH_PERCEPTION_HELLO_DEPTH_PERCEPTION_APP_H_
#define CPP_BASIC_EXAMPLES_HELLO_DEPTH_PERCEPTION_HELLO_DEPTH_PERCEPTION_APP_H_

#include <android/log.h>
#include <jni.h>

#include <tango_client_api.h>  // NOLINT
#include <tango_support.h>     // NOLINT

#define LOG_TAG "cpp_hello_depth_perception"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace hello_depth_perception {

// HelloDepthPerceptionApp handles the application lifecycle and resources.
class HelloDepthPerceptionApp {
 public:
  // Class constructor.
  HelloDepthPerceptionApp() : tango_config_(nullptr) {}

  // Class destructor.
  ~HelloDepthPerceptionApp() {
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
  // the corresponding resources in the OnPause() callback from
  // Android, otherwise, this application will hold on to the Tango
  // resources and other application will not be able to connect to
  // Tango Service.
  void OnPause();

 private:
  // Tango configuration file, this object is for configuring Tango Service
  // setup before connecting to service. For instance, we turn on the depth
  // sensing in this example.
  TangoConfig tango_config_;
};
}  // namespace hello_depth_perception

#endif  // CPP_BASIC_EXAMPLES_HELLO_DEPTH_PERCEPTION_HELLO_DEPTH_PERCEPTION_APP_H_
