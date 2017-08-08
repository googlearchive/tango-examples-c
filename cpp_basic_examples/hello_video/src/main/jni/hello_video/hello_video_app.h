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

#ifndef CPP_BASIC_EXAMPLES_HELLO_VIDEO_HELLO_VIDEO_APP_H_
#define CPP_BASIC_EXAMPLES_HELLO_VIDEO_HELLO_VIDEO_APP_H_

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
  // the coresponding resources in the OnPause() callback from
  // Android, otherwise, this application will hold on to the Tango
  // resources and other application will not be able to connect to
  // Tango Service.
  void OnPause();


  void OnFrameAvailable(TangoCameraId camera_id, const TangoImageBuffer* buffer);
  void OnImageAvailable(TangoCameraId camera_id,
                        const TangoImage* image,
                        const TangoCameraMetadata* metadata);



 private:
  // Tango configration file, this object is for configuring Tango Service setup
  // before connect to service. For example, we set the flag
  // config_enable_auto_recovery based user's input and then start Tango.
  TangoConfig tango_config_;
};
}  // namespace hello_video

#endif  // CPP_BASIC_EXAMPLES_HELLO_VIDEO_HELLO_VIDEO_APP_H_
