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

#include <tango_support_api.h>

#include "native-activity-example/native_activity_application.h"

namespace {
/* Cube attributes. */
const glm::vec3 kCubePosition = glm::vec3(-1.0f, 0.265f, -2.0f);
const glm::vec3 kCubeScale = glm::vec3(0.38f, 0.53f, 0.57f);
const tango_gl::Color kCubeColor = tango_gl::Color(0.f, 1.0f, 0.f);

/* Grid attributes. */
const tango_gl::Color kGridColor = tango_gl::Color(0.85f, 0.85f, 0.85f);

/* Shared attributes for cube and grid. */
const glm::vec3 kFloorPosition = glm::vec3(0.0f, -1.4f, 0.0f);
static constexpr float kCamViewMaxDist = 100.f;
static constexpr float kFovScaler = 0.1f;

/* Tango config attributes. */
static constexpr bool kUseLearningMode = false;
static constexpr double kCurrentPoseTimestamp = 0.0;
}  // namespace

NativeActivityApplication::NativeActivityApplication()
    : app_(nullptr),
      visible_(false),
      width_(0),
      height_(0),
      image_plane_ratio_(0.0f) {
  start_service_to_device_frame_pair_.base =
      TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  start_service_to_device_frame_pair_.target = TANGO_COORDINATE_FRAME_DEVICE;
}

NativeActivityApplication::~NativeActivityApplication() {}

void NativeActivityApplication::EngineTermDisplay() {
  if (display_ != EGL_NO_DISPLAY) {
    eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (context_ != EGL_NO_CONTEXT) {
      eglDestroyContext(display_, context_);
    }
    if (surface_ != EGL_NO_SURFACE) {
      eglDestroySurface(display_, surface_);
    }
    eglTerminate(display_);
  }

  SetVisible(false);
  display_ = EGL_NO_DISPLAY;
  context_ = EGL_NO_CONTEXT;
  surface_ = EGL_NO_SURFACE;
}

bool NativeActivityApplication::IsVisible() { return visible_; }

bool NativeActivityApplication::OnSurfaceCreated() {
  if (app_->window == nullptr) {
    LOGE("NativeActivityApplication::OnSurfaceCreated: Window is null.");
    return false;
  }

  /* Initialize render objects. */
  if (!InitializeDisplay()) {
    LOGE("NativeActivityApplication::InitializeDisplay(): Failed.");
    return false;
  }
  LOGI("NativeActivityApplication::InitializeDisplay(): Success.");

  /* Connect texture. */
  if (!TangoConnectTexture()) {
    LOGE("NativeActivityApplication::TangoConnectTexture(): Failed.");
    return false;
  }
  LOGI("NativeActivityApplication::TangoConnectTexture(): Success.");

  /* Connect to tango service. */
  if (!TangoConnect()) {
    LOGE("NativeActivityApplication::TangoConnect(): Failed.");
    return false;
  }
  LOGI("NativeActivityApplication::TangoConnect(): Success.");

  /* Setup and render display. */
  SetUpDisplay();
  Render();
  return true;
}

void NativeActivityApplication::Render() {
  if (display_ == nullptr) {
    LOGE("NativeActivityApplication::Render: No Display.");
    return;
  }

  /* Store the current view matrix. */
  glm::mat4 view_matrix;

  eglMakeCurrent(display_, surface_, surface_, context_);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  /* Use the updated texture time to update pose. */
  if (UpdateTextureAndPose()) {
    UpdateViewMatrix(&view_matrix);
  }

  /* Render texture as a background using identity matrix. */
  glDisable(GL_DEPTH_TEST);
  video_overlay_->Render(glm::mat4(1.0f), glm::mat4(1.0f));
  glEnable(GL_DEPTH_TEST);

  /* Render the grid and the cube. */
  grid_->Render(projection_mat_ar_, view_matrix);
  cube_->Render(projection_mat_ar_, view_matrix);

  eglSwapBuffers(display_, surface_);
}

void NativeActivityApplication::SetAppState(struct android_app* state) {
  if (state == nullptr) {
    LOGE("NativeActivityApplication::SetAppState: state is NULL.");
  } else {
    app_ = state;
  }
}

bool NativeActivityApplication::TangoConnect() {
  /* Here, we'll connect to the TangoService and set up to run. Note that we're
   * passing in a pointer to ourselves as the context which will be passed back
   * in our callbacks if necessary. */
  TangoErrorType retval = TangoService_connect(this, tango_config_);
  if (retval != TANGO_SUCCESS) {
    LOGE("NativeActivityApplication::TangoConnect Failed with error code %d.",
         retval);
    return false;
  }
  LOGI("NativeActivityApplication::TangoConnect Success.");
  return true;
}

