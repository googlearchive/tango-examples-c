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

#include "tango-gl-renderer/axis.h"
#include "tango-gl-renderer/camera.h"
#include "tango-gl-renderer/cube.h"
#include "tango-gl-renderer/frustum.h"
#include "tango-gl-renderer/gl_util.h"
#include "tango-gl-renderer/grid.h"
#include "tango-gl-renderer/trace.h"

#include "tango_data.h"
#include "video_overlay.h"

// Render camera's parent transformation.
// This object is a pivot transformtion for render camera to rotate around.
Transform* cam_parent_transform;

// Render camera.
Camera *cam;

// Coordinate axis for display.
Axis *axis;

// Color Camera frustum.
Frustum *frustum;

// Ground grid.
Grid* ground;

// AR element grid.
Grid* ar_grid;

// Trace of pose data.
Trace *trace;

// AR element cube.
Cube *cube;

// Color camera preview.
VideoOverlay *video_overlay;

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

// AR cube position in world coordinate.
const glm::vec3 kCubePosition = glm::vec3(-1.0f, 0.265f, -2.0f);

// AR grid position, can be modified based on the real world scene.
const glm::vec3 kGridPosition = glm::vec3(0.0f, 1.26f, -2.0f);

// AR cube dimension, based on real world scene.
const glm::vec3 kCubeScale = glm::vec3(0.38f, 0.53f, 0.57f);

// Height offset is used for offset height of motion tracking
// pose data. Motion tracking start position is (0,0,0). Adding
// a height offset will give a more reasonable pose while a common
// human is holding the device. The units is in meters.
glm::vec3 world_position = glm::vec3(0.0f, -1.4f, 0.0f);

// Color camera's position and rotatioin.
glm::vec3 cc_position;
glm::quat cc_rotation;

// Projection matrix from render camera.
glm::mat4 projection_mat;

// First person projection matrix from color camera intrinsics.
glm::mat4 projection_mat_ar;

// First person view matrix from color camera extrinsics.
glm::mat4 view_mat;

// Start of Service with respect to Opengl World matrix.
glm::mat4 ss_to_ow_mat;

// Device with respect to IMU matrix.
glm::mat4 d_to_imu_mat;

// Color Camera with respect to IMU matrix.
glm::mat4 c_to_imu_mat;

// Opengl Camera with respect to Color Camera matrix.
glm::mat4 oc_to_c_mat;

// Opengl World with respect to Opengl Camera matrix.
glm::mat4 ow_to_oc_mat;

// Color Camera image plane ratio.
float image_plane_ratio;
float image_width;
float image_height;

// Color Camera image plane distance to view point.
float image_plane_dis;
float image_plane_dis_original;

