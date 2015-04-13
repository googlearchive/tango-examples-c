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

#ifndef HELLO_TANGO_JNI_TANGO_HANDLER_H_
#define HELLO_TANGO_JNI_TANGO_HANDLER_H_

#include <android/log.h>

#include "tango_client_api.h"  // NOLINT

#define LOG_TAG "hello-tango-jni"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace hello_tango_jni {
// TangoHandler provide functionalities to communicate with Tango Service.
class TangoHandler {
 public:
  TangoHandler();
  ~TangoHandler();

  // Initialize Tango Service, this function starts the communication
  // between the application and Tango Service.
  // The activity object is used for checking if the API version is outdated.
  TangoErrorType Initialize(JNIEnv* env, jobject caller_activity);

  // Setup the configuration file of Tango Service.
  TangoErrorType SetupConfig();

  // Connect the onPoseAvailable callback.
  TangoErrorType ConnectPoseCallback();

  // Connect to Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start Motion Tracking.
  TangoErrorType ConnectService();

  // Disconnect from Tango Service, release all the resources that the app is
  // holding from Tango Service.
  void DisconnectService();

 private:
  TangoConfig tango_config_;
};
}  // namespace hello_tango_jni

#endif  // HELLO_TANGO_JNI_TANGO_HANDLER_H_
