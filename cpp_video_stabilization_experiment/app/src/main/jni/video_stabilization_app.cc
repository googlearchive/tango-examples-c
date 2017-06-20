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
#include <sstream>
#include <string>

#include <glm/gtx/quaternion.hpp>
#include <tango-gl/conversions.h>

#include "tango-video-stabilization/video_stabilization_app.h"

namespace {
// The minimum Tango Core version required from this application.
constexpr int kTangoCoreMinimumVersion = 9377;

// Far clipping plane of the AR camera.
const float kArCameraNearClippingPlane = 0.1f;
const float kArCameraFarClippingPlane = 100.0f;

// This function routes texture callbacks to the application object for
// handling.
//
// @param context, context will be a pointer to a VideoStabilizationApp
//        instance on which to call callbacks.
// @param id, id of the updated camera..
void OnTextureAvailableRouter(void* context, TangoCameraId id) {
  tango_video_stabilization::VideoStabilizationApp* app =
      static_cast<tango_video_stabilization::VideoStabilizationApp*>(context);
  app->OnTextureAvailable(id);
}

}  // namespace

namespace tango_video_stabilization {

void VideoStabilizationApp::OnTextureAvailable(TangoCameraId id) {
  if (id == TANGO_CAMERA_COLOR) {
    RequestRender();
  }
}

void VideoStabilizationApp::OnCreate(JNIEnv* env, jobject activity) {
  // Check the installed version of the TangoCore.  If it is too old, then
  // it will not support the most up to date features.
  int version;
  TangoErrorType err = TangoSupport_getTangoVersion(env, activity, &version);
  if (err != TANGO_SUCCESS || version < kTangoCoreMinimumVersion) {
    LOGE("VideoStabilizationApp::OnCreate, Tango Core version is out of date.");
    std::exit(EXIT_SUCCESS);
  }

  // We want to be able to trigger rendering on demand in our Java code.
  // As such, we need to store the activity we'd like to interact with and the
  // id of the method we'd like to call on that activity.
  calling_activity_obj_ = env->NewGlobalRef(activity);
  jclass cls = env->GetObjectClass(activity);
  on_demand_render_ = env->GetMethodID(cls, "requestRender", "()V");

  is_service_connected_ = false;
  is_gl_initialized_ = false;
}

bool VideoStabilizationApp::OnTangoServiceConnected(JNIEnv* env,
                                                    jobject iBinder) {
  TangoErrorType ret = TangoService_setBinder(env, iBinder);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "VideoStabilizationApp: Failed to set Tango binder with"
        "error code: %d",
        ret);
    return false;
  }

  TangoSetupConfig();
  TangoConnectCallbacks();
  TangoConnect();

  is_service_connected_ = true;

  UpdateViewportAndProjectionMatrix();

  return true;
}

void VideoStabilizationApp::ActivityDestroyed() {
  JNIEnv* env;
  java_vm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
  env->DeleteGlobalRef(calling_activity_obj_);

  calling_activity_obj_ = nullptr;
  on_demand_render_ = nullptr;
}

int VideoStabilizationApp::TangoSetupConfig() {
  // Here, we'll configure the service to run in the way we'd want. For this
  // application, we'll start from the default configuration
  // (TANGO_CONFIG_DEFAULT). This enables basic motion tracking capabilities.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE("VideoStabilizationApp: Failed to get default config form");
    return TANGO_ERROR;
  }

  // Set auto-recovery for motion tracking as requested by the user.
  int ret =
      TangoConfig_setBool(tango_config_, "config_enable_auto_recovery", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "VideoStabilizationApp: config_enable_auto_recovery() failed with error"
        "code: %d",
        ret);
    return ret;
  }

  // Enable color camera from config.
  ret = TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "VideoStabilizationApp: config_enable_color_camera() failed with error"
        "code: %d",
        ret);
    return ret;
  }

  // Low latency IMU integration enables aggressive integration of the latest
  // inertial measurements to provide lower latency pose estimates. This will
  // improve the AR experience.
  ret = TangoConfig_setBool(tango_config_,
                            "config_enable_low_latency_imu_integration", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "VideoStabilizationApp: config_enable_low_latency_imu_integration() "
        "failed with error code: %d",
        ret);
    return ret;
  }

  return ret;
}

