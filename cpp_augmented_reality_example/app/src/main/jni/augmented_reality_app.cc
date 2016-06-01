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

#include <tango-gl/conversions.h>
#include <tango_support_api.h>

#include "tango-augmented-reality/augmented_reality_app.h"

namespace {
const int kVersionStringLength = 128;

// Far clipping plane of the AR camera.
const float kArCameraNearClippingPlane = 0.1f;
const float kArCameraFarClippingPlane = 100.0f;

// This function routes onTangoEvent callbacks to the application object for
// handling.
//
// @param context, context will be a pointer to a AugmentedRealityApp
//        instance on which to call callbacks.
// @param event, TangoEvent to route to onTangoEventAvailable function.
void onTangoEventAvailableRouter(void* context, const TangoEvent* event) {
  tango_augmented_reality::AugmentedRealityApp* app =
      static_cast<tango_augmented_reality::AugmentedRealityApp*>(context);
  app->onTangoEventAvailable(event);
}

// This function routes texture callbacks to the application object for
// handling.
//
// @param context, context will be a pointer to a AugmentedRealityApp
//        instance on which to call callbacks.
// @param id, id of the updated camera..
void onTextureAvailableRouter(void* context, TangoCameraId id) {
  tango_augmented_reality::AugmentedRealityApp* app =
      static_cast<tango_augmented_reality::AugmentedRealityApp*>(context);
  app->onTextureAvailable(id);
}
}  // namespace

namespace tango_augmented_reality {
void AugmentedRealityApp::onTangoEventAvailable(const TangoEvent* event) {
  std::lock_guard<std::mutex> lock(tango_event_mutex_);
  tango_event_data_.UpdateTangoEvent(event);
}

void AugmentedRealityApp::onTextureAvailable(TangoCameraId id) {
  if (id == TANGO_CAMERA_COLOR) {
    RequestRender();
  }
}

AugmentedRealityApp::AugmentedRealityApp()
    : calling_activity_obj_(nullptr), on_demand_render_(nullptr) {
  is_service_connected_ = false;
  is_texture_id_set_ = false;
}

AugmentedRealityApp::~AugmentedRealityApp() { TangoConfig_free(tango_config_); }

bool AugmentedRealityApp::CheckTangoVersion(JNIEnv* env, jobject activity,
                                            int min_tango_version) {
  // Check the installed version of the TangoCore.  If it is too old, then
  // it will not support the most up to date features.
  int version;
  TangoErrorType err = TangoSupport_GetTangoVersion(env, activity, &version);
  return err == TANGO_SUCCESS && version >= min_tango_version;
}

bool AugmentedRealityApp::OnTangoServiceConnected(JNIEnv* env, jobject activity,
                                                  jobject iBinder) {
  TangoErrorType ret = TangoService_setBinder(env, iBinder);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "AugmentedRealityApp: Failed to set Tango binder with"
        "error code: %d",
        ret);
    return false;
  }

  // We want to be able to trigger rendering on demand in our Java code.
  // As such, we need to store the activity we'd like to interact with and the
  // id of the method we'd like to call on that activity.
  jclass cls = env->GetObjectClass(activity);
  on_demand_render_ = env->GetMethodID(cls, "requestRender", "()V");

  calling_activity_obj_ = env->NewGlobalRef(activity);

  is_service_connected_ = true;
  return true;
}

void AugmentedRealityApp::ActivityDestroyed() {
  JNIEnv* env;
  java_vm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
  env->DeleteGlobalRef(calling_activity_obj_);

  calling_activity_obj_ = nullptr;
  on_demand_render_ = nullptr;
}

int AugmentedRealityApp::TangoSetupConfig() {
  // Here, we'll configure the service to run in the way we'd want. For this
  // application, we'll start from the default configuration
  // (TANGO_CONFIG_DEFAULT). This enables basic motion tracking capabilities.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE("AugmentedRealityApp: Failed to get default config form");
    return TANGO_ERROR;
  }

  // Set auto-recovery for motion tracking as requested by the user.
  int ret =
      TangoConfig_setBool(tango_config_, "config_enable_auto_recovery", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "AugmentedRealityApp: config_enable_auto_recovery() failed with error"
        "code: %d",
        ret);
    return ret;
  }

  // Enable color camera from config.
  ret = TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "AugmentedRealityApp: config_enable_color_camera() failed with error"
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
        "AugmentedRealityApp: config_enable_low_latency_imu_integration() "
        "failed with error code: %d",
        ret);
    return ret;
  }

  // Get TangoCore version string from service.
  char tango_core_version[kVersionStringLength];
  ret = TangoConfig_getString(tango_config_, "tango_service_library_version",
                              tango_core_version, kVersionStringLength);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "AugmentedRealityApp: get tango core version failed with error"
        "code: %d",
        ret);
    return ret;
  }
  tango_core_version_string_ = tango_core_version;

  return ret;
}

