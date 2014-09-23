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

#include "axis.h"
#include "camera.h"
#include "frustum.h"
#include "gl_util.h"
#include "grid.h"
#include "tango_data.h"
#include "trace.h"

GLuint screen_width;
GLuint screen_height;

// Render camera's parent transformation.
// This object is a pivot transformtion for render camera to rotate around.
Transform* cam_parent_transform;

Camera *cam;
Axis *axis;
Frustum *frustum;
Grid *grid;
Trace *trace;

int cur_cam_index = 0;

float cam_start_angle[2];
float cam_cur_angle[2];
float cam_start_dist;
float cam_cur_dist;

enum CameraType {
  FIRST_PERSON = 0,
  THIRD_PERSON = 1,
  TOP_DOWN = 2
};

int camera_type;

// Quaternion format of rotation.
const glm::vec3 kThirdPersonCameraPosition = glm::vec3(5.0f, 5.0f, 5.0f);
const glm::quat kThirdPersonCameraRotation = glm::quat(0.85355f, -0.35355f,
                                                       0.35355f, 0.14645f);
const glm::vec3 kTopDownCameraPosition = glm::vec3(0.0f, 8.0f, 0.0f);
const glm::quat kTopDownCameraRotation = glm::quat(0.70711f, -0.70711f, 0.0f,
                                                   0.0f);
const glm::vec3 kGridPosition = glm::vec3(0.0f, -1.67f, 0.0f);

bool SetupGraphics(int w, int h) {
  LOGI("setupGraphics(%d, %d)", w, h);

  screen_width = w;
  screen_height = h;

  cam_parent_transform = new Transform();
  cam = new Camera();
  axis = new Axis();
  frustum = new Frustum();
  trace = new Trace();
  grid = new Grid();

  // Set the parent-child camera transfromation.
  cam->SetParent(cam_parent_transform);
  
  cam->SetAspectRatio((float) (w / h));
  return true;
}

// Render frustum and trace with current position and rotation
// updated from TangoData, TangoPosition and TangoRotation is updated via callback function
// OnPoseAvailable(), which is updated when new pose data is available.
bool RenderFrame() {
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glViewport(0, 0, screen_width, screen_height);

  grid->SetPosition(kGridPosition);
  grid->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());

  glm::vec3 position = GlUtil::ConvertPositionToOpenGL(
      TangoData::GetInstance().tango_position);
  glm::quat rotation = GlUtil::ConvertRotationToOpenGL(
      TangoData::GetInstance().tango_rotation);

  if (cur_cam_index == 0) {
    cam->SetPosition(position);
    cam->SetRotation(rotation);
  } else {
    // Get parent camera's rotation from touch.
    // Note that the render camera is a child transformation
    // of the this transformation.
    // cam_cur_angle[0] is the x-axis touch, cooresponding to y-axis rotation.
    // cam_cur_angle[0] is the y-axis touch, cooresponding to x-axis rotation.
    glm::quat parent_cam_rot =
      glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), -cam_cur_angle[0], glm::vec3(0, 1, 0));
    parent_cam_rot = glm::rotate(parent_cam_rot, -cam_cur_angle[1], glm::vec3(1, 0, 0));
    
    // Set render camera parent position and rotation.
    cam_parent_transform->SetRotation(parent_cam_rot);
    cam_parent_transform->SetPosition(position);
    
    // Set camera view distance, based on touch interaction.
    cam->SetPosition(glm::vec3(0.0f,0.0f, cam_cur_dist));
    
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

void SetCamera(int camera_index) {
  cur_cam_index = camera_index;
  cam_cur_angle[0] = cam_cur_angle[1] = cam_cur_dist = 0.0f;
  switch (camera_index) {
    case 0:
      cam_parent_transform->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
      cam_parent_transform->SetRotation(glm::quat(1.0f, 0.0f, 0.0, 0.0f));
      break;
    case 1:
      cam_parent_transform->SetRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
      cam->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
      cam->SetRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
      cam_cur_dist = 4.0f;
      cam_cur_angle[0] = -0.785f;
      cam_cur_angle[1] = 0.785f;
      break;
    case 2:
      cam_parent_transform->SetRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
      cam->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
      cam->SetRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
      cam_cur_dist = 8.0f;
      cam_cur_angle[1] = 1.57f;
      break;
    default:
      break;
  }
}

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_com_projecttango_motiontrackingnative_TangoJNINative_Initialize(
    JNIEnv* env, jobject obj) {
  if (!TangoData::GetInstance().Initialize())
  {
    LOGE("Tango initialization failed");
  }
}

