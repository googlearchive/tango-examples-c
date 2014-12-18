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

#include "tango-gl-renderer/axis.h"
#include "tango-gl-renderer/camera.h"
#include "tango-gl-renderer/frustum.h"
#include "tango-gl-renderer/gl_util.h"
#include "tango-gl-renderer/grid.h"
#include "tango-gl-renderer/trace.h"

#include "tango_data.h"

GLuint screen_width;
GLuint screen_height;

// Render camera's parent transformation.
// This object is a pivot transformtion for render camera to rotate around.
Transform* cam_parent_transform;

// Render camera.
Camera* cam;

// Device axis (in device frame of reference).
Axis* axis;

// Device frustum.
Frustum* frustum;

// Ground grid.
Grid* grid;

// Trace of pose data.
Trace* trace;

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

// Render and camera controlling constant values.
// Height offset is used for offset height of motion tracking
// pose data. Motion tracking start position is (0,0,0). Adding
// a height offset will give a more reasonable pose while a common
// human is holding the device. The units is in meters.
const glm::vec3 kHeightOffset = glm::vec3(0.0f, 1.3f, 0.0f);

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
// First person camera's FOV is 45 degrees.
const float kHighFov = 65.0f;
const float kLowFov = 45.0f;

// Color of the motion tracking trajectory.
const float kTraceColor[] = {0.22f, 0.28f, 0.67f, 1.0f};

// Frustum scale.
const glm::vec3 kFrustumScale = glm::vec3(0.4f, 0.3f, 0.5f);

// Set camera type, set render camera's parent position and rotation.
void SetCamera(CameraType camera_index) {
  camera_type = camera_index;
  cam_cur_angle[0] = cam_cur_angle[1] = cam_cur_dist = 0.0f;
  switch (camera_index) {
    case CameraType::FIRST_PERSON:
      cam->SetFieldOfView(kLowFov);
      cam_parent_transform->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
      cam_parent_transform->SetRotation(glm::quat(1.0f, 0.0f, 0.0, 0.0f));
      break;
    case CameraType::THIRD_PERSON:
      cam->SetFieldOfView(kHighFov);
      cam->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
      cam->SetRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
      cam_cur_dist = kThirdPersonCameraDist;
      cam_cur_angle[0] = -PI / 4.0f;
      cam_cur_angle[1] = PI / 4.0f;
      break;
    case CameraType::TOP_DOWN:
      cam->SetFieldOfView(kHighFov);
      cam->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
      cam->SetRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
      cam_cur_dist = kTopDownCameraDist;
      cam_cur_angle[1] = PI / 2.0f;
      break;
    default:
      break;
  }
}

bool InitGlContent() {
  cam_parent_transform = new Transform();
  cam = new Camera();
  axis = new Axis();
  frustum = new Frustum();
  trace = new Trace();
  grid = new Grid();

  frustum->SetScale(kFrustumScale);

  // Set trace's color to show motion tracking trajectory.
  trace->SetTraceColor(kTraceColor);

  // Set the parent-child camera transfromation.
  cam->SetParent(cam_parent_transform);

  SetCamera(CameraType::THIRD_PERSON);

  return true;
}

bool SetupGraphics(int w, int h) {
  screen_width = w;
  screen_height = h;
  if (h == 0) {
    LOGE("Setup graphic height not valid");
    return false;
  }
  cam->SetAspectRatio(static_cast<float>(w) / static_cast<float>(h));
  return true;
}

