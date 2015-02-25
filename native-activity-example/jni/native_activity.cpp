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
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <tango_client_api.h>
#include <unistd.h>

#include "tango-gl/camera.h"
#include "tango-gl/conversions.h"
#include "tango-gl/cube.h"
#include "tango-gl/frustum.h"
#include "tango-gl/grid.h"
#include "tango-gl/util.h"
#include "tango-gl/video_overlay.h"

bool quit = false;

tango_gl::Transform* cam_parent_transform;
tango_gl::Camera* cam;
tango_gl::Grid* grid;
tango_gl::Cube* cube;
tango_gl::VideoOverlay* video_overlay;

enum CameraType {
  FIRST_PERSON = 0,
  THIRD_PERSON = 1,
  TOP_DOWN = 2
};
CameraType camera_type;

struct engine {
  struct android_app* app;
  TangoConfig config;

  bool visible;
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  int32_t width;
  int32_t height;

  int draw_pipe[2];

  pthread_mutex_t pose_lock;
  TangoPoseData pose;
} engine;

static glm::mat4 projection_mat;
static glm::mat4 projection_mat_ar;
static glm::mat4 view_mat;
static glm::mat4 ow_T_ss;
static glm::mat4 imu_T_d;
static glm::mat4 imu_T_c;
static glm::mat4 c_T_oc;
static glm::mat4 ow_T_oc;
static float image_plane_ratio;
static float image_plane_dis;
static float image_plane_dis_original;

const float kHighFov = 65.0f;
const float kLowFov = 45.0f;
const glm::quat kZeroQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
const glm::vec3 kCubePosition = glm::vec3(-1.0f, 0.265f, -2.0f);
const glm::vec3 kCubeScale = glm::vec3(0.38f, 0.53f, 0.57f);
const glm::vec3 kFloorPosition = glm::vec3(0.0f, -1.4f, 0.0f);
const float kCamViewMaxDist = 100.f;
const float kFovScaler = 0.1f;

// Set camera type, set render camera's parent position and rotation.
void SetCamera(CameraType camera_index) {
  camera_type = camera_index;
  switch (camera_index) {
    case CameraType::FIRST_PERSON:
      cam->SetFieldOfView(kLowFov);
      cam_parent_transform->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
      cam_parent_transform->SetRotation(kZeroQuat);
      break;
    case CameraType::THIRD_PERSON:
      cam->SetFieldOfView(kHighFov);
      cam->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
      cam->SetRotation(kZeroQuat);
      break;
    case CameraType::TOP_DOWN:
      cam->SetFieldOfView(kHighFov);
      cam->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
      cam->SetRotation(kZeroQuat);
      break;
    default:
      break;
  }
}

static void SetupViewport(int w, int h) {
  float screen_ratio = static_cast<float>(h) / static_cast<float>(w);
  if (image_plane_ratio < screen_ratio) {
    glViewport(0, 0, w, w * image_plane_ratio);
  } else {
    glViewport((w - h / image_plane_ratio) / 2, 0, h / image_plane_ratio, h);
  }
}

static bool SetupIntrinsics() {
  LOGI("\tTangoService_getCameraIntrinsics()");
  TangoCameraIntrinsics ccIntrinsics;
  if (TangoService_getCameraIntrinsics(TANGO_CAMERA_COLOR, &ccIntrinsics) !=
      TANGO_SUCCESS) {
    LOGE("TangoService_getCameraIntrinsics(): Failed");
    return false;
  }
  float cc_width = static_cast<float>(ccIntrinsics.width);
  float cc_height = static_cast<float>(ccIntrinsics.height);
  float cc_fx = static_cast<float>(ccIntrinsics.fx);

  image_plane_ratio = cc_height / cc_width;
  image_plane_dis_original = 2.0f * cc_fx / cc_width;
  image_plane_dis = image_plane_dis_original;
  projection_mat_ar = glm::frustum(
      -1.0f * kFovScaler, 1.0f * kFovScaler, -image_plane_ratio * kFovScaler,
      image_plane_ratio * kFovScaler, image_plane_dis * kFovScaler,
      kCamViewMaxDist);
  return true;
}