void SetupExtrinsics() {
  d_to_imu_mat = glm::translate(glm::mat4(1.0f),
                                TangoData::GetInstance().d_to_imu_position) *
                 glm::mat4_cast(TangoData::GetInstance().d_to_imu_rotation);

  c_to_imu_mat = glm::translate(glm::mat4(1.0f),
                                TangoData::GetInstance().c_to_imu_position) *
                 glm::mat4_cast(TangoData::GetInstance().c_to_imu_rotation);
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

bool SetupGraphics() {
  cam_parent_transform = new Transform();
  cam = new Camera();
  axis = new Axis();
  frustum = new Frustum();
  trace = new Trace();
  ground = new Grid(1.0f, 10, 10);
  ar_grid = new Grid(0.1f, 6, 4);
  cube = new Cube();
  video_overlay = new VideoOverlay();

  camera_type = CameraType::FIRST_PERSON;
  cam->SetParent(cam_parent_transform);
  cam->SetFieldOfView(kFov);

  ss_to_ow_mat = GlUtil::ss_to_ow_mat;
  oc_to_c_mat = GlUtil::oc_to_c_mat;

  ground->SetPosition(world_position);
  ar_grid->SetPosition(kGridPosition + world_position);
  ar_grid->SetRotation(kArGridRotation);
  cube->SetPosition(kCubePosition + world_position);
  cube->SetScale(kCubeScale);
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
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  TangoData::GetInstance().UpdateColorTexture();
  TangoData::GetInstance().GetPoseAtTime();

  glm::vec3 position = TangoData::GetInstance().tango_position;
  glm::quat rotation = TangoData::GetInstance().tango_rotation;

  glm::mat4 d_to_ss_mat =
      glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation);

  ow_to_oc_mat = ss_to_ow_mat * d_to_ss_mat * glm::inverse(d_to_imu_mat) *
                 c_to_imu_mat * oc_to_c_mat;
  glm::vec3 scale;
  GlUtil::DecomposeMatrix(ow_to_oc_mat, cc_position, cc_rotation, scale);

  if (camera_type == CameraType::FIRST_PERSON) {
    glDisable(GL_DEPTH_TEST);
    video_overlay->Render(glm::mat4(1.0f), glm::mat4(1.0f));
    glEnable(GL_DEPTH_TEST);
    projection_mat = projection_mat_ar;
    view_mat = glm::inverse(ow_to_oc_mat);
  } else {
    glm::quat parent_cam_rot = glm::rotate(kZeroQuat, -cam_cur_angle[0],
                                           glm::vec3(kZero, 1.0f, kZero));
    parent_cam_rot = glm::rotate(parent_cam_rot, -cam_cur_angle[1],
                                 glm::vec3(1.0f, kZero, kZero));

    cam_parent_transform->SetRotation(parent_cam_rot);
    cam_parent_transform->SetPosition(position);

    cam->SetPosition(glm::vec3(kZero, kZero, cam_cur_dist));

    projection_mat = cam->GetProjectionMatrix();
    view_mat = cam->GetViewMatrix();

    frustum->SetPosition(cc_position);
    frustum->SetRotation(cc_rotation);
    frustum->Render(projection_mat, view_mat);

    trace->UpdateVertexArray(cc_position);
    trace->Render(projection_mat, view_mat);

    axis->SetPosition(cc_position);
    axis->SetRotation(cc_rotation);
    axis->Render(projection_mat, view_mat);

    video_overlay->Render(projection_mat, view_mat);
  }

  ground->Render(projection_mat, view_mat);
  ar_grid->Render(projection_mat, view_mat);
  cube->Render(projection_mat, view_mat);
  return true;
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

      cam_parent_transform->SetRotation(kZeroQuat);
      cam->SetPosition(kZeroVec3);
      cam->SetRotation(kZeroQuat);
      cam_cur_dist = kThirdPersonCameraDist;
      cam_cur_angle[0] = -PI / 4.0f;
      cam_cur_angle[1] = PI / 4.0f;
      break;
    case CameraType::TOP_DOWN:
      video_overlay->SetScale(glm::vec3(1.0f, image_plane_ratio, 1.0f));
      video_overlay->SetRotation(kZeroQuat);
      video_overlay->SetPosition(glm::vec3(kZero, kZero, -image_plane_dis));
      video_overlay->SetParent(axis);

      cam_parent_transform->SetRotation(kZeroQuat);
      cam->SetPosition(kZeroVec3);
      cam->SetRotation(kZeroQuat);
      cam_cur_dist = kTopDownCameraDist;
      cam_cur_angle[1] = PI / 2.0f;
      break;
    default:
      break;
  }
}

// Reset virtual world, use the current color camera's position as origin.
void ResetAR() {
  world_position = cc_position + world_position;
  ground->SetPosition(world_position);
  ar_grid->SetPosition(world_position + kGridPosition);
  cube->SetPosition(world_position + kCubePosition);

  image_plane_dis = image_plane_dis_original;
  projection_mat_ar = glm::frustum(
      -1.0f * kFovScaler, 1.0f * kFovScaler, -image_plane_ratio * kFovScaler,
      image_plane_ratio * kFovScaler, image_plane_dis * kFovScaler,
      kCamViewMaxDist);
  frustum->SetScale(glm::vec3(1.0f, image_plane_ratio, image_plane_dis));
  SetCamera(camera_type);

  trace->ClearVertexArray();
}