// Render frustum and trace with current position and rotation
// updated from TangoData, TangoPosition and TangoRotation is updated via
// callback function OnPoseAvailable(), which is updated when new pose data
// is available.
bool RenderFrame() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glViewport(0, 0, screen_width, screen_height);

  grid->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());

  pthread_mutex_lock(&TangoData::GetInstance().pose_mutex);
  glm::vec3 position = GlUtil::ConvertPositionToOpenGL(
      TangoData::GetInstance().tango_position);
  position += kHeightOffset;
  glm::quat rotation = GlUtil::ConvertRotationToOpenGL(
      TangoData::GetInstance().tango_rotation);
  pthread_mutex_unlock(&TangoData::GetInstance().pose_mutex);

  if (camera_type == CameraType::FIRST_PERSON) {
    cam->SetPosition(position);
    cam->SetRotation(rotation);
  } else {
    // Get parent camera's rotation from touch.
    // Note that the render camera is a child transformation
    // of the this transformation.
    // cam_cur_angle[0] is the x-axis touch, cooresponding to y-axis rotation.
    // cam_cur_angle[0] is the y-axis touch, cooresponding to x-axis rotation.
    glm::quat parent_cam_rot =
        glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), -cam_cur_angle[0],
                    glm::vec3(0, 1, 0));
    parent_cam_rot =
        glm::rotate(parent_cam_rot, -cam_cur_angle[1], glm::vec3(1, 0, 0));

    // Set render camera parent position and rotation.
    cam_parent_transform->SetRotation(parent_cam_rot);
    cam_parent_transform->SetPosition(position);

    // Set camera view distance, based on touch interaction.
    cam->SetPosition(glm::vec3(0.0f, 0.0f, cam_cur_dist));

    frustum->SetPosition(position);
    frustum->SetRotation(rotation);
    frustum->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());

    axis->SetPosition(position);
    axis->SetRotation(rotation);
    axis->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());
  }

  trace->UpdateVertexArray(position);
  trace->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());

  return true;
}

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_initialize(
    JNIEnv* env, jobject, jobject activity) {
  TangoErrorType err = TangoData::GetInstance().Initialize(env, activity);
  if (err != TANGO_SUCCESS) {
    if (err == TANGO_INVALID) {
      LOGE("Tango Service version mis-match");
    } else {
      LOGE("Tango Service initialize internal error");
    }
  }
  return static_cast<int>(err);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_setupConfig(
    JNIEnv*, jobject, bool isAutoReset) {
  if (!TangoData::GetInstance().SetConfig(isAutoReset)) {
    LOGE("Tango set config failed");
  }
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_connect(
    JNIEnv*, jobject) {
  TangoErrorType err = TangoData::GetInstance().Connect();
  if (err != TANGO_SUCCESS) {
    LOGE("Tango Service connect failed");
  }
  return static_cast<int>(err);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_connectCallbacks(
    JNIEnv*, jobject) {
  if (!TangoData::GetInstance().ConnectCallbacks()) {
    LOGE("Tango ConnectCallbacks failed");
  }
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_disconnect(
    JNIEnv*, jobject) {
  TangoData::GetInstance().Disconnect();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_freeGLContent(
    JNIEnv*, jobject) {
  delete cam;
  delete axis;
  delete grid;
  delete frustum;
  delete trace;
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_initGlContent(
    JNIEnv*, jobject) {
  InitGlContent();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_setupGraphic(
    JNIEnv*, jobject, jint width, jint height) {
  SetupGraphics(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_render(
    JNIEnv*, jobject) {
  RenderFrame();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_resetMotionTracking(
    JNIEnv*, jobject) {
  TangoData::GetInstance().ResetMotionTracking();
  LOGI("Reset Tango Motion Tracking");
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_setCamera(
    JNIEnv*, jobject, int camera_index) {
  SetCamera((CameraType)camera_index);
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_getPoseString(
    JNIEnv* env, jobject) {
  pthread_mutex_lock(&TangoData::GetInstance().pose_mutex);
  std::string pose_string_cpy =
      std::string(TangoData::GetInstance().pose_string);
  pthread_mutex_unlock(&TangoData::GetInstance().pose_mutex);
  return (env)->NewStringUTF(pose_string_cpy.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_getEventString(
    JNIEnv* env, jobject) {
  pthread_mutex_lock(&TangoData::GetInstance().event_mutex);
  std::string event_string_cpy =
      std::string(TangoData::GetInstance().event_string);
  pthread_mutex_unlock(&TangoData::GetInstance().event_mutex);
  return (env)->NewStringUTF(event_string_cpy.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_getVersionNumber(
    JNIEnv* env, jobject) {
  return (env)
      ->NewStringUTF(TangoData::GetInstance().lib_version_string.c_str());
}

// Touching GL interface.
JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_startSetCameraOffset(
    JNIEnv*, jobject) {
  if (cam != NULL) {
    cam_start_angle[0] = cam_cur_angle[0];
    cam_start_angle[1] = cam_cur_angle[1];
    cam_start_dist = cam->GetPosition().z;
  }
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_setCameraOffset(
    JNIEnv*, jobject, float rotation_x, float rotation_y, float dist) {
  if (cam != NULL) {
    cam_cur_angle[0] = cam_start_angle[0] + rotation_x;
    cam_cur_angle[1] = cam_start_angle[1] + rotation_y;
    dist = GlUtil::Clamp(cam_start_dist + dist * kZoomSpeed, kCamViewMinDist,
                         kCamViewMaxDist);
    cam_cur_dist = dist;
  }
}
#ifdef __cplusplus
}
#endif
