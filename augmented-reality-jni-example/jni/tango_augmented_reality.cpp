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
#include <math.h>

#include "axis.h"
//#include "camera.h"
#include "frustum.h"
#include "gl_util.h"
#include "grid.h"
#include "tango_data.h"
#include "trace.h"
#include "ar_ruler.h"
#include "cube.h"
#include "video_overlay.h"

GLuint screen_width;
GLuint screen_height;

//Camera *cam;
Axis *axis;
Frustum *frustum;
Grid *grid;
Trace *trace;
ArRuler *arRuler;
Cube *cube;
VideoOverlay *video_overlay;

enum CameraType {
  FIRST_PERSON = 0,
  THIRD_PERSON = 1,
  TOP_DOWN = 2
};

int camera_type;

// Quaternion format of rotation.
const glm::vec3 kThirdPersonCameraPosition = glm::vec3(-1.5f, 3.0f, 3.0f);
const glm::quat kThirdPersonCameraRotation = glm::quat(0.91598f, -0.37941f,
                                                       -0.12059f, -0.04995f);
const glm::vec3 kTopDownCameraPosition = glm::vec3(0.0f, 3.0f, 0.0f);
const glm::quat kTopDownCameraRotation = glm::quat(0.70711f, -0.70711f, 0.0f,
                                                   0.0f);
const glm::vec3 kGridPosition = glm::vec3(0.0f, -1.67f, 0.0f);
const glm::vec3 kCubePosition = glm::vec3(-1.0f, 0.1 - 1.67f, -3.0f);

glm::mat4 projectionMat;
glm::mat4 viewMat;
glm::mat4 ssToOWMat;
glm::mat4 dToIMUMat;
glm::mat4 cToIMUMat;
glm::mat4 ocToCMat;
glm::mat4 ocToDMat;

// Print out a column major matrix.
void printMatrix(glm::mat4 matrix) {
  int i;
  for (i = 0; i < 4; i++) {
    LOGI("%f,%f,%f,%f", matrix[0][i], matrix[1][i], matrix[2][i], matrix[3][i]);
  }
  LOGI("  ");
}

void printVector(glm::vec3 vector) {
  LOGI("%f,%f,%f", vector[0], vector[1], vector[2]);
  LOGI("  ");
}

void SetupExtrinsics() {
  dToIMUMat = glm::translate(glm::mat4(1.0f),
                             TangoData::GetInstance().dToIMU_position)
      * glm::inverse(glm::mat4_cast(TangoData::GetInstance().dToIMU_rotation));
  LOGI("I_T_D = \n");
  printMatrix(dToIMUMat);
  cToIMUMat = glm::translate(glm::mat4(1.0f),TangoData::GetInstance().cToIMU_position) *
      glm::inverse(glm::mat4_cast(TangoData::GetInstance().cToIMU_rotation));
  LOGI("I_T_C = \n");
  printMatrix(cToIMUMat);
  LOGI("I_T_D^-1 * I_T_C = \n");
  printMatrix(glm::inverse(dToIMUMat) * cToIMUMat);

}

bool SetupGraphics(int w, int h) {
  LOGI("setupGraphics(%d, %d)", w, h);

  screen_width = w;
  screen_height = h;

  //cam = new Camera();
  axis = new Axis();
  frustum = new Frustum();
  trace = new Trace();
  grid = new Grid();
  arRuler = new ArRuler();
  cube = new Cube();
  video_overlay = new VideoOverlay();

  camera_type = FIRST_PERSON;
  //cam->SetAspectRatio((float) (w / h));
  projectionMat = glm::perspective(38.16f, (float) (1280.0f / 720.0f),
                                   1.625875f, 100.0f);
  float ssToOWArray[16] = {
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, -1.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
  };

  float ocToCArray[16] = {
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, -1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, -1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
  };

  float ocToDArray[16] = {
      1, 0, 0, 0,
      0, cos(13 * 3.14f / 180.0f), sin(13 * 3.14f / 180.0f), 0,
      0, -sin(13 * 3.14f / 180.0f), cos(13 * 3.14f / 180.0f), 0,
      0, 0, 0, 1
  };

  memcpy(glm::value_ptr(ssToOWMat), ssToOWArray, sizeof(ssToOWArray));
  LOGI("OW_T_SS = \n");
  printMatrix(ssToOWMat);
  memcpy(glm::value_ptr(ocToCMat), ocToCArray, sizeof(ocToCArray));

  memcpy(glm::value_ptr(ocToDMat), ocToDArray, sizeof(ocToDArray));
  LOGI("D_T_Oc = \n");
  printMatrix(ocToDMat);
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

  TangoData::GetInstance().UpdateColorTexture();
  video_overlay->Render();

  glm::vec3 position = TangoData::GetInstance().GetTangoPosition();
  glm::quat rotation = TangoData::GetInstance().GetTangoRotation();

  glm::mat4 dToSSMat = glm::translate(glm::mat4(1.0f), position)
      * glm::mat4_cast(rotation);

  glm::mat4 viewInversed = ssToOWMat * dToSSMat * glm::inverse(dToIMUMat) * cToIMUMat * ocToCMat;

  viewMat = glm::inverse(viewInversed);

//  if (camera_type == FIRST_PERSON) {
//    cam->SetPosition(position);
//    cam->SetRotation(rotation);
//  } else {
//    if(camera_type == TOP_DOWN){
//      cam->SetPosition(position+kTopDownCameraPosition);
//    }else{
//      cam->SetPosition(position+kThirdPersonCameraPosition);
//    }
//    frustum->SetPosition(position);
//    frustum->SetRotation(rotation);
//    frustum->Render(cam->GetCurrentProjectionViewMatrix());
//
//    trace->UpdateVertexArray(position);
//    trace->Render(cam->GetCurrentProjectionViewMatrix());
//
//    axis->SetPosition(position);
//    axis->SetRotation(rotation);
//    axis->Render(cam->GetCurrentProjectionViewMatrix());
//  }

  grid->SetPosition(kGridPosition);
  grid->Render(projectionMat, viewMat);

  arRuler->SetPosition(kGridPosition);
  arRuler->Render(projectionMat, viewMat);

  cube->SetPosition(kCubePosition);
  cube->SetScale(glm::vec3(0.2f, 0.2f, 0.2f));
  cube->Render(projectionMat, viewMat);
  return true;
}

