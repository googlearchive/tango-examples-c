/*
 * Copyright 2017 Google Inc. All Rights Reserved.
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
#include <thread>

#include <tango-gl/conversions.h>
#include <tango_support.h>

#include "tango-marker-detection/marker_detection_app.h"

namespace {
const int kVersionStringLength = 128;

// The minimum Tango Core version required from this application.
const int kTangoCoreMinimumVersion = 9377;

// Far clipping plane of the AR camera.
const float kArCameraNearClippingPlane = 0.1f;
const float kArCameraFarClippingPlane = 100.0f;

// Marker detecting frequency, in frames per second.
const int kMarkerDetectionFPS = 30;

// This function routes texture callbacks to the application object for
// handling.
//
// @param context, context will be a pointer to a MarkerDetectionApp
//        instance on which to call callbacks.
// @param id, id of the updated camera.
void onTextureAvailableRouter(void* context, TangoCameraId id) {
  tango_marker_detection::MarkerDetectionApp* app =
      static_cast<tango_marker_detection::MarkerDetectionApp*>(context);
  app->onTextureAvailable(id);
}

// This function routes color image frame callbacks to the application object
// for handling.
//
// @param context, context will be a pointer to a MarkerDetectionApp
//        instance on which to call callbacks.
// @param id, id of the updated camera.
// @param buffer, the image buffer.
void OnFrameAvailableRouter(void* context, TangoCameraId,
                            const TangoImageBuffer* buffer) {
  tango_marker_detection::MarkerDetectionApp* app =
      static_cast<tango_marker_detection::MarkerDetectionApp*>(context);
  app->OnFrameAvailable(buffer);
}
}  // namespace

namespace tango_marker_detection {
void MarkerDetectionApp::onTextureAvailable(TangoCameraId id) {
  if (id == TANGO_CAMERA_COLOR) {
    RequestRender();
  }
}

void MarkerDetectionApp::OnFrameAvailable(const TangoImageBuffer* buffer) {
  if (image_buffer_manager_ == nullptr) {
    return;
  }
  TangoSupport_updateImageBuffer(image_buffer_manager_, buffer);
}

void MarkerDetectionApp::OnCreate(JNIEnv* env, jobject activity,
                                  int display_rotation) {
  // Check the installed version of the TangoCore against minimum required
  // Tango version.
  int version;
  TangoErrorType err = TangoSupport_getTangoVersion(env, activity, &version);
  if (err != TANGO_SUCCESS || version < kTangoCoreMinimumVersion) {
    LOGE("MarkerDetectionApp: Tango Core version is out of date.");
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

  display_rotation_ = static_cast<TangoSupport_Rotation>(display_rotation);
  is_video_overlay_rotation_dirty_ = true;
  prev_marker_detection_timestamp_ = 0;
}

void MarkerDetectionApp::OnTangoServiceConnected(JNIEnv* env, jobject iBinder) {
  TangoErrorType ret = TangoService_setBinder(env, iBinder);
  if (ret != TANGO_SUCCESS) {
    LOGE("MarkerDetectionApp: Failed to set Tango binder with error code: %d",
         ret);
    std::exit(EXIT_SUCCESS);
  }

  TangoSetupConfig();
  TangoConnectCallbacks();
  TangoConnect();
  SetupIntrinsics();
  SetupImageBufferManager();

  is_service_connected_ = true;
  UpdateViewportAndProjectionMatrix();
}

void MarkerDetectionApp::OnDestroy() {
  JNIEnv* env;
  java_vm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
  env->DeleteGlobalRef(calling_activity_obj_);

  calling_activity_obj_ = nullptr;
  on_demand_render_ = nullptr;
}

void MarkerDetectionApp::TangoSetupConfig() {
  // Here, we'll configure the service to run in the way we'd want. For this
  // application, we'll start from the default configuration
  // (TANGO_CONFIG_DEFAULT). This enables basic motion tracking capabilities.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    LOGE("MarkerDetectionApp: Failed to get default config form");
    std::exit(EXIT_SUCCESS);
  }

  // Set auto-recovery for motion tracking.
  int ret =
      TangoConfig_setBool(tango_config_, "config_enable_auto_recovery", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MarkerDetectionApp: config_enable_auto_recovery() failed with error"
        "code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  // Enable color camera from config.
  ret = TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MarkerDetectionApp: config_enable_color_camera() failed with error"
        "code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  // Low latency IMU integration enables aggressive integration of the latest
  // inertial measurements to provide lower latency pose estimates. This will
  // improve the AR experience.
  ret = TangoConfig_setBool(tango_config_,
                            "config_enable_low_latency_imu_integration", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MarkerDetectionApp: config_enable_low_latency_imu_integration() "
        "failed with error code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  // Drift correction allows motion tracking to recover after it loses tracking.
  ret = TangoConfig_setBool(tango_config_, "config_enable_drift_correction",
                            true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MarkerDetectionApp: enabling config_enable_drift_correction "
        "failed with error code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }
}

void MarkerDetectionApp::TangoConnectCallbacks() {
  // Register color camera texture callback. Textures from color camera are
  // used for realtime video.
  TangoErrorType ret = TangoService_connectOnTextureAvailable(
      TANGO_CAMERA_COLOR, this, onTextureAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MarkerDetectionApp: Failed to connect texture callback with error"
        "code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  // Register for color frame callback as we'll need color images for
  // marker detection.
  ret = TangoService_connectOnFrameAvailable(TANGO_CAMERA_COLOR, this,
                                             OnFrameAvailableRouter);

  if (ret != TANGO_SUCCESS) {
    LOGE("MarkerDetectionApp: Error connecting to camera frame %d", ret);
    std::exit(EXIT_SUCCESS);
  }
}

// Connect to Tango Service.
void MarkerDetectionApp::TangoConnect() {
  TangoErrorType ret = TangoService_connect(this, tango_config_);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MarkerDetectionApp: Failed to connect to the Tango service with"
        "error code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  // Initialize TangoSupport context.
  TangoSupport_initialize(TangoService_getPoseAtTime,
                          TangoService_getCameraIntrinsics);
}

TangoErrorType MarkerDetectionApp::SetupIntrinsics() {
  TangoErrorType ret = TangoService_getCameraIntrinsics(
      TANGO_CAMERA_COLOR, &color_camera_intrinsics_);

  if (ret != TANGO_SUCCESS) {
    LOGE(
        "ARMarkerApplication: Failed to get camera intrinsics with error"
        "code: %d",
        ret);
  }

  return ret;
}

// Image buffer manager helps to cache image buffers in background.
TangoErrorType MarkerDetectionApp::SetupImageBufferManager() {
  if (image_buffer_manager_ == nullptr) {
    TangoErrorType status = TangoSupport_createImageBufferManager(
        TANGO_HAL_PIXEL_FORMAT_YCrCb_420_SP, color_camera_intrinsics_.width,
        color_camera_intrinsics_.height, &image_buffer_manager_);
    if (status != TANGO_SUCCESS) {
      LOGE(
          "MarkerDetectionApp: Failed create image buffer manager "
          "with error code: %d",
          status);
      return status;
    }
  }
  return TANGO_SUCCESS;
}

void MarkerDetectionApp::OnPause() {
  TangoDisconnect();
  DeleteResources();
}

void MarkerDetectionApp::TangoDisconnect() {
  is_service_connected_ = false;
  is_gl_initialized_ = false;
  is_video_overlay_rotation_dirty_ = true;

  // When disconnecting from the Tango Service, it is important to make sure to
  // free the configuration object. Note that disconnecting from the service,
  // resets all configuration, and disconnects all callbacks. If an application
  // resumes after disconnecting, it needs to re-register configuration and
  // callbacks.
  TangoConfig_free(tango_config_);
  tango_config_ = nullptr;
  TangoService_disconnect();
}

void MarkerDetectionApp::OnSurfaceCreated(AAssetManager* aasset_manager) {
  main_scene_.InitGLContent(aasset_manager);
  is_gl_initialized_ = true;
  UpdateViewportAndProjectionMatrix();
}

void MarkerDetectionApp::OnSurfaceChanged(int width, int height) {
  viewport_width_ = width;
  viewport_height_ = height;

  UpdateViewportAndProjectionMatrix();
}

void MarkerDetectionApp::UpdateViewportAndProjectionMatrix() {
  if (!is_service_connected_ || !is_gl_initialized_) {
    return;
  }
  // Query intrinsics for the color camera from the Tango Service and compute
  // the actually projection matrix and the view port ratio.
  float image_plane_ratio = 0.0f;

  // Use rotated camera intrinsics to generate projection matrix.
  TangoCameraIntrinsics rotated_camera_intrinsics;
  int ret = TangoSupport_getCameraIntrinsicsBasedOnDisplayRotation(
      TANGO_CAMERA_COLOR, display_rotation_, &rotated_camera_intrinsics);

  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MarkerDetectionApp: Failed to get camera intrinsics with error code: "
        "%d",
        ret);
    return;
  }

  float image_width = static_cast<float>(rotated_camera_intrinsics.width);
  float image_height = static_cast<float>(rotated_camera_intrinsics.height);
  float fx = static_cast<float>(rotated_camera_intrinsics.fx);
  float fy = static_cast<float>(rotated_camera_intrinsics.fy);
  float cx = static_cast<float>(rotated_camera_intrinsics.cx);
  float cy = static_cast<float>(rotated_camera_intrinsics.cy);

  ar_camera_projection_matrix_ =
      tango_gl::Camera::ProjectionMatrixForCameraIntrinsics(
          image_width, image_height, fx, fy, cx, cy, kArCameraNearClippingPlane,
          kArCameraFarClippingPlane);
  image_plane_ratio = image_height / image_width;

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

void MarkerDetectionApp::OnDeviceRotationChanged(int display_rotation) {
  display_rotation_ = static_cast<TangoSupport_Rotation>(display_rotation);
  is_video_overlay_rotation_dirty_ = true;
}

void MarkerDetectionApp::OnDrawFrame() {
  main_scene_.Clear();

  // If tracking is lost, no content will be rendered. This is to prevent
  // flickering that would otherwise happen.
  if (!is_gl_initialized_ || !is_service_connected_) {
    return;
  }

  if (is_video_overlay_rotation_dirty_) {
    main_scene_.SetVideoOverlayRotation(display_rotation_);
    is_video_overlay_rotation_dirty_ = false;
  }

  // Update video texture.
  double video_overlay_timestamp;
  TangoErrorType status = TangoService_updateTextureExternalOes(
      TANGO_CAMERA_COLOR, main_scene_.GetVideoOverlayTextureId(),
      &video_overlay_timestamp);

  if (status != TANGO_SUCCESS) {
    LOGE("MarkerDetectionApp: Failed to update video overlay texture: %d",
         status);
    return;
  }

  // Get AR camera transformation matrix.
  glm::mat4 ar_camera_transformation_matrix;
  status = GetARCameraTransformationMatrix(video_overlay_timestamp,
                                           display_rotation_,
                                           &ar_camera_transformation_matrix);
  if (status != TANGO_SUCCESS) {
    LOGE("MarkerDetectionApp: Failed to get camera transformation: %d", status);
    return;
  }
  main_scene_.SetupCamera(ar_camera_projection_matrix_,
                          ar_camera_transformation_matrix);

  // Detect markers in secondary thread.
  DetectMarkers(video_overlay_timestamp);

  // Render video overlay, and objects.
  main_scene_.Render();
}

TangoErrorType MarkerDetectionApp::GetARCameraTransformationMatrix(
    double timestamp, TangoSupport_Rotation display_rotation,
    glm::mat4* ar_camera_transformation_matrix) {
  TangoSupport_MatrixTransformData matrix;
  TangoErrorType status = TangoSupport_getMatrixTransformAtTime(
      timestamp, TANGO_COORDINATE_FRAME_START_OF_SERVICE,
      TANGO_COORDINATE_FRAME_CAMERA_COLOR, TANGO_SUPPORT_ENGINE_OPENGL,
      TANGO_SUPPORT_ENGINE_OPENGL, display_rotation, &matrix);

  if (matrix.status_code != TANGO_POSE_VALID) {
    return status;
  }

  *ar_camera_transformation_matrix = glm::make_mat4(matrix.matrix);

  return TANGO_SUCCESS;
}

TangoErrorType MarkerDetectionApp::GetColorCameraExtrinsics(
    double timestamp, glm::mat4* camera_extrinsics_matrix) {
  // When drift correction mode is enabled, we need to query the device with
  // respect to Start of Service pose in order to use the drift corrected pose.
  TangoSupport_MatrixTransformData matrix;
  TangoErrorType status = TangoSupport_getMatrixTransformAtTime(
      timestamp, TANGO_COORDINATE_FRAME_START_OF_SERVICE,
      TANGO_COORDINATE_FRAME_CAMERA_COLOR, TANGO_SUPPORT_ENGINE_OPENGL,
      TANGO_SUPPORT_ENGINE_TANGO, TANGO_SUPPORT_ROTATION_IGNORED, &matrix);
  if (matrix.status_code != TANGO_POSE_VALID) {
    return status;
  }

  *camera_extrinsics_matrix = glm::make_mat4(matrix.matrix);

  return TANGO_SUCCESS;
}

void MarkerDetectionApp::DetectMarkers(double timestamp) {
  // Marker detection is a time-consuming process. This is to make sure marker
  // detection process runs at a frequency no higher than a pre-defined FPS.
  if (timestamp < prev_marker_detection_timestamp_ + 1.0 / kMarkerDetectionFPS)
    return;

  prev_marker_detection_timestamp_ = timestamp;

  // Start the secondary thread
  std::thread t([this]() {
    if (marker_detection_thread_mutex_.try_lock()) {
      // Get latest image buffer.
      TangoImageBuffer* image_buffer = nullptr;
      TangoErrorType status = TangoSupport_getLatestImageBuffer(
          image_buffer_manager_, &image_buffer);
      if (status == TANGO_SUCCESS) {
        // Get camera extrinsics matrix.
        glm::mat4 camera_extrinsics_matrix;
        status = GetColorCameraExtrinsics(image_buffer->timestamp,
                                          &camera_extrinsics_matrix);
        if (status == TANGO_SUCCESS) {
          main_scene_.DetectMarkers(*image_buffer, camera_extrinsics_matrix);
        }
      }
      marker_detection_thread_mutex_.unlock();
    }
  });

  // Let the thread run independently from the rendering thread.
  t.detach();
}

void MarkerDetectionApp::DeleteResources() {
  main_scene_.DeleteResources();
  is_gl_initialized_ = false;
}

void MarkerDetectionApp::RequestRender() {
  if (calling_activity_obj_ == nullptr || on_demand_render_ == nullptr) {
    LOGE("MarkerDetectionApp: Can not reference Activity to request render");
    return;
  }

  // Here, we notify the Java activity that we'd like it to trigger a render.
  JNIEnv* env;
  java_vm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
  env->CallVoidMethod(calling_activity_obj_, on_demand_render_);
}
}  // namespace tango_marker_detection