static bool SetupExtrinsics() {
  // Retrieve the Extrinsic
  TangoPoseData poseData;
  TangoCoordinateFramePair pair;
  pair.base = TANGO_COORDINATE_FRAME_IMU;
  pair.target = TANGO_COORDINATE_FRAME_DEVICE;
  if (TangoService_getPoseAtTime(0.0, pair, &poseData) != TANGO_SUCCESS) {
    LOGE("TangoService_getPoseAtTime(): Failed");
    return false;
  }
  glm::vec3 imu_p_d =
      glm::vec3(poseData.translation[0], poseData.translation[1],
                poseData.translation[2]);
  glm::quat imu_q_d =
      glm::quat(poseData.orientation[3], poseData.orientation[0],
                poseData.orientation[1], poseData.orientation[2]);

  pair.target = TANGO_COORDINATE_FRAME_CAMERA_COLOR;
  if (TangoService_getPoseAtTime(0.0, pair, &poseData) != TANGO_SUCCESS) {
    LOGE("TangoService_getPoseAtTime(): Failed");
    return false;
  }
  glm::vec3 imu_p_c =
      glm::vec3(poseData.translation[0], poseData.translation[1],
                poseData.translation[2]);
  glm::quat imu_q_c =
      glm::quat(poseData.orientation[3], poseData.orientation[0],
                poseData.orientation[1], poseData.orientation[2]);

  imu_T_d = glm::translate(glm::mat4(1.0f), imu_p_d) * glm::mat4_cast(imu_q_d);
  imu_T_c = glm::translate(glm::mat4(1.0f), imu_p_c) * glm::mat4_cast(imu_q_c);

  return true;
}

//
// Initialize an EGL context for the current display.
//
static int engine_init_display(struct engine* engine) {
  // Specify the attributes of the desired configuration,
  // Select an EGLConfig with at least 8 bits per color
  // component compatible with on-screen windows
  const EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE,
                            8,                EGL_GREEN_SIZE, 8,
                            EGL_RED_SIZE,     8,              EGL_NONE};
  EGLint w, h, format;
  EGLint numConfigs;
  EGLConfig config;
  EGLSurface surface;
  EGLContext context;

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  eglInitialize(display, 0, 0);

  // Pick the first EGLConfig that matches our criteria
  eglChooseConfig(display, attribs, &config, 1, &numConfigs);

  // EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
  // guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
  // As soon as the EGLConfig is picked, safely reconfigure the
  // ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID.
  eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

  ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

  surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
  context = eglCreateContext(display, config, NULL, NULL);

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
    LOGI("Unable to eglMakeCurrent");
    return -1;
  }

  eglQuerySurface(display, surface, EGL_WIDTH, &w);
  eglQuerySurface(display, surface, EGL_HEIGHT, &h);

  engine->display = display;
  engine->context = context;
  engine->surface = surface;
  engine->width = w;
  engine->height = h;

  cam_parent_transform = new tango_gl::Transform();
  cam = new tango_gl::Camera();
  grid = new tango_gl::Grid();
  cube = new tango_gl::Cube();
  video_overlay = new tango_gl::VideoOverlay();
  cam->SetParent(cam_parent_transform);
  SetCamera(CameraType::THIRD_PERSON);
  cam->SetAspectRatio(static_cast<float>(w) / static_cast<float>(h));
  grid->SetPosition(kFloorPosition);
  cube->SetPosition(kCubePosition + kFloorPosition);
  cube->SetScale(kCubeScale);

  ow_T_ss = tango_gl::conversions::opengl_world_T_tango_world();
  c_T_oc = tango_gl::conversions::color_camera_T_opengl_camera();

  return 0;
}

// Just the current frame in the display.
static void engine_draw_frame(struct engine* engine) {
  double image_timestamp = 0.f;
  TangoPoseData pose;
  if (engine->display == NULL) {
    LOGI("No Display");
    return;
  }

  eglMakeCurrent(engine->display, engine->surface, engine->surface,
                 engine->context);

  // Get the pose data.
  pthread_mutex_lock(&engine->pose_lock);
  pose = engine->pose;
  pthread_mutex_unlock(&engine->pose_lock);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  TangoService_updateTexture(TANGO_CAMERA_COLOR, &image_timestamp);

  glm::vec3 ss_p_d =
      glm::vec3(pose.translation[0], pose.translation[1], pose.translation[2]);
  glm::quat ss_q_d = glm::quat(pose.orientation[3], pose.orientation[0],
                               pose.orientation[1], pose.orientation[2]);

  glm::mat4 ss_T_d =
      glm::translate(glm::mat4(1.0f), ss_p_d) * glm::mat4_cast(ss_q_d);
  ow_T_oc = ow_T_ss * ss_T_d * glm::inverse(imu_T_d) * imu_T_c * c_T_oc;

  view_mat = glm::inverse(ow_T_oc);

  glDisable(GL_DEPTH_TEST);
  video_overlay->Render(glm::mat4(1.0f), glm::mat4(1.0f));
  glEnable(GL_DEPTH_TEST);
  grid->Render(projection_mat_ar, view_mat);
  cube->Render(projection_mat_ar, view_mat);

  eglSwapBuffers(engine->display, engine->surface);
}