int AugmentedRealityApp::TangoConnectCallbacks() {
  // Attach onEventAvailable callback.
  // The callback will be called after the service is connected.
  int ret = TangoService_connectOnTangoEvent(onTangoEventAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "AugmentedRealityApp: Failed to connect to event callback with error"
        "code: %d",
        ret);
    return ret;
  }

  return ret;
}

// Connect to Tango Service, service will start running, and
// pose can be queried.
bool AugmentedRealityApp::TangoConnect() {
  TangoErrorType ret = TangoService_connect(this, tango_config_);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "AugmentedRealityApp: Failed to connect to the Tango service with"
        "error code: %d",
        ret);
    return false;
  }

  // Initialize TangoSupport context.
  TangoSupport_initialize(TangoService_getPoseAtTime);

  return true;
}

void AugmentedRealityApp::TangoDisconnect() {
  // When disconnecting from the Tango Service, it is important to make sure to
  // free your configuration object. Note that disconnecting from the service,
  // resets all configuration, and disconnects all callbacks. If an application
  // resumes after disconnecting, it must re-register configuration and
  // callbacks with the service.
  TangoConfig_free(tango_config_);
  tango_config_ = nullptr;
  TangoService_disconnect();
}

void AugmentedRealityApp::TangoResetMotionTracking() {
  main_scene_.ResetTrajectory();
  TangoService_resetMotionTracking();
}

void AugmentedRealityApp::InitializeGLContent() {
  main_scene_.InitGLContent();
}

void AugmentedRealityApp::SetViewPort(int width, int height) {
  viewport_width_ = width;
  viewport_height_ = height;
}

void AugmentedRealityApp::Render() {
  if (is_service_connected_ && !is_texture_id_set_) {
    is_texture_id_set_ = true;
    // Connect color camera texture. TangoService_connectTextureId expects a
    // valid texture id from the caller, so we will need to wait until the GL
    // content is properly allocated.
    TangoErrorType ret = TangoService_connectTextureId(
        TANGO_CAMERA_COLOR, main_scene_.GetVideoOverlayTextureId(), this,
        onTextureAvailableRouter);
    if (ret != TANGO_SUCCESS) {
      LOGE(
          "AugmentedRealityApp: Failed to connect the texture id with error"
          "code: %d",
          ret);
    }

    // Query intrinsics for the color camera from the Tango Service, because we
    // want to match the virtual render camera's intrinsics to the physical
    // camera, we will compute the actually projection matrix and the view port
    // ratio for the render.
    ret = TangoService_getCameraIntrinsics(TANGO_CAMERA_COLOR,
                                           &color_camera_intrinsics_);
    if (ret != TANGO_SUCCESS) {
      LOGE(
          "AugmentedRealityApp: Failed to get camera intrinsics with error"
          "code: %d",
          ret);
    }

    float image_width = static_cast<float>(color_camera_intrinsics_.width);
    float image_height = static_cast<float>(color_camera_intrinsics_.height);
    float fx = static_cast<float>(color_camera_intrinsics_.fx);
    float fy = static_cast<float>(color_camera_intrinsics_.fy);
    float cx = static_cast<float>(color_camera_intrinsics_.cx);
    float cy = static_cast<float>(color_camera_intrinsics_.cy);

    float image_plane_ratio = image_height / image_width;
    float image_plane_distance = 2.0f * fx / image_width;

    glm::mat4 projection_mat_ar =
        tango_gl::Camera::ProjectionMatrixForCameraIntrinsics(
            image_width, image_height, fx, fy, cx, cy,
            kArCameraNearClippingPlane, kArCameraFarClippingPlane);

    main_scene_.SetFrustumScale(
        glm::vec3(1.0f, image_plane_ratio, image_plane_distance));
    main_scene_.SetCameraImagePlaneRatio(image_plane_ratio);
    main_scene_.SetImagePlaneDistance(image_plane_distance);
    main_scene_.SetARCameraProjectionMatrix(projection_mat_ar);

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
      glViewport(0, 0, viewport_height_ / image_plane_ratio, viewport_height_);
    } else {
      glViewport(0, 0, viewport_width_, viewport_width_ * image_plane_ratio);
    }
    main_scene_.SetCameraType(
        tango_gl::GestureCamera::CameraType::kFirstPerson);
  }

  double video_overlay_timestamp;
  TangoErrorType status =
      TangoService_updateTexture(TANGO_CAMERA_COLOR, &video_overlay_timestamp);

  if (status == TANGO_SUCCESS) {
    TangoDoubleMatrixTransformData matrix_transform;
    status = TangoSupport_getDoubleMatrixTransformAtTime(
        video_overlay_timestamp, TANGO_COORDINATE_FRAME_START_OF_SERVICE,
        TANGO_COORDINATE_FRAME_CAMERA_COLOR, TANGO_SUPPORT_ENGINE_OPENGL,
        TANGO_SUPPORT_ENGINE_OPENGL, ROTATION_0, &matrix_transform);
    if (matrix_transform.status_code == TANGO_POSE_VALID) {
      {
        std::lock_guard<std::mutex> lock(transform_mutex_);
        UpdateTransform(matrix_transform.matrix, video_overlay_timestamp);
      }
      main_scene_.Render(cur_start_service_T_camera_);
    } else {
      LOGE(
          "PlaneFittingApplication: Could not find a valid matrix transform at "
          "time %lf for the color camera.",
          video_overlay_timestamp);
    }
  } else {
    LOGE(
        "AugmentedRealityApp: Failed to update video overlay texture with "
        "error code: %d",
        status);
  }
}