bool NativeActivityApplication::CheckTangoVersion(JNIEnv* env, jobject activity,
                                                  int min_tango_version) {
  if (env == nullptr) {
    LOGE("NativeActivityApplication::TangoInitialize env is NULL.");
  }
  // Check the installed version of the TangoCore.  If it is too old, then
  // it will not support the most up to date features.
  int version;
  TangoErrorType err = TangoSupport_GetTangoVersion(env, activity, &version);

  if (err != TANGO_SUCCESS) {
    LOGE("TangoService_initialize(): Failed with error code: %d.", err);
    return false;
  }
  if (version < min_tango_version) {
    LOGE(
        "TangoService_initialize(): Installed version is lower than the "
        "minimum required version");
    return false;
  }

  LOGI("TangoService_initialize(): Success.");
  return true;
}

bool NativeActivityApplication::TangoSetupConfig() {
  /* Here, we'll configure the service to run in the way we'd want. For this
   * application, we'll start from the default configuration
   * (TANGO_CONFIG_DEFAULT). This enables basic motion tracking capabilities. */
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE("TangoService_getConfig(): Failed.");
    return false;
  }
  LOGI("TangoService_getConfig(): Success.");

  /* Enable color camera for augmented reality application. */
  if (!EnableCamera(&tango_config_)) {
    LOGE("NativeActivityApplication::EnableCamera: Failed.");
    return false;
  }

  /* Report the current successful TangoConfig. */
  LOGI("TangoConfig:%s", TangoConfig_toString(tango_config_));
  return true;
}

void NativeActivityApplication::Terminate() {
  if (IsVisible()) {
    EngineTermDisplay();
    SetVisible(false);
  }
}

/* Private helper functions. */
bool NativeActivityApplication::EnableCamera(TangoConfig* tango_config) {
  if (tango_config == nullptr) {
    LOGE("tango_config should not be NULL.");
    return false;
  }
  TangoErrorType retval =
      TangoConfig_setBool(*tango_config, "config_enable_color_camera", true);
  if (retval != TANGO_SUCCESS) {
    LOGE(
        "TangoConfig_setBool() for \"config_enable_color_camera\": Failed with"
        " error code: %d.",
        retval);
    return false;
  }
  LOGI("TangoConfig_setBool() for \"config_enable_color_camera\": Success.");

  /* It is important to add this configuration if color camera is enabled.
   * Low latency IMU integration enables aggressive integration of the latest
   * inertial measurements to provide lower latency pose estimates. This will
   * improve the AR experience. */
  retval = TangoConfig_setBool(
      tango_config_, "config_enable_low_latency_imu_integration", true);
  if (retval != TANGO_SUCCESS) {
    LOGE(
        "TangoConfig_setBool() for \"config_enable_low_latency_imu_integration"
        "\": Failed with error code: %d.",
        retval);
    return false;
  }
  LOGI(
      "TangoConfig_setBool() for \"config_enable_low_latency_imu_integration\""
      ": Success.");
  return true;
}

bool NativeActivityApplication::GetPoseAtTime(
    const double image_timestamp, TangoPoseData* start_service_T_device) {
  TangoPoseData temp_pose;
  TangoErrorType retval = TangoService_getPoseAtTime(
      image_timestamp, start_service_to_device_frame_pair_, &temp_pose);
  if (retval != TANGO_SUCCESS) {
    LOGE(
        "NativeActivityApplication::GetPoseAtTime: Could not find a pose"
        " at time %lf for the color camera.",
        image_timestamp);
    return false;
  }

  /* If it's successful, only output the updated pose if it is valid. */
  if (temp_pose.status_code != TANGO_POSE_VALID) {
    return false;
  }

  *start_service_T_device = temp_pose;
  return true;
}

