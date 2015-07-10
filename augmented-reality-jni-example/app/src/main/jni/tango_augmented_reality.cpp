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

#include <jni.h>
#include <string>

#include "tango-gl/axis.h"
#include "tango-gl/camera.h"
#include "tango-gl/color.h"
#include "tango-gl/conversions.h"
#include "tango-gl/cube.h"
#include "tango-gl/frustum.h"
#include "tango-gl/grid.h"
#include "tango-gl/goal_marker.h"
#include "tango-gl/trace.h"
#include "tango-gl/util.h"
#include "tango-gl/video_overlay.h"

#include "tango_data.h"

// Render camera's parent transformation.
// This object is a pivot transformtion for render camera to rotate around.
tango_gl::Transform* cam_parent_transform;

// Render camera.
tango_gl::Camera* cam;

// Coordinate axis for display.
tango_gl::Axis* axis;

// Color Camera frustum.
tango_gl::Frustum* frustum;

// Ground grid.
tango_gl::Grid* ground;

// Trace of pose data.
tango_gl::Trace* trace;

tango_gl::GoalMarker* marker;

// Color camera preview.
tango_gl::VideoOverlay* video_overlay;

// Single finger touch positional values.
// First element in the array is x-axis touching position.
// Second element in the array is y-axis touching position.
float cam_start_angle[2];
float cam_cur_angle[2];

// Double finger touch distance value.
float cam_start_dist;
float cam_cur_dist;

enum CameraType {
  FIRST_PERSON = 0,
  THIRD_PERSON = 1,
  TOP_DOWN = 2
};
CameraType camera_type;

// Render camera observation distance in third person camera mode.
const float kThirdPersonCameraDist = 7.0f;

// Render camera observation distance in top down camera mode.
const float kTopDownCameraDist = 5.0f;

// Zoom in speed.
const float kZoomSpeed = 10.0f;

// Min/max clamp value of camera observation distance.
const float kCamViewMinDist = 1.0f;
const float kCamViewMaxDist = 100.f;

// FOV set up values.
// Third and top down camera's FOV is 65 degrees.
// First person is color camera's FOV.
const float kFov = 65.0f;

// Scale frustum size for closer near clipping plane.
const float kFovScaler = 0.1f;

// Increment value each time move AR elements.
const float kArElementIncrement = 0.05f;

const float kZero = 0.0f;
const glm::vec3 kZeroVec3 = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::quat kZeroQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

// AR grid rotation, 90 degrees around x axis.
const glm::quat kArGridRotation = glm::quat(0.70711f, -0.70711f, 0.0f, 0.0f);
const glm::quat kMarkerRotation = glm::quat(0.f, 0.f, 1.0f, 0.f);
const glm::vec3 kMarkerPosition = glm::vec3(0.0f, 0.85f, -3.0f);
const glm::vec3 kMarkerOffset = glm::vec3(0.0f, 0.85f, 0.0f);

// Color of the ground grid.
const tango_gl::Color kGridColor(0.85f, 0.85f, 0.85f);
// Color of the goal marker.
const tango_gl::Color kMarkerColor(1.0f, 0.f, 0.f);

// AR cube position in world coordinate.
const glm::vec3 kCubePosition = glm::vec3(-1.0f, 0.265f, -2.0f);

// AR grid position, can be modified based on the real world scene.
const glm::vec3 kGridPosition = glm::vec3(0.0f, 1.26f, -2.0f);

// AR cube dimension, based on real world scene.
const glm::vec3 kCubeScale = glm::vec3(0.38f, 0.53f, 0.57f);

// Marker scale.
const glm::vec3 kMarkerScale = glm::vec3(0.05f, 0.05f, 0.05f);

// Height offset is used for offset height of motion tracking
// pose data. Motion tracking start position is (0,0,0). Adding
// a height offset will give a more reasonable pose while a common
// human is holding the device. The units is in meters.
const glm::vec3 kFloorOffset = glm::vec3(0.0f, -1.4f, 0.0f);
glm::vec3 world_position = glm::vec3(0.0f, -1.4f, 0.0f);

// Position and rotation of the opengl camera with respect to the opengl world.
// (This is the opengl representation of the physical color camera's location.)
glm::vec3 ow_p_oc;
glm::quat ow_q_oc;

// Projection matrix from render camera.
glm::mat4 projection_mat;

// First person projection matrix from color camera intrinsics.
glm::mat4 projection_mat_ar;

// First person view matrix from color camera extrinsics.
glm::mat4 view_mat;

// Tango start-of-service with respect to Opengl World matrix.
glm::mat4 ow_T_ss;

// Device with respect to IMU matrix.
glm::mat4 imu_T_device;

// Color Camera with respect to IMU matrix.
glm::mat4 imu_T_cc;