// Tear down the EGL context currently associated with the display.
static void engine_term_display(struct engine* engine) {
  if (engine->display != EGL_NO_DISPLAY) {
    eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);
    if (engine->context != EGL_NO_CONTEXT) {
      eglDestroyContext(engine->display, engine->context);
    }
    if (engine->surface != EGL_NO_SURFACE) {
      eglDestroySurface(engine->display, engine->surface);
    }
    eglTerminate(engine->display);
  }
  delete cube;
  delete video_overlay;
  delete grid;

  engine->visible = false;
  engine->display = EGL_NO_DISPLAY;
  engine->context = EGL_NO_CONTEXT;
  engine->surface = EGL_NO_SURFACE;
  memset(&engine->pose, 0, sizeof(engine->pose));
}

static void onPoseAvailable(void*, const TangoPoseData* pose) {
  const int verbose = 0;
  if (verbose) {
    LOGI("PoseCallback: %lf %lf %lf %lf %lf\t%lf %lf %lf", pose->timestamp,
         pose->orientation[0], pose->orientation[1], pose->orientation[2],
         pose->orientation[3], pose->translation[0], pose->translation[1],
         pose->translation[2]);
  }
  if (pose->status_code == TANGO_POSE_VALID) {
    pthread_mutex_lock(&engine.pose_lock);
    engine.pose = *pose;
    pthread_mutex_unlock(&engine.pose_lock);
  }
}

static void onFrameAvailable(void*, TangoCameraId) {
  double timestamp = 0;
  write(engine.draw_pipe[1], &timestamp, sizeof(timestamp));
}

// Process the next input event.
static int32_t engine_handle_input(struct android_app*, AInputEvent*) {
  return 0;
}

// Process the next main command.
static void engine_handle_cmd(struct android_app*, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_SAVE_STATE:
      LOGI("APP_CMD_SAVE_STATE");
      // The system has asked to save current state.
      // engine.app->savedState = malloc(sizeof(struct saved_state));
      // *((struct saved_state*)engine.app->savedState) = engine.state;
      // engine.app->savedStateSize = sizeof(struct saved_state);
      break;
    case APP_CMD_INIT_WINDOW:
      LOGI("APP_CMD_INIT_WINDOW");
      // The window is being shown, get it ready.
      TangoCoordinateFramePair frames;
      frames.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
      frames.target = TANGO_COORDINATE_FRAME_DEVICE;
      // Attach the onPoseAvailable callback.
      LOGI("\tTangoService_connectOnPoseAvailable()");
      if (TangoService_connectOnPoseAvailable(1, &frames, onPoseAvailable) !=
          TANGO_SUCCESS) {
        LOGE("TangoService_connectOnPoseAvailable(): Failed");
        break;
      }

      engine_init_display(&engine);

      LOGI("\tTangoService_conntextTextureId(COLOR)");
      if (TangoService_connectTextureId(TANGO_CAMERA_COLOR,
                                        video_overlay->GetTextureId(), NULL,
                                        &onFrameAvailable) != TANGO_SUCCESS) {
        LOGE("TangoService_connectTextureId(): Failed");
        break;
      }
      // Connect to the Tango Service.
      LOGI("\tTangoService_connect()");
      TangoService_connect(NULL, engine.config);
      engine.visible = true;

      SetupIntrinsics();
      SetupExtrinsics();
      SetupViewport(engine.width, engine.height);
      break;
    case APP_CMD_TERM_WINDOW:
      LOGI("APP_CMD_TERM_WINDOW");
      // The window is being hidden or closed, clean it up.
      if (engine.visible) {
        engine.visible = false;
        LOGI("\tTangoService_disconnect()");
        engine_term_display(&engine);
        TangoService_disconnect();
      }
      break;
    case APP_CMD_GAINED_FOCUS:
      LOGI("APP_CMD_GAINED_FOCUS");
      // When app gains focus, start monitoring the accelerometer and
      // lock in this configuration.
      break;
    case APP_CMD_LOST_FOCUS:
      LOGI("APP_CMD_LOST_FOCUS");
      // When app loses focus, stop monitoring the accelerometer.
      // This is to avoid consuming battery while not being used.
      break;
    case APP_CMD_RESUME:
      LOGI("APP_CMD_RESUME");
      break;
    case APP_CMD_START:
      LOGI("APP_CMD_START");
      break;
    case APP_CMD_PAUSE:
      LOGI("APP_CMD_PAUSE");
      if (engine.visible) {
        LOGI("\tTangoService_disconnect()");
        engine_term_display(&engine);
        TangoService_disconnect();
        engine.visible = false;
      }
      break;
    case APP_CMD_STOP:
      LOGI("APP_CMD_STOP");
      break;
    case APP_CMD_INPUT_CHANGED:
      LOGI("APP_CMD_INPUT_CHANGED");
      break;
    case APP_CMD_CONFIG_CHANGED:
      LOGI("APP_CMD_CONFIG_CHANGED");
      break;
    case APP_CMD_DESTROY:
      LOGI("APP_CMD_DESTROY");
      break;
    case APP_CMD_WINDOW_RESIZED:
      LOGI("APP_CMD_WINDOW_RESIZED");
      break;
    case APP_CMD_WINDOW_REDRAW_NEEDED:
      LOGI("APP_CMD_WINDOW_REDRAW_NEEDED");
      break;
    case APP_CMD_CONTENT_RECT_CHANGED:
      LOGI("APP_CMD_CONTENT_RECT_CHANGED");
      break;
  }
}