bool NativeActivityApplication::InitializeDisplay() {
  /* initialize OpenGL ES and EGL.*/

  /* Here specify the attributes of the desired configuration.
   * Below, we select an EGLConfig with at least 8 bits per color
   * component compatible with on-screen windows. */
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

  /* Here, the application chooses the configuration it desires. In this
   * sample, we have a very simplified selection process, where we pick
   * the first EGLConfig that matches our criteria. */
  eglChooseConfig(display, attribs, &config, 1, &numConfigs);

  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
   * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
   * As soon as we picked a EGLConfig, we can safely reconfigure the
   * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

  ANativeWindow_setBuffersGeometry(app_->window, 0, 0, format);

  surface = eglCreateWindowSurface(display, config, app_->window, NULL);
  context = eglCreateContext(display, config, NULL, NULL);

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
    LOGI("Unable to eglMakeCurrent");
    return false;
  }

  eglQuerySurface(display, surface, EGL_WIDTH, &w);
  eglQuerySurface(display, surface, EGL_HEIGHT, &h);

  display_ = display;
  context_ = context;
  surface_ = surface;
  width_ = w;
  height_ = h;

  /* Setup rendering objects. */
  InitializeCube();
  InitializeGrid();
  video_overlay_.reset(new tango_gl::VideoOverlay());
  /* We'll store the fixed transform between the opengl frame convention.
   * (Y-up, X-right) and tango frame convention. (Z-up, X-right). */
  opengl_world_T_start_service_ =
      tango_gl::conversions::opengl_world_T_tango_world();
  color_camera_T_opengl_camera_ =
      tango_gl::conversions::color_camera_T_opengl_camera();
  return true;
}

void NativeActivityApplication::InitializeCube() {
  cube_.reset(new tango_gl::Cube());
  cube_->SetPosition(kCubePosition + kFloorPosition);
  cube_->SetScale(kCubeScale);
  cube_->SetColor(kCubeColor);
}

void NativeActivityApplication::InitializeGrid() {
  grid_.reset(new tango_gl::Grid());
  grid_->SetPosition(kFloorPosition);
  grid_->SetColor(kGridColor);
}

void NativeActivityApplication::SetUpDisplay() {
  if (!SetUpIntrinsics()) {
    LOGE("NativeActivityApplication::SetupIntrinsics(): Failed.");
    return;
  }

  if (!SetUpExtrinsics()) {
    LOGE("NativeActivityApplication::SetupExtrinsics(): Failed.");
    return;
  }

  SetUpViewport();
  SetVisible(true);
}

bool NativeActivityApplication::SetUpExtrinsics() {
  /* We need to get the extrinsic transform between the color camera and the
   * imu coordinate frames. This matrix is then used to compute the extrinsic
   * transform between color camera and device: c_T_d = c_T_imu * imu_T_d;
   * Note that the matrix c_T_d is a constant transformation since the hardware
   * will not change, we use the getPoseAtTime() function to query it once right
   * after the Tango Service connected and store it for efficiency. */
  TangoPoseData pose_data;
  TangoCoordinateFramePair tango_coordinate_frame_pair_temp;
  TangoErrorType retval;

  /* Get pose data from imu to device. */
  tango_coordinate_frame_pair_temp.base = TANGO_COORDINATE_FRAME_IMU;
  tango_coordinate_frame_pair_temp.target = TANGO_COORDINATE_FRAME_DEVICE;
  retval = TangoService_getPoseAtTime(
      kCurrentPoseTimestamp, tango_coordinate_frame_pair_temp, &pose_data);
  if (retval != TANGO_SUCCESS) {
    LOGE("TangoService_getPoseAtTime(): Failed with error code %d", retval);
    return false;
  }
  /* Copy out the translation and orientation parts from the pose data. */
  glm::vec3 imu_p_d =
      glm::vec3(pose_data.translation[0], pose_data.translation[1],
                pose_data.translation[2]);
  glm::quat imu_q_d =
      glm::quat(pose_data.orientation[3], pose_data.orientation[0],
                pose_data.orientation[1], pose_data.orientation[2]);

  /* Get pose data from imu to color camera. */
  tango_coordinate_frame_pair_temp.target = TANGO_COORDINATE_FRAME_CAMERA_COLOR;
  retval = TangoService_getPoseAtTime(
      kCurrentPoseTimestamp, tango_coordinate_frame_pair_temp, &pose_data);
  if (retval != TANGO_SUCCESS) {
    LOGE("TangoService_getPoseAtTime(): Failed with error code %d", retval);
    return false;
  }

  /* Copy out the translation and orientation parts from the updated pose data.
   */
  glm::vec3 imu_p_cam =
      glm::vec3(pose_data.translation[0], pose_data.translation[1],
                pose_data.translation[2]);
  glm::quat imu_q_cam =
      glm::quat(pose_data.orientation[3], pose_data.orientation[0],
                pose_data.orientation[1], pose_data.orientation[2]);

  /* Set transformations for IMU to device, and IMU to camera in order to
   * calculate view matrix for render objects. */
  imu_T_device_ =
      glm::translate(glm::mat4(1.0f), imu_p_d) * glm::mat4_cast(imu_q_d);
  imu_T_color_camera_ =
      glm::translate(glm::mat4(1.0f), imu_p_cam) * glm::mat4_cast(imu_q_cam);

  return true;
}