// Connect to Tango Service, service will start running, and
// pose can be queried.
bool VideoStabilizationApp::TangoConnect() {
  TangoErrorType ret = TangoService_connect(this, tango_config_);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "VideoStabilizationApp: Failed to connect to the Tango service with"
        "error code: %d",
        ret);
    return false;
  }

  // Initialize TangoSupport context.
  TangoSupport_initialize(TangoService_getPoseAtTime,
                          TangoService_getCameraIntrinsics);

  return true;
}

void VideoStabilizationApp::TangoConnectCallbacks() {
  // Connect color camera texture.
  TangoErrorType ret = TangoService_connectOnTextureAvailable(
      TANGO_CAMERA_COLOR, this, OnTextureAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "VideoStabilizationApp: Failed to connect texture callback with error"
        "code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }
}

void VideoStabilizationApp::OnPause() {
  if (is_service_connected_) {
    TangoDisconnect();
    is_service_connected_ = false;
  }

  DeleteResources();
}

void VideoStabilizationApp::TangoDisconnect() {
  // When disconnecting from the Tango Service, it is important to make sure to
  // free your configuration object. Note that disconnecting from the service,
  // resets all configuration, and disconnects all callbacks. If an application
  // resumes after disconnecting, it must re-register configuration and
  // callbacks with the service.
  TangoConfig_free(tango_config_);
  tango_config_ = nullptr;
  TangoService_disconnect();
}

void VideoStabilizationApp::InitializeGLContent() {
  main_scene_.InitGLContent();
  is_gl_initialized_ = true;
  UpdateViewportAndProjectionMatrix();
}

void VideoStabilizationApp::SetViewPort(int width, int height) {
  viewport_width_ = width;
  viewport_height_ = height;
  UpdateViewportAndProjectionMatrix();
}

void VideoStabilizationApp::UpdateViewportAndProjectionMatrix() {
  if (!is_service_connected_ || !is_gl_initialized_) {
    return;
  }

  // Query intrinsics for the color camera from the Tango Service, because we
  // want to match the virtual render camera's intrinsics to the physical
  // camera, we will compute the actually projection matrix and the view port
  // ratio for the render.
  float image_plane_ratio = 0.0f;
  int ret = TangoService_getCameraIntrinsics(TANGO_CAMERA_COLOR,
                                             &color_camera_intrinsics_);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "VideoStabilizationApp: Failed to get camera intrinsics with error"
        "code: %d",
        ret);
    return;
  }

  float image_width = static_cast<float>(color_camera_intrinsics_.width);
  float image_height = static_cast<float>(color_camera_intrinsics_.height);
  float fx = static_cast<float>(color_camera_intrinsics_.fx);
  float fy = static_cast<float>(color_camera_intrinsics_.fy);
  float cx = static_cast<float>(color_camera_intrinsics_.cx);
  float cy = static_cast<float>(color_camera_intrinsics_.cy);
  float image_plane_distance = 2.0f * fx / image_width;

  glm::mat4 projection_mat_ar;
  if (viewport_width_ > viewport_height_) {
    projection_mat_ar = tango_gl::Camera::ProjectionMatrixForCameraIntrinsics(
        image_width, image_height, fx, fy, cx, cy, kArCameraNearClippingPlane,
        kArCameraFarClippingPlane);
    image_plane_ratio = image_height / image_width;
  } else {
    projection_mat_ar = tango_gl::Camera::ProjectionMatrixForCameraIntrinsics(
        image_height, image_width, fy, fx, cy, cx, kArCameraNearClippingPlane,
        kArCameraFarClippingPlane);
    image_plane_ratio = image_width / image_height;
  }
  main_scene_.SetProjectionMatrix(projection_mat_ar);
  main_scene_.SetImagePlaneDistance(image_plane_distance);
  main_scene_.SetCameraImagePlaneRatio(image_plane_ratio);
  main_scene_.SetDisplayRotation(TANGO_SUPPORT_ROTATION_IGNORED);

  float screen_ratio = static_cast<float>(viewport_height_) /
                       static_cast<float>(viewport_width_);
  // In the following code, we place the view port at (0, 0) from the bottom
  // left corner of the screen. By placing it at (0,0), the view port may not
  // be exactly centered on the screen. However, this won't affect AR
  // visualization as the correct registration of AR objects relies on the
  // aspect ratio of the screen and video overlay, but not the position of the
  // view port.
  //
  // To place the view port in the center of the screen, please use following
  // code:
  //
  // if (image_plane_ratio < screen_ratio) {
  //   glViewport(-(h / image_plane_ratio - w) / 2, 0,
  //              h / image_plane_ratio, h);
  // } else {
  //   glViewport(0, -(w * image_plane_ratio - h) / 2, w,
  //              w * image_plane_ratio);
  // }

  if (image_plane_ratio < screen_ratio) {
    main_scene_.SetupViewport(viewport_height_ / image_plane_ratio,
                              viewport_height_);
  } else {
    main_scene_.SetupViewport(viewport_width_,
                              viewport_width_ * image_plane_ratio);
  }
}