void AugmentedRealityApp::DeleteResources() {
  is_service_connected_ = false;
  is_texture_id_set_ = false;
  main_scene_.DeleteResources();
}

std::string AugmentedRealityApp::GetTransformString() {
  std::lock_guard<std::mutex> lock(transform_mutex_);
  return transform_string_;
}

std::string AugmentedRealityApp::GetEventString() {
  std::lock_guard<std::mutex> lock(tango_event_mutex_);
  return tango_event_data_.GetTangoEventString().c_str();
}

std::string AugmentedRealityApp::GetVersionString() {
  return tango_core_version_string_.c_str();
}

void AugmentedRealityApp::SetCameraType(
    tango_gl::GestureCamera::CameraType camera_type) {
  main_scene_.SetCameraType(camera_type);
}

void AugmentedRealityApp::OnTouchEvent(
    int touch_count, tango_gl::GestureCamera::TouchEvent event, float x0,
    float y0, float x1, float y1) {
  main_scene_.OnTouchEvent(touch_count, event, x0, y0, x1, y1);
}

void AugmentedRealityApp::RequestRender() {
  if (calling_activity_obj_ == nullptr || on_demand_render_ == nullptr) {
    LOGE("Can not reference Activity to request render");
    return;
  }

  // Here, we notify the Java activity that we'd like it to trigger a render.
  JNIEnv* env;
  java_vm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
  env->CallVoidMethod(calling_activity_obj_, on_demand_render_);
}

void AugmentedRealityApp::UpdateTransform(const double transform[16],
                                          double timestamp) {
  prev_start_service_T_camera_ = cur_start_service_T_camera_;
  cur_start_service_T_camera_ = glm::make_mat4(transform);
  // Increase pose counter.
  ++transform_counter_;
  prev_timestamp_ = cur_timestamp_;
  cur_timestamp_ = timestamp;
  FormatTransformString();
}

void AugmentedRealityApp::FormatTransformString() {
  const float* transform =
      (const float*)glm::value_ptr(cur_start_service_T_camera_);
  std::stringstream string_stream;
  string_stream.setf(std::ios_base::fixed, std::ios_base::floatfield);
  string_stream.precision(3);
  string_stream << "count: " << transform_counter_
                << ", delta time (ms): " << (cur_timestamp_ - prev_timestamp_)
                << std::endl << "position (m): [" << transform[12] << ", "
                << transform[13] << ", " << transform[14] << "]" << std::endl
                << "rotation matrix: [" << transform[0] << ", " << transform[1]
                << ", " << transform[2] << ", " << transform[4] << ", "
                << transform[5] << ", " << transform[6] << ", " << transform[8]
                << ", " << transform[9] << ", " << transform[10] << "]";
  transform_string_ = string_stream.str();
  string_stream.flush();
}
}  // namespace tango_augmented_reality