bool NativeActivityApplication::SetUpIntrinsics() {
  TangoCameraIntrinsics ccIntrinsics;
  TangoErrorType retval;
  retval = TangoService_getCameraIntrinsics(TANGO_CAMERA_COLOR, &ccIntrinsics);
  if (retval != TANGO_SUCCESS) {
    LOGE("TangoService_getCameraIntrinsics(): Failed with error code %d.",
         retval);
    return false;
  }

  SetImagePlaneRatio(ccIntrinsics);
  SetProjectionMatAr(ccIntrinsics);
  return true;
}

void NativeActivityApplication::SetImagePlaneRatio(
    const TangoCameraIntrinsics& ccIntrinsics) {
  image_plane_ratio_ = static_cast<float>(ccIntrinsics.height) /
                       static_cast<float>(ccIntrinsics.width);
}

void NativeActivityApplication::SetProjectionMatAr(
    const TangoCameraIntrinsics& ccIntrinsics) {
  float image_plane_dis = 2.0f * static_cast<float>(ccIntrinsics.fx) /
                          static_cast<float>(ccIntrinsics.width);

  projection_mat_ar_ = glm::frustum(
      -1.0f * kFovScaler, 1.0f * kFovScaler, -image_plane_ratio_ * kFovScaler,
      image_plane_ratio_ * kFovScaler, image_plane_dis * kFovScaler,
      kCamViewMaxDist);
}

void NativeActivityApplication::SetUpViewport() {
  double screen_ratio =
      static_cast<float>(height_) / static_cast<float>(width_);
  if (image_plane_ratio_ < screen_ratio) {
    glViewport(0, 0, height_ / image_plane_ratio_, height_);
  } else {
    glViewport((width_ - height_ / image_plane_ratio_) / 2, 0,
               height_ / image_plane_ratio_, height_);
  }
}

void NativeActivityApplication::SetVisible(bool visible) { visible_ = visible; }

bool NativeActivityApplication::TangoConnectTexture() {
  /* The Tango service allows you to connect an OpenGL texture directly to its
   * RGB and fisheye cameras. This is the most efficient way of receiving
   * images from the service because it avoids copies. You get access to the
   * graphic buffer directly. As we're interested in rendering the color image
   * in our render loop, we'll be polling for the color image as needed. */
  TangoErrorType retval = TangoService_connectTextureId(
      TANGO_CAMERA_COLOR, video_overlay_->GetTextureId(), this, nullptr);

  if (retval != TANGO_SUCCESS) {
    LOGE("TangoService_connectTextureId(): Failed with error code %d.", retval);
    return false;
  }
  LOGI("TangoService_connectTextureId(): Success.");
  return true;
}

void NativeActivityApplication::TangoDisconnect() {
  TangoConfig_free(tango_config_);
  tango_config_ = nullptr;
  TangoService_disconnect();
}

bool NativeActivityApplication::UpdateTextureAndPose() {
  double texture_timestamp = 0.0f;
  TangoErrorType retval =
      TangoService_updateTexture(TANGO_CAMERA_COLOR, &texture_timestamp);
  if (retval == TANGO_SUCCESS) {
    /* Update pose with the texture timestamp only if texture is updated. */
    if (GetPoseAtTime(texture_timestamp, &tango_pose_)) {
      return true;
    } else {
      LOGI(
          "NativeActivityApplication:GetPoseAtTime Failed!"
          " Timestampe: %lf. Continue rendering with previous pose.",
          texture_timestamp);
    }
  } else {
    LOGI(
        "NativeActivityApplication:TangoService_updateTexture Failed!"
        " Timestampe: %lf. Continue rendering with previous pose.",
        texture_timestamp);
  }

  return false;
}

void NativeActivityApplication::UpdateViewMatrix(glm::mat4* view_matrix) {
  /* Get the transformation from Tango world to device. */
  glm::vec3 tw_p_d =
      glm::vec3(tango_pose_.translation[0], tango_pose_.translation[1],
                tango_pose_.translation[2]);
  glm::quat tw_q_d =
      glm::quat(tango_pose_.orientation[3], tango_pose_.orientation[0],
                tango_pose_.orientation[1], tango_pose_.orientation[2]);

  glm::mat4 tw_T_d =
      glm::translate(glm::mat4(1.0f), tw_p_d) * glm::mat4_cast(tw_q_d);

  /* Get the transformation from OpenGL world to OpenGL camera. */
  glm::mat4 ow_T_oc = opengl_world_T_start_service_ * tw_T_d *
                      glm::inverse(imu_T_device_) * imu_T_color_camera_ *
                      color_camera_T_opengl_camera_;

  /* Set view matrix. */
  *view_matrix = glm::inverse(ow_T_oc);
}
