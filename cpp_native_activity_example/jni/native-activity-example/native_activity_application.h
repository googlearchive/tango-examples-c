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

#ifndef NATIVE_ACTIVITY_EXAMPLE_NATIVE_ACTIVITY_APPLICATION_H_
#define NATIVE_ACTIVITY_EXAMPLE_NATIVE_ACTIVITY_APPLICATION_H_

#include <memory>
#include <mutex>

#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <tango_client_api.h>
#include <unistd.h>

#include "tango-gl/color.h"
#include "tango-gl/conversions.h"
#include "tango-gl/cube.h"
#include "tango-gl/frustum.h"
#include "tango-gl/grid.h"
#include "tango-gl/util.h"
#include "tango-gl/video_overlay.h"
/**
 * This class is the main application for NativeActivity. The class manages the
 * application's lifecycle and interaction with the Tango service. Primarily,
 * this involves registering for callbacks and passing on the necessary
 * information to stored objects.
 *
 * This example application currently stores shared state for our app, and
 * does not have any callbacks.
 */
class NativeActivityApplication {
 public:
  NativeActivityApplication();
  ~NativeActivityApplication();

  /**
   * @brief Tear down the EGL context currently associated with the display.
   */
  void EngineTermDisplay();

  /**
   * @return true if the display is visible.
   */
  bool IsVisible();

  /**
   * @brief Initialize renderables, connect to Tango service, set up display and
   * start rendering.
   */
  bool OnSurfaceCreated();

  /**
   * @brief Render display.
   */
  void Render();

  /**
   * @brief Set /ref app_ to point to the current android application.
   */
  void SetAppState(struct android_app* state);

  /**
   * @brief Connect to Tango service.
   */
  bool TangoConnect();

  /**
   * @brief Disconnect Tango service.
   */
  void TangoDisconnect();

  /**
   * @brief Check that the installed version of the Tango API is up to date.
   */
  bool CheckTangoVersion(JNIEnv* env, jobject activity, int min_tango_version);

  /**
   * @brief Set up Tango service configuration.
   */
  bool TangoSetupConfig();

  /**
   * @brief Disconnect Tango service and clean up the data.
   */
  void Terminate();

 private:
  /* Store the android activity pointer from /ref android_main. */
  struct android_app* app_;

  /* Store the Tango configuration. */
  TangoConfig tango_config_;

  /* Store the visibility of the display. */
  bool visible_;

  /* Store EGL context attributes for display. */
  EGLDisplay display_;
  EGLContext context_;
  EGLSurface surface_;
  int32_t width_;
  int32_t height_;

  /* Store renderables. */
  std::unique_ptr<tango_gl::Grid> grid_;
  std::unique_ptr<tango_gl::Cube> cube_;
  std::unique_ptr<tango_gl::VideoOverlay> video_overlay_;

  /* The current position that contains transformation from start of service to
   * device. */
  TangoPoseData tango_pose_;
  /* This pair is used to specify Tango coordinate frames when it is used by
   * /ref TangoService_getPoseAtTime. */
  TangoCoordinateFramePair start_service_to_device_frame_pair_;

  /* The image plane ratio.
   * This is set by /ref SetupIntrinsics, and used by /ref SetupViewport. */
  float image_plane_ratio_;

  /* The augmented reality projection matrix.
   * This is set by /ref SetupIntrinsics, and used by /ref Render. */
  glm::mat4 projection_mat_ar_;

  /* These are used to update view matrix for rendering. */
  /* device with respect to IMU. Set by /ref SetupExtrinsics. */
  glm::mat4 imu_T_device_;
  /* color camera with respect to IMU. Set by /ref SetupExtrinsics. */
  glm::mat4 imu_T_color_camera_;
  /* Start of service with respect to OpenGL world. */
  glm::mat4 opengl_world_T_start_service_;
  /* OpenGL camera with respect to Color camera. */
  glm::mat4 color_camera_T_opengl_camera_;

  /**
   * @brief Enable color camera in tango configuration.
   * @return return true if "config_enable_color_camera" is successfully set to
   * true in /ref tango_config.
   */
  bool EnableCamera(TangoConfig* tango_config);

  /**
   * @brief Get transformation from start of service to device and output it
   * through /ref tango_pose at the /ref image_timestamp.
   * @return true if /ref tango_pose is updated successfully.
   */
  bool GetPoseAtTime(const double image_timestamp, TangoPoseData* tango_pose);

  /**
   * @brief Initialize an EGL context for the current display.
   * Reference:
   * http://developer.android.com/reference/android/app/NativeActivity.html
   */
  bool InitializeDisplay();

  /* Helper functions for InitializeDisplay. */
  void InitializeCube();
  void InitializeGrid();

  /**
   * Set up renderables.
   */
  void SetUpDisplay();

  /**
   * Get transformations for IMU to Device(/ref imu_T_device_) and IMU to color
   * camera(/ref imu_T_cam_) from extrinsics.
   */
  bool SetUpExtrinsics();

  /**
   * Get /ref image_plane_ratio_ and /ref projection_matr_ar_ from intrinsics.
   */
  bool SetUpIntrinsics();

  /* Helper functions for /ref SetupIntrinsics. */
  void SetImagePlaneRatio(const TangoCameraIntrinsics& ccIntrinsics);
  void SetProjectionMatAr(const TangoCameraIntrinsics& ccIntrinsics);

  /**
   * Set up the correct viewport.
   */
  void SetUpViewport();

  /**
   * @brief Set visibility of the display.
   */
  void SetVisible(bool visible);

  /**
   * @brief Connect color camera texture with /ref video_overlay_ through the
   * texture ID.
   */
  bool TangoConnectTexture();

  /**
   * @brief Update /ref video_overlay_ texture, and update /ref tango_pose_ if
   * the texture update is successful.
   */
  bool UpdateTextureAndPose();

  /**
   * @brief Get /ref view_matrix based on the /ref tango_pose_, IMU to color
   * camera, and Start of service with respect to OpenGL world transfroms.
   */
  void UpdateViewMatrix(glm::mat4* view_matrix);
};
#endif  // NATIVE_ACTIVITY_EXAMPLE_NATIVE_ACTIVITY_APPLICATION_H_
