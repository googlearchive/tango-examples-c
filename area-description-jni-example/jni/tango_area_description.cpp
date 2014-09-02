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

#include <string>

#include "axis.h"
#include "camera.h"
#include "frustum.h"
#include "gl_util.h"
#include "grid.h"
#include "tango_data.h"
#include "trace.h"

GLuint screen_width;
GLuint screen_height;

Camera *cam;
Axis *axis;
Frustum *frustum;
Grid *grid;
Trace *trace;

enum CameraType {
  FIRST_PERSON = 0,
  THIRD_PERSON = 1,
  TOP_DOWN = 2
};

int camera_type;

// Quaternion format of rotation.
const glm::vec3 kThirdPersonCameraPosition = glm::vec3(0.0f, 3.0f, 3.0f);
const glm::quat kThirdPersonCameraRotation = glm::quat(0.92388f, -0.38268f,
                                                       0.0f, 0.0f);
const glm::vec3 kTopDownCameraPosition = glm::vec3(0.0f, 3.0f, 0.0f);
const glm::quat kTopDownCameraRotation = glm::quat(0.70711f, -0.70711f, 0.0f,
                                                   0.0f);

bool SetupGraphics(int w, int h) {
  LOGI("setupGraphics(%d, %d)", w, h);

  screen_width = w;
  screen_height = h;

  cam = new Camera();
  axis = new Axis();
  frustum = new Frustum();
  trace = new Trace();
  grid = new Grid();

  camera_type = FIRST_PERSON;
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

  grid->SetPosition(glm::vec3(0.0f, -0.8f, 0.0f));
  grid->Render(cam->GetCurrentProjectionViewMatrix());

  int pose_index =
    TangoData::GetInstance().current_pose_status[1]==TANGO_POSE_VALID ? 1 : 0;
  glm::vec3 position = GlUtil::ConvertPositionToOpenGL(
      TangoData::GetInstance().tango_position[pose_index]);
  glm::quat rotation = GlUtil::ConvertRotationToOpenGL(
      TangoData::GetInstance().tango_rotation[pose_index]);

  if (camera_type == FIRST_PERSON) {
    cam->SetPosition(position);
    cam->SetRotation(rotation);
  } else {
    frustum->SetPosition(position);
    frustum->SetRotation(rotation);
    frustum->Render(cam->GetCurrentProjectionViewMatrix());

    trace->UpdateVertexArray(position);
    trace->Render(cam->GetCurrentProjectionViewMatrix());

    axis->SetPosition(position);
    axis->SetRotation(rotation);
    axis->Render(cam->GetCurrentProjectionViewMatrix());
  }
  return true;
}

void SetCamera(int camera_index) {
  camera_type = camera_index;
  switch (camera_index) {
    case FIRST_PERSON:
      LOGI("setting to First Person Camera");
      break;
    case THIRD_PERSON:
      cam->SetPosition(kThirdPersonCameraPosition);
      cam->SetRotation(kThirdPersonCameraRotation);
      LOGI("setting to Third Person Camera");
      break;
    case TOP_DOWN:
      cam->SetPosition(kTopDownCameraPosition);
      cam->SetRotation(kTopDownCameraRotation);
      LOGI("setting to Top Down Camera");
      break;
    default:
      break;
  }
}

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_Initialize(
    JNIEnv* env, jobject obj, bool is_learning, bool is_load_adf) {
  LOGI("leanring:%d, adf:%d", is_learning, is_load_adf);
  LOGI("In onCreate: Initialing and setting config");
  if (!TangoData::GetInstance().Initialize())
  {
    LOGE("Tango initialization failed");
  }
  if (!TangoData::GetInstance().SetConfig(is_learning, is_load_adf))
  {
    LOGE("Tango set config failed");
  }
}

JNIEXPORT void JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_ConnectService(
    JNIEnv* env, jobject obj) {
  LOGI("In OnResume: Locking config and connecting service");
  if (!TangoData::GetInstance().LockConfig()) {
    LOGE("Tango lock config failed");
  }
  if (!TangoData::GetInstance().Connect()) {
    LOGE("Tango connect failed");
  }
}

JNIEXPORT void JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_DisconnectService(
    JNIEnv* env, jobject obj) {
  LOGI("In OnPause: Unlocking config and disconnecting service");
  if (TangoData::GetInstance().UnlockConfig()) {
    LOGE("Tango unlock file failed");
  }
  TangoData::GetInstance().Disconnect();
}

JNIEXPORT void JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_OnDestroy(
    JNIEnv* env, jobject obj) {
  delete cam;
  delete axis;
  delete grid;
  delete frustum;
  delete trace;
}

JNIEXPORT void JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_SetupGraphic(
    JNIEnv* env, jobject obj, jint width, jint height) {
  SetupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_Render(
    JNIEnv* env, jobject obj) {
  RenderFrame();
}

JNIEXPORT void JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_SetCamera(
    JNIEnv* env, jobject obj, int camera_index) {
  SetCamera(camera_index);
}

JNIEXPORT jstring JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_SaveADF(
    JNIEnv* env, jobject obj) {
  // Save ADF.
  return (env)->NewStringUTF(TangoData::GetInstance().SaveADF());
}

JNIEXPORT void JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_RemoveAllAdfs(
    JNIEnv* env, jobject obj) {
  TangoData::GetInstance().RemoveAllAdfs();
}

JNIEXPORT jstring JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_GetUUID(
    JNIEnv* env, jobject obj) {
  return (env)->NewStringUTF(TangoData::GetInstance().uuid_);
}

JNIEXPORT jstring JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_GetIsEnabledLearn(
    JNIEnv* env, jobject obj) {
  return (env)->NewStringUTF(
      TangoData::GetInstance().is_learning_mode_enabled ? "Enabled" : "Disable");
}

JNIEXPORT jstring JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_GetPoseString(
    JNIEnv* env, jobject obj, int index) {
  char pose_string[100];
  char status[30];
  
  switch (TangoData::GetInstance().current_pose_status[index]) {
    case TANGO_POSE_INITIALIZING:
      sprintf(status,"Initializing");
      break;
    case TANGO_POSE_VALID:
      sprintf(status, "Valid");
      break;
    case TANGO_POSE_INVALID:
      sprintf(status, "Invalid");
      break;
    case TANGO_POSE_UNKNOWN:
      sprintf(status, "Unknown");
      break;
    default:
      break;
  }
  
  sprintf(pose_string, "status:%s, count:%d, pose:%4.2f %4.2f %4.2f, delta_time: %4.2fms",
          status,
          TangoData::GetInstance().frame_count[index],
          TangoData::GetInstance().tango_position[index].x,
          TangoData::GetInstance().tango_position[index].y,
          TangoData::GetInstance().tango_position[index].z,
          TangoData::GetInstance().frame_delta_time[index]*1000.0f);

  return (env)->NewStringUTF(pose_string);
}

#ifdef __cplusplus
}
#endif