void VideoStabilizationApp::Render() {
  // If tracking is lost, further down in this method Scene::Render
  // will not be called. Prevent flickering that would otherwise
  // happen by rendering solid black as a fallback.
  main_scene_.Clear();

  if (!is_service_connected_) {
    return;
  }

  TangoErrorType status = TangoService_updateTextureExternalOes(
      TANGO_CAMERA_COLOR, main_scene_.GetVideoOverlayTextureId(),
      &last_gpu_timestamp_);

  if (status == TANGO_SUCCESS) {
    TangoPoseData pose;
    TangoSupport_getPoseAtTime(
        0.0, TANGO_COORDINATE_FRAME_START_OF_SERVICE,
        TANGO_COORDINATE_FRAME_CAMERA_COLOR, TANGO_SUPPORT_ENGINE_OPENGL,
        TANGO_SUPPORT_ENGINE_OPENGL, TANGO_SUPPORT_ROTATION_IGNORED, &pose);
    if (pose.status_code == TANGO_POSE_VALID) {
      main_scene_.AddNewPose(pose);
      main_scene_.Render();
    } else {
      LOGE(
          "VideoStabilizationApp: Could not find a valid pose at "
          "time %lf for the color camera.",
          last_gpu_timestamp_);
    }
  } else {
    LOGE(
        "VideoStabilizationApp: Failed to update video overlay texture with "
        "error code: %d",
        status);
  }
}

void VideoStabilizationApp::DeleteResources() {
  main_scene_.DeleteResources();
  is_gl_initialized_ = false;
}

void VideoStabilizationApp::RequestRender() {
  if (calling_activity_obj_ == nullptr || on_demand_render_ == nullptr) {
    LOGE("Can not reference Activity to request render");
    return;
  }

  // Here, we notify the Java activity that we'd like it to trigger a render.
  JNIEnv* env;
  java_vm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
  env->CallVoidMethod(calling_activity_obj_, on_demand_render_);
}

void VideoStabilizationApp::SetEnableVideoStabilization(
    bool enable_video_stabilization) {
  main_scene_.SetEnableVideoStabilization(enable_video_stabilization);
}

void VideoStabilizationApp::SetCameraLocked(bool camera_locked) {
  main_scene_.SetCameraLocked(camera_locked);
}
}  // namespace tango_video_stabilization