// Opengl Camera with respect to Color Camera matrix.
glm::mat4 cc_T_oc;

// Opengl Camera with respect to Opengl World matrix.
glm::mat4 ow_T_oc;

// Color Camera image plane ratio.
float image_plane_ratio;
float image_width;
float image_height;

// Color Camera image plane distance to view point.
float image_plane_dis;
float image_plane_dis_original;

void SetupExtrinsics() {
  TangoData& instance = TangoData::GetInstance();
  imu_T_device = glm::translate(glm::mat4(1.0f), instance.imu_p_device) *
                 glm::mat4_cast(instance.imu_q_device);

  imu_T_cc = glm::translate(glm::mat4(1.0f), instance.imu_p_cc) *
             glm::mat4_cast(instance.imu_q_cc);
}

// Setup projection matrix in first person view from color camera intrinsics.
void SetupIntrinsics() {
  image_width = static_cast<float>(TangoData::GetInstance().cc_width);
  image_height = static_cast<float>(TangoData::GetInstance().cc_height);
  // Image plane focal length for x axis.
  float img_fl = static_cast<float>(TangoData::GetInstance().cc_fx);
  image_plane_ratio = image_height / image_width;
  image_plane_dis_original = 2.0f * img_fl / image_width;
  image_plane_dis = image_plane_dis_original;
  projection_mat_ar = glm::frustum(
      -1.0f * kFovScaler, 1.0f * kFovScaler, -image_plane_ratio * kFovScaler,
      image_plane_ratio * kFovScaler, image_plane_dis * kFovScaler,
      kCamViewMaxDist);
  frustum->SetScale(glm::vec3(1.0f, image_plane_ratio, image_plane_dis));
}

void SetCamera(CameraType camera_index) {
  camera_type = camera_index;
  cam_cur_angle[0] = cam_cur_angle[1] = cam_cur_dist = kZero;
  switch (camera_index) {
    case CameraType::FIRST_PERSON:
      cam_parent_transform->SetPosition(kZeroVec3);
      cam_parent_transform->SetRotation(kZeroQuat);
      video_overlay->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
      video_overlay->SetPosition(kZeroVec3);
      video_overlay->SetRotation(kZeroQuat);
      video_overlay->SetParent(NULL);
      break;
    case CameraType::THIRD_PERSON:
      video_overlay->SetParent(axis);
      video_overlay->SetScale(glm::vec3(1.0f, image_plane_ratio, 1.0f));
      video_overlay->SetRotation(kZeroQuat);
      video_overlay->SetPosition(glm::vec3(kZero, kZero, -image_plane_dis));

      cam->SetPosition(kZeroVec3);
      cam->SetRotation(kZeroQuat);
      cam_cur_dist = kThirdPersonCameraDist;
      cam_cur_angle[0] = -M_PI / 4.0f;
      cam_cur_angle[1] = M_PI / 4.0f;
      break;
    case CameraType::TOP_DOWN:
      video_overlay->SetScale(glm::vec3(1.0f, image_plane_ratio, 1.0f));
      video_overlay->SetRotation(kZeroQuat);
      video_overlay->SetPosition(glm::vec3(kZero, kZero, -image_plane_dis));
      video_overlay->SetParent(axis);

      cam->SetPosition(kZeroVec3);
      cam->SetRotation(kZeroQuat);
      cam_cur_dist = kTopDownCameraDist;
      cam_cur_angle[1] = M_PI / 2.0f;
      break;
    default:
      break;
  }
}

bool SetupGraphics() {
  cam_parent_transform = new tango_gl::Transform();
  cam = new tango_gl::Camera();
  axis = new tango_gl::Axis();
  frustum = new tango_gl::Frustum();
  trace = new tango_gl::Trace();
  ground = new tango_gl::Grid(1.0f, 10, 10);
  marker = new tango_gl::GoalMarker();
  video_overlay = new tango_gl::VideoOverlay();

  cam->SetParent(cam_parent_transform);
  cam->SetFieldOfView(kFov);
  SetCamera(CameraType::FIRST_PERSON);

  ow_T_ss = tango_gl::conversions::opengl_world_T_tango_world();
  cc_T_oc = tango_gl::conversions::color_camera_T_opengl_camera();

  ground->SetPosition(world_position);
  ground->SetColor(kGridColor);

  marker->SetPosition(kMarkerPosition + world_position);
  marker->SetScale(kMarkerScale);
  marker->SetRotation(kMarkerRotation);
  marker->SetColor(kMarkerColor);
  return true;
}

// Update viewport according to surface dimensions, always use image plane's
// ratio and make full use of the screen.
void SetupViewport(int w, int h) {
  float screen_ratio = static_cast<float>(h) / static_cast<float>(w);
  if (image_plane_ratio < screen_ratio) {
    glViewport(0, 0, w, w * image_plane_ratio);
  } else {
    glViewport((w - h / image_plane_ratio) / 2, 0, h / image_plane_ratio, h);
  }
  cam->SetAspectRatio(1.0f / screen_ratio);
}