// Parameter names required by ndk-build.
static int engine_draw_frame_callback(int fd, int, void*) {
  double pose_timestamp;
  if (read(fd, &pose_timestamp, sizeof(pose_timestamp)) ==
      sizeof(pose_timestamp)) {
    engine_draw_frame(&engine);
  }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {

  // Make sure glue isn't stripped.
  app_dummy();

  memset(&engine, 0, sizeof(engine));
  state->userData = &engine;
  state->onAppCmd = engine_handle_cmd;
  state->onInputEvent = engine_handle_input;
  engine.app = state;

  pipe(engine.draw_pipe);

  // Get the current JNI context(env) and native activity object(activity)
  // to check the version that the version of Tango Service installed
  // on the device meets the minimum number required by the clien library.
  JNIEnv* env;
  JavaVM* m_vm = state->activity->vm;
  m_vm->AttachCurrentThread(&env, nullptr);
  jobject activity = state->activity->clazz;
  if (env == nullptr) LOGI("JNIEnv is null");
  if (activity == nullptr) LOGI("Native Activity object is null");

  TangoErrorType rval = TANGO_SUCCESS;
  rval = TangoService_initialize(env, activity);
  if (rval == TANGO_INVALID) {
    LOGI("TangoService_initialize() returned INVALID");
  } else if (rval == TANGO_ERROR) {
    LOGI("TangoService_initialize() returned TANGO_ERROR.");
    return;
  } else {
    LOGI("TangoService_initialize(): Success");
  }

  // Allocate a TangoConfig object.
  if ((engine.config = TangoService_getConfig(TANGO_CONFIG_DEFAULT)) == NULL) {
    LOGI("TangoService_getConfig(): Failed");
    return;
  }

  // Set a parameter in TangoConfig.  Disable/Enable for debugging.
  const bool kUseLearningMode = false;
  if (TangoConfig_setBool(engine.config, "config_enable_learning_mode",
                          kUseLearningMode) != 0) {
    LOGI("TangoConfig_setBool() for \"config_enable_learning_mode\": Failed");
    return;
  }

  // Report the current TangoConfig.
  LOGI("TangoConfig:%s", TangoConfig_toString(engine.config));

  // Attach the draw_pipe[0] (reader) to the ALooper.
  ALooper_addFd(ALooper_forThread(), engine.draw_pipe[0], ALOOPER_POLL_CALLBACK,
                ALOOPER_EVENT_INPUT, engine_draw_frame_callback, &engine);

  while (!quit) {
    // Read all pending events.
    int ident;
    int events;
    struct android_poll_source* source;

    // If not visible, block forever waiting for events.
    // If visible, loop until all events are read, then continue
    // to draw the next frame of animation.
    while ((ident = ALooper_pollAll(-1, NULL, &events, (void**)&source)) >= 0) {

      // Process this event.
      if (source != NULL) {
        source->process(state, source);
      }

      // Check if exiting.
      if (state->destroyRequested != 0) {
        quit = true;
        break;
      }
    }
  }

  // Tear down the display.
  engine_term_display(&engine);
  quit = false;
}