JNIEXPORT void JNICALL Java_com_projecttango_motiontrackingnative_TangoJNINative_SetupConfig(
    JNIEnv* env, jobject obj, bool isAutoReset) {
  if (!TangoData::GetInstance().SetConfig(isAutoReset))
  {
    LOGE("Tango set config failed");
  }
}
  
JNIEXPORT void JNICALL Java_com_projecttango_motiontrackingnative_TangoJNINative_ConnectService(
    JNIEnv* env, jobject obj) {
  if (!TangoData::GetInstance().Connect()) {
    LOGE("Tango connect failed");
  }
}

JNIEXPORT void JNICALL Java_com_projecttango_motiontrackingnative_TangoJNINative_DisconnectService(
    JNIEnv* env, jobject obj) {
  TangoData::GetInstance().Disconnect();
}

JNIEXPORT void JNICALL Java_com_projecttango_motiontrackingnative_TangoJNINative_OnDestroy(
    JNIEnv* env, jobject obj) {
  delete cam;
  delete axis;
  delete grid;
  delete frustum;
  delete trace;
}

JNIEXPORT void JNICALL Java_com_projecttango_motiontrackingnative_TangoJNINative_SetupGraphic(
    JNIEnv* env, jobject obj, jint width, jint height) {
  SetupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_com_projecttango_motiontrackingnative_TangoJNINative_Render(
    JNIEnv* env, jobject obj) {
  RenderFrame();
}

JNIEXPORT void JNICALL Java_com_projecttango_motiontrackingnative_TangoJNINative_ResetMotionTracking(
    JNIEnv* env, jobject obj) {
  TangoData::GetInstance().ResetMotionTracking();
  LOGI("Reset Tango Motion Tracking");
}

JNIEXPORT void JNICALL Java_com_projecttango_motiontrackingnative_TangoJNINative_SetCamera(
    JNIEnv* env, jobject obj, int camera_index) {
  SetCamera(camera_index);
}

JNIEXPORT jstring JNICALL Java_com_projecttango_motiontrackingnative_TangoJNINative_GetPoseString(
    JNIEnv* env, jobject obj) {
  return (env)->NewStringUTF(TangoData::GetInstance().GetPoseDataString());
}

JNIEXPORT jstring JNICALL Java_com_projecttango_motiontrackingnative_TangoJNINative_GetEventString(
    JNIEnv* env, jobject obj) {
  return (env)->NewStringUTF(TangoData::GetInstance().GetEventString());
}

JNIEXPORT jstring JNICALL Java_com_projecttango_motiontrackingnative_TangoJNINative_GetVersionNumber(
    JNIEnv* env, jobject obj) {
  return (env)->NewStringUTF(TangoData::GetInstance().GetVersionString());
}
  
// Touching GL interface.
JNIEXPORT void JNICALL Java_com_projecttango_motiontrackingnative_TangoJNINative_StartSetCameraOffset(
    JNIEnv* env, jobject obj) {
  cam_start_angle[0] = cam_cur_angle[0];
  cam_start_angle[1] = cam_cur_angle[1];
  cam_start_dist = cam->GetPosition().z;
}
  
JNIEXPORT void JNICALL Java_com_projecttango_motiontrackingnative_TangoJNINative_SetCameraOffset(
    JNIEnv* env, jobject obj, float rotation_x, float rotation_y, float dist) {
  cam_cur_angle[0] = cam_start_angle[0] + rotation_x;
  cam_cur_angle[1] = cam_start_angle[1] + rotation_y;
  dist = GlUtil::Clamp(cam_start_dist + dist*10.0f, 1.0f, 100.0f);
  cam_cur_dist = dist;
}
#ifdef __cplusplus
}
#endif