// Render AR elements, frustum and trace with current Color Camera position and
// rotation
// updated from TangoData, TangoPosition and TangoRotation is updated via
// polling function GetPoseAtTime().
bool RenderFrame() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  TangoData::GetInstance().UpdateColorTexture();
  TangoData::GetInstance().GetPoseAtTime();

  glm::vec3 ss_p_device = TangoData::GetInstance().tango_position;
  glm::quat ss_q_device = TangoData::GetInstance().tango_rotation;
  glm::mat4 ss_T_device = glm::translate(glm::mat4(1.0f), ss_p_device) *
                          glm::mat4_cast(ss_q_device);
  ow_T_oc =
      ow_T_ss * ss_T_device * glm::inverse(imu_T_device) * imu_T_cc * cc_T_oc;
  glm::vec3 scale;
  tango_gl::util::DecomposeMatrix(ow_T_oc, ow_p_oc, ow_q_oc, scale);

  if (camera_type == CameraType::FIRST_PERSON) {
    glDisable(GL_DEPTH_TEST);
    video_overlay->Render(glm::mat4(1.0f), glm::mat4(1.0f));
    glEnable(GL_DEPTH_TEST);
    projection_mat = projection_mat_ar;
    view_mat = glm::inverse(ow_T_oc);
  } else {
    glm::quat parent_cam_rot = glm::rotate(kZeroQuat, -cam_cur_angle[0],
                                           glm::vec3(kZero, 1.0f, kZero));
    parent_cam_rot = glm::rotate(parent_cam_rot, -cam_cur_angle[1],
                                 glm::vec3(1.0f, kZero, kZero));

    cam_parent_transform->SetRotation(parent_cam_rot);
    cam_parent_transform->SetPosition(tango_gl::conversions::Vec3TangoToGl(ss_p_device));

    cam->SetPosition(glm::vec3(kZero, kZero, cam_cur_dist));

    projection_mat = cam->GetProjectionMatrix();
    view_mat = cam->GetViewMatrix();

    frustum->SetPosition(ow_p_oc);
    frustum->SetRotation(ow_q_oc);
    frustum->Render(projection_mat, view_mat);

    trace->UpdateVertexArray(ow_p_oc);
    trace->Render(projection_mat, view_mat);

    axis->SetPosition(ow_p_oc);
    axis->SetRotation(ow_q_oc);
    axis->Render(projection_mat, view_mat);

    video_overlay->Render(projection_mat, view_mat);
  }

  ground->Render(projection_mat, view_mat);
  marker->SetRotation(kZeroQuat);
  marker->Render(projection_mat, view_mat);
  marker->SetRotation(kMarkerRotation);
  marker->Render(projection_mat, view_mat);
  return true;
}

// Reset virtual world, use the current color camera's position as origin.
void ResetAR() {
  world_position = ow_p_oc + kFloorOffset;
  ground->SetPosition(world_position);
  marker->SetPosition(world_position + kMarkerPosition);
  image_plane_dis = image_plane_dis_original;
  projection_mat_ar = glm::frustum(
      -1.0f * kFovScaler, 1.0f * kFovScaler, -image_plane_ratio * kFovScaler,
      image_plane_ratio * kFovScaler, image_plane_dis * kFovScaler,
      kCamViewMaxDist);
  frustum->SetScale(glm::vec3(1.0f, image_plane_ratio, image_plane_dis));
  SetCamera(camera_type);

  trace->ClearVertexArray();
}

void PlaceObject() {
  marker->SetPosition(world_position + ow_p_oc + kMarkerOffset);
}

