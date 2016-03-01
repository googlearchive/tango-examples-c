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

#define GLM_FORCE_RADIANS

#include <android_native_app_glue.h>

#include "native-activity-example/native_activity_application.h"

namespace {

const int MIN_TANGO_CORE_VERSION = 6804;

}  // namespace

#ifdef __cplusplus
extern "C" {
#endif
/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* /*app*/,
                                   AInputEvent* /*event*/) {
  /* No input is used in this application. */
  return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
  NativeActivityApplication* app_engine =
      reinterpret_cast<NativeActivityApplication*>(app->userData);
  switch (cmd) {
    case APP_CMD_SAVE_STATE:
      LOGI("APP_CMD_SAVE_STATE");
      /* The system has asked us to save our current state.  Do so here.
       * In this simple example application, we do not save any state. */
      break;
    case APP_CMD_INIT_WINDOW:
      LOGI("APP_CMD_INIT_WINDOW");
      /* Configure tango service here.*/

      /* Allocate a TangoConfig object and configure Tango Service setup. */
      if (!app_engine->TangoSetupConfig()) {
        LOGE("app_engine->TangoSetupConfig(): Failed.");
        return;
      }
      LOGI("app_engine->TangoSetupConfig(): Success.");

      if (!app_engine->OnSurfaceCreated()) {
        LOGE("app_engine->OnSurfaceCreated(): Failed.");
      }
      LOGI("app_engine->OnSurfaceCreated(): Success.");
      break;
    case APP_CMD_TERM_WINDOW:
      LOGI("APP_CMD_TERM_WINDOW");
      /* The window is being hidden or closed, clean it up. */
      app_engine->Terminate();
      break;
    case APP_CMD_PAUSE:
      LOGI("APP_CMD_PAUSE");
      app_engine->TangoDisconnect();
      break;
  }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 *
 * Reference Tutorial:
 * http://developer.android.com/reference/android/app/NativeActivity.html
 */
void android_main(struct android_app* state) {
  NativeActivityApplication app_engine;

  /* Make sure glue isn't stripped. */
  app_dummy();

  state->userData = &app_engine;
  state->onAppCmd = engine_handle_cmd;
  state->onInputEvent = engine_handle_input;
  app_engine.SetAppState(state);

  /* Initialize tango service here.*/

  /* Get the current JNI context(env) and native activity object(activity)
   * to check the version that the version of Tango Service installed
   * on the device meets the minimum number required by the client library. */
  JNIEnv* env;
  state->activity->vm->AttachCurrentThread(&env, nullptr);
  jobject activity = state->activity->clazz;

  if (env == nullptr) {
    LOGE("JNIEnv is null");
    return;
  }
  if (activity == nullptr) {
    LOGE("Native Activity object is null");
    return;
  }

  if (!app_engine.CheckTangoVersion(env, activity, MIN_TANGO_CORE_VERSION)) {
    LOGE("app_engine.CheckTangoVersion(): Failed.");
    return;
  }

  /* loop waiting for stuff to do. */
  while (1) {
    /* Read all pending events. */
    int ident;
    int events;
    struct android_poll_source* source;

    /* If it is not visible, we will block forever waiting for events.
     * Otherwise, we loop until all events are read, then continue
     * to draw the next frame. */
    while (
        (ident = ALooper_pollAll(app_engine.IsVisible() ? 0 : -1, NULL, &events,
                                 reinterpret_cast<void**>(&source))) >= 0) {
      /* Process this event. */
      if (source != NULL) {
        source->process(state, source);
      }

      /* Check if we are exiting. */
      if (state->destroyRequested != 0) {
        app_engine.EngineTermDisplay();
        return;
      }
    }

    if (app_engine.IsVisible()) {
      app_engine.Render();
    }
  }
}
#ifdef __cplusplus
}
#endif