void SetCamera(int camera_index) {
  camera_type = camera_index;
  switch (camera_index) {
    case FIRST_PERSON:
      LOGI("setting to First Person Camera");
      break;
    case THIRD_PERSON:
      //cam->SetRotation(kThirdPersonCameraRotation);
      LOGI("setting to Third Person Camera");
      break;
    case TOP_DOWN:
      //cam->SetRotation(kTopDownCameraRotation);
      LOGI("setting to Top Down Camera");
      break;
    default:
      break;
  }
}

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_com_projecttango_augmentedrealitynative_TangoJNINative_Initialize(
    JNIEnv* env, jobject obj, bool isAutoReset) {
  if(isAutoReset) {
    LOGI("Initialize with auto reset");
  } else {
    LOGI("Initialize with manual reset");
  }
  if (!TangoData::GetInstance().Initialize())
  {
    LOGE("Tango initialization failed");
  }
  if (!TangoData::GetInstance().SetConfig(isAutoReset))
  {
    LOGE("Tango set config failed");
  }
  SetupExtrinsics();
  TangoData::GetInstance().ConnectTexture(video_overlay->texture_id);
}

JNIEXPORT void JNICALL Java_com_projecttango_augmentedrealitynative_TangoJNINative_ConnectService(
    JNIEnv* env, jobject obj) {
  LOGI("ConnectService:Locking config and connecting service");
  if (!TangoData::GetInstance().LockConfig()) {
    LOGE("Tango lock config failed");
  }
  if (!TangoData::GetInstance().Connect()) {
    LOGE("Tango connect failed");
  }
}

JNIEXPORT void JNICALL Java_com_projecttango_augmentedrealitynative_TangoJNINative_DisconnectService(
    JNIEnv* env, jobject obj) {
  LOGI("DisconectService: Unlocking config and disconnecting service");
  if (TangoData::GetInstance().UnlockConfig()) {
    LOGE("Tango unlock file failed");
  }
  TangoData::GetInstance().Disconnect();
}

JNIEXPORT void JNICALL Java_com_projecttango_augmentedrealitynative_TangoJNINative_OnDestroy(
    JNIEnv* env, jobject obj) {
//  delete cam;
  delete axis;
  delete grid;
  delete frustum;
  delete trace;
  delete arRuler;
  delete cube;
  delete video_overlay;
}

JNIEXPORT void JNICALL Java_com_projecttango_augmentedrealitynative_TangoJNINative_SetupGraphic(
    JNIEnv* env, jobject obj, jint width, jint height) {
  SetupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_com_projecttango_augmentedrealitynative_TangoJNINative_Render(
    JNIEnv* env, jobject obj) {
  RenderFrame();
}

JNIEXPORT void JNICALL Java_com_projecttango_augmentedrealitynative_TangoJNINative_ResetMotionTracking(
    JNIEnv* env, jobject obj) {
  TangoData::GetInstance().ResetMotionTracking();
  LOGI("Reset Tango Motion Tracking");
}

JNIEXPORT void JNICALL Java_com_projecttango_augmentedrealitynative_TangoJNINative_SetCamera(
    JNIEnv* env, jobject obj, int camera_index) {
  SetCamera(camera_index);
}

JNIEXPORT jstring JNICALL Java_com_projecttango_augmentedrealitynative_TangoJNINative_PoseToString(
    JNIEnv* env, jobject obj) {
  return (env)->NewStringUTF(TangoData::GetInstance().PoseToString());
}

JNIEXPORT jstring JNICALL Java_com_projecttango_augmentedrealitynative_TangoJNINative_EventToString(
    JNIEnv* env, jobject obj) {
  return (env)->NewStringUTF(TangoData::GetInstance().eventString);
}

JNIEXPORT jchar JNICALL Java_com_projecttango_augmentedrealitynative_TangoJNINative_UpdateStatus(
    JNIEnv* env, jobject obj) {
  return TangoData::GetInstance().GetTangoPoseStatus();
}

JNIEXPORT jstring JNICALL Java_com_projecttango_augmentedrealitynative_TangoJNINative_GetVersionNumber(
    JNIEnv* env, jobject obj) {
  return (env)->NewStringUTF(TangoData::GetInstance().lib_version);
}
#ifdef __cplusplus
}
#endif