void UpdateARElement(int ar_element, int interaction_type) {
  glm::vec3 translation;
  // LOGI("%d %d", ar_element,(interaction_type - 1) / 2 );
  switch ((interaction_type - 1) / 2) {
    case 0:
      translation =
          glm::vec3(glm::pow(-1.0f, static_cast<float>(interaction_type)) *
                        kArElementIncrement,
                    kZero, kZero);
      break;
    case 1:
      translation = glm::vec3(
          kZero, glm::pow(-1.0f, static_cast<float>(interaction_type)) *
                     kArElementIncrement,
          kZero);
      break;
    case 2:
      translation = glm::vec3(
          kZero, kZero, glm::pow(-1.0f, static_cast<float>(interaction_type)) *
                            kArElementIncrement);
      break;
  }
  switch (ar_element) {
    case 1:
      ground->Translate(translation);
      marker->Translate(translation);
      break;
    case 2:
      marker->Translate(translation);
      break;
    case 3:
      ground->Translate(translation);
      break;
    case 4:
      if (interaction_type == 3) {
        image_plane_dis += -kArElementIncrement;
      }
      if (interaction_type == 4) {
        image_plane_dis += kArElementIncrement;
      }
      projection_mat_ar = glm::frustum(
          -1.0f * kFovScaler, 1.0f * kFovScaler,
          -image_plane_ratio * kFovScaler, image_plane_ratio * kFovScaler,
          image_plane_dis * kFovScaler, kCamViewMaxDist);
      frustum->SetScale(glm::vec3(1.0f, image_plane_ratio, image_plane_dis));
      SetCamera(camera_type);
      break;
  }
}

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_initialize(
    JNIEnv* env, jobject, jobject activity) {
  TangoErrorType err = TangoData::GetInstance().Initialize(env, activity);
  if (err != TANGO_SUCCESS) {
    if (err == TANGO_INVALID) {
      LOGE("Tango Service version mis-match");
    } else {
      LOGE("Tango Service initialize internal error");
    }
  }
  if (err == TANGO_SUCCESS) {
    LOGI("Tango service initialize success");
  }
  return static_cast<int>(err);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_setupConfig(
    JNIEnv*, jobject, bool is_auto_recovery) {
  if (!TangoData::GetInstance().SetConfig(is_auto_recovery)) {
    LOGE("Tango set config failed");
  } else {
    LOGI("Tango set config success");
  }
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_connectTexture(
    JNIEnv*, jobject) {
  TangoData::GetInstance().ConnectTexture(video_overlay->GetTextureId());
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_connectService(
    JNIEnv*, jobject) {
  TangoErrorType err = TangoData::GetInstance().Connect();
  if (err == TANGO_SUCCESS) {
    TangoData::GetInstance().GetExtrinsics();
    TangoData::GetInstance().GetIntrinsics();
    SetupExtrinsics();
    SetupIntrinsics();
    LOGI("Tango Service connect success");
  } else {
    LOGE("Tango Service connect failed");
  }
  return static_cast<int>(err);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_setupViewport(
    JNIEnv*, jobject, jint width, jint height) {
  SetupViewport(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_disconnectService(
    JNIEnv*, jobject) {
  TangoData::GetInstance().Disconnect();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_onDestroy(
    JNIEnv*, jobject) {
  delete cam;
  delete axis;
  delete ground;
  delete frustum;
  delete trace;
  delete video_overlay;
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_setupGraphic(
    JNIEnv*, jobject) {
  SetupGraphics();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_render(
    JNIEnv*, jobject) {
  RenderFrame();
}

JNIEXPORT jboolean JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_getIsLocalized(
    JNIEnv*, jobject) {
  pthread_mutex_lock(&TangoData::GetInstance().pose_mutex);
  bool is_localized = TangoData::GetInstance().is_localized;
  pthread_mutex_unlock(&TangoData::GetInstance().pose_mutex);
  return is_localized;
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_resetMotionTracking(
    JNIEnv*, jobject) {
  ResetAR();
  // TangoData::GetInstance().ResetMotionTracking();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_setCamera(
    JNIEnv*, jobject, int camera_index) {
  SetCamera(static_cast<CameraType>(camera_index));
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_placeObject(
    JNIEnv*, jobject) {
  PlaceObject();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_updateARElement(
    JNIEnv*, jobject, int ar_element, int interaction_type) {
  UpdateARElement(ar_element, interaction_type);
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_getPoseString(
    JNIEnv* env, jobject) {
  pthread_mutex_lock(&TangoData::GetInstance().pose_mutex);
  std::string pose_string_cpy =
      std::string(TangoData::GetInstance().pose_string);
  pthread_mutex_unlock(&TangoData::GetInstance().pose_mutex);
  return (env)->NewStringUTF(pose_string_cpy.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_getVersionNumber(
    JNIEnv* env, jobject) {
  return (env)
      ->NewStringUTF(TangoData::GetInstance().lib_version_string.c_str());
}

// Touching GL interface.
JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_startSetCameraOffset(
    JNIEnv*, jobject) {
  cam_start_angle[0] = cam_cur_angle[0];
  cam_start_angle[1] = cam_cur_angle[1];
  cam_start_dist = cam->GetPosition().z;
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeaugmentedreality_TangoJNINative_setCameraOffset(
    JNIEnv*, jobject, float rotation_x, float rotation_y, float dist) {
  cam_cur_angle[0] = cam_start_angle[0] + rotation_x;
  cam_cur_angle[1] = cam_start_angle[1] + rotation_y;
  dist = tango_gl::util::Clamp(cam_start_dist + dist * kZoomSpeed,
                               kCamViewMinDist, kCamViewMaxDist);
  cam_cur_dist = dist;
}
#ifdef __cplusplus
}
#endif