void UpdateARElement(int ar_element, int interaction_type) {
  glm::vec3 translation;
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
      cube->Translate(translation);
      ar_grid->Translate(translation);
      break;
    case 2:
      cube->Translate(translation);
      break;
    case 3:
      ar_grid->Translate(translation);
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
Java_com_projecttango_augmentedrealitynative_TangoJNINative_Initialize(
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
  return (int)err;
}

JNIEXPORT void JNICALL
Java_com_projecttango_augmentedrealitynative_TangoJNINative_SetupConfig(
    JNIEnv*, jobject, bool is_auto_recovery) {
  if (!TangoData::GetInstance().SetConfig(is_auto_recovery)) {
    LOGE("Tango set config failed");
  } else {
    LOGI("Tango set config success");
  }
}

JNIEXPORT void JNICALL
Java_com_projecttango_augmentedrealitynative_TangoJNINative_ConnectTexture(
    JNIEnv*, jobject) {
  TangoData::GetInstance().ConnectTexture(video_overlay->texture_id);
}

JNIEXPORT jint JNICALL
Java_com_projecttango_augmentedrealitynative_TangoJNINative_ConnectService(
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
  return (int)err;
}

JNIEXPORT void JNICALL
Java_com_projecttango_augmentedrealitynative_TangoJNINative_SetupViewport(
    JNIEnv*, jobject, jint width, jint height) {
  SetupViewport(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_augmentedrealitynative_TangoJNINative_DisconnectService(
    JNIEnv*, jobject) {
  TangoData::GetInstance().Disconnect();
}

JNIEXPORT void JNICALL
Java_com_projecttango_augmentedrealitynative_TangoJNINative_OnDestroy(JNIEnv*,
                                                                      jobject) {
  delete cam;
  delete axis;
  delete ground;
  delete ar_grid;
  delete frustum;
  delete trace;
  delete cube;
  delete video_overlay;
}

JNIEXPORT void JNICALL
Java_com_projecttango_augmentedrealitynative_TangoJNINative_SetupGraphic(
    JNIEnv*, jobject) {
  SetupGraphics();
}

JNIEXPORT void JNICALL
Java_com_projecttango_augmentedrealitynative_TangoJNINative_Render(JNIEnv*,
                                                                   jobject) {
  RenderFrame();
}

JNIEXPORT void JNICALL
Java_com_projecttango_augmentedrealitynative_TangoJNINative_ResetMotionTracking(
    JNIEnv*, jobject) {
  ResetAR();
  // TangoData::GetInstance().ResetMotionTracking();
}

JNIEXPORT void JNICALL
Java_com_projecttango_augmentedrealitynative_TangoJNINative_SetCamera(
    JNIEnv*, jobject, int camera_index) {
  SetCamera(static_cast<CameraType>(camera_index));
}

JNIEXPORT void JNICALL
Java_com_projecttango_augmentedrealitynative_TangoJNINative_UpdateARElement(
    JNIEnv*, jobject, int ar_element, int interaction_type) {
  UpdateARElement(ar_element, interaction_type);
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_augmentedrealitynative_TangoJNINative_GetPoseString(
    JNIEnv* env, jobject) {
  pthread_mutex_lock(&TangoData::GetInstance().pose_mutex);
  string pose_string_cpy = string(TangoData::GetInstance().pose_string);
  pthread_mutex_unlock(&TangoData::GetInstance().pose_mutex);
  return (env)->NewStringUTF(pose_string_cpy.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_augmentedrealitynative_TangoJNINative_GetVersionNumber(
    JNIEnv* env, jobject) {
  return (env)
      ->NewStringUTF(TangoData::GetInstance().lib_version_string.c_str());
}

// Touching GL interface.
JNIEXPORT void JNICALL
Java_com_projecttango_augmentedrealitynative_TangoJNINative_StartSetCameraOffset(
    JNIEnv*, jobject) {
  cam_start_angle[0] = cam_cur_angle[0];
  cam_start_angle[1] = cam_cur_angle[1];
  cam_start_dist = cam->GetPosition().z;
}

JNIEXPORT void JNICALL
Java_com_projecttango_augmentedrealitynative_TangoJNINative_SetCameraOffset(
    JNIEnv*, jobject, float rotation_x, float rotation_y, float dist) {
  cam_cur_angle[0] = cam_start_angle[0] + rotation_x;
  cam_cur_angle[1] = cam_start_angle[1] + rotation_y;
  dist = GlUtil::Clamp(cam_start_dist + dist * kZoomSpeed, kCamViewMinDist,
                       kCamViewMaxDist);
  cam_cur_dist = dist;
}
#ifdef __cplusplus
}
#endif

