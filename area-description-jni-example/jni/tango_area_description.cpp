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

#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>

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

// Render camera.
Camera* cam;

// Device axis (in device frame of reference).
Axis* axis;

// Device frustum.
Frustum* frustum;

// Ground grid.
Grid* grid;

// Trace of motion tracking.
Trace* trace_motion;

// Trace of ADF.
Trace* trace_adf;

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
const float kTraceMotionColor[] = {0.22f, 0.28f, 0.67f, 1.0f};
// Color of the motion tracking trajectory.
const float kTraceADFColor[] = {0.39f, 0.56f, 0.03f, 1.0f};

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
  trace_motion = new Trace();
  trace_adf = new Trace();
  grid = new Grid();

  frustum->SetScale(kFrustumScale);

  // Set the parent-child camera transfromation.
  cam->SetParent(cam_parent_transform);

  SetCamera(CameraType::THIRD_PERSON);

  // Set trace's color to show motion tracking trajectory.
  trace_motion->SetTraceColor(kTraceMotionColor);

  // Set trace's color to show motion tracking trajectory.
  trace_adf->SetTraceColor(kTraceADFColor);
  return true;
}

bool SetupGraphics(int w, int h) {
  screen_width = w;
  screen_height = h;
  if (h == 0) {
    LOGE("Setup graphic height not valid");
    return false;
  }
  cam->SetAspectRatio((float)(w / h));
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

  grid->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f) - kHeightOffset);
  grid->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());

  int pose_index =
    TangoData::GetInstance().current_pose_status[1]==TANGO_POSE_VALID ? 1 : 0;

  pthread_mutex_lock(&TangoData::GetInstance().pose_mutex);
  glm::vec3 position = GlUtil::ConvertPositionToOpenGL(
      TangoData::GetInstance().tango_position[pose_index]);
  glm::quat rotation = GlUtil::ConvertRotationToOpenGL(
      TangoData::GetInstance().tango_rotation[pose_index]);
  glm::vec3 position_motion = GlUtil::ConvertPositionToOpenGL(
      TangoData::GetInstance().tango_position[0]);
  glm::vec3 position_adf = GlUtil::ConvertPositionToOpenGL(
      TangoData::GetInstance().tango_position[1]);
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

  if (TangoData::GetInstance().is_relocalized) {
    trace_adf->UpdateVertexArray(position_adf);
  } else {
    trace_motion->UpdateVertexArray(position_motion);
  }
  trace_adf->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());
  trace_motion->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());

  return true;
}

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_InitGlContent(
    JNIEnv*, jobject, jint width, jint height) {
  InitGlContent();
}

JNIEXPORT jint JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_Initialize(
    JNIEnv* env, jobject, jobject activity) {
  TangoErrorType err = TangoData::GetInstance().Initialize(env, activity);
  if (err != TANGO_SUCCESS) {
    if (err == TANGO_INVALID) {
      LOGE("Tango Service version mis-match");
    } else {
      LOGE("Tango Service initialize internal error");
    }
  }
  return (int)err;
}

JNIEXPORT jint JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_Connect(JNIEnv*,
                                                                   jobject) {
  TangoErrorType err = TangoData::GetInstance().Connect();
  if (err != TANGO_SUCCESS) {
    LOGE("Tango Service connect failed");
  }
  return (int)err;
}

JNIEXPORT void JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_SetupConfig(
    JNIEnv*, jobject, bool is_learning, bool is_load_adf) {
  LOGI("leanring:%d, adf:%d", is_learning, is_load_adf);
  if (!TangoData::GetInstance().SetConfig(is_learning, is_load_adf))
  {
    LOGE("Tango set config failed");
  }
}

JNIEXPORT void JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_Disconnect(JNIEnv*,
                                                                      jobject) {
  TangoData::GetInstance().Disconnect();
}

JNIEXPORT void JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_OnDestroy(JNIEnv*,
                                                                     jobject) {
  delete cam;
  delete axis;
  delete grid;
  delete frustum;
  delete trace_adf;
  delete trace_motion;
  TangoData::GetInstance().ResetData();
}

JNIEXPORT void JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_SetupGraphic(
    JNIEnv*, jobject, jint width, jint height) {
  SetupGraphics(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_Render(JNIEnv*,
                                                                  jobject) {
  RenderFrame();
}

JNIEXPORT void JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_SetCamera(
    JNIEnv*, jobject, int camera_index) {
  SetCamera(static_cast<CameraType>(camera_index));
}

JNIEXPORT bool JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_SaveADF(JNIEnv*,
                                                                   jobject) {
  // Save ADF.
  return TangoData::GetInstance().SaveADF();
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_GetUUID(JNIEnv* env,
                                                                   jobject) {
  return env->NewStringUTF(TangoData::GetInstance().cur_uuid.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_GetAllUUIDs(
    JNIEnv* env, jobject) {
  return env->NewStringUTF(TangoData::GetInstance().GetAllUUIDs());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_GetUUIDMetadataValue(
    JNIEnv* env, jobject, jstring uuid, jstring metadata_key) {
  const char* uuid_ = env->GetStringUTFChars(uuid, NULL);
  const char* metadata_key_ = env->GetStringUTFChars(metadata_key, NULL);
  return env->NewStringUTF(
      TangoData::GetInstance().GetUUIDMetadataValue(uuid_, metadata_key_));
}

JNIEXPORT void JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_SetUUIDMetadataValue(
    JNIEnv* env, jobject, jstring uuid, jstring metadata_key, jint value_size,
    jstring metadata_value) {
  const char* uuid_ = env->GetStringUTFChars(uuid, NULL);
  const char* metadata_key_ = env->GetStringUTFChars(metadata_key, NULL);
  const char* metadata_value_ = env->GetStringUTFChars(metadata_value, NULL);
  TangoData::GetInstance().SetUUIDMetadataValue(uuid_, metadata_key_,
                                                value_size, metadata_value_);
}

JNIEXPORT void JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_DeleteADF(
    JNIEnv* env, jobject, jstring uuid) {
  const char* uuid_ = env->GetStringUTFChars(uuid, NULL);
  TangoData::GetInstance().DeleteADF(uuid_);
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_GetPoseString(
    JNIEnv* env, jobject, int index) {
  stringstream string_stream;
  if (TangoData::GetInstance().current_pose_status[index] == -4) {
    string_stream
        << "status: N/A, count: N/A, delta time (ms): N/A, position (m): "
           "N/A, quat: N/A";
    return env->NewStringUTF(string_stream.str().c_str());
  }

  const char* status;
  switch (TangoData::GetInstance().current_pose_status[index]) {
    case TANGO_POSE_INITIALIZING:
      status = "initializing";
      break;
    case TANGO_POSE_VALID:
      status = "valid";
      break;
    case TANGO_POSE_INVALID:
      status = "invalid";
      break;
    case TANGO_POSE_UNKNOWN:
      status = "unknown";
      break;
    default:
      status = "status_code_invalid";
      break;
  }

  string_stream.setf(std::ios_base::fixed, std::ios_base::floatfield);
  string_stream.precision(3);
  pthread_mutex_lock(&TangoData::GetInstance().pose_mutex);
  string_stream << "status: " << status
                << ", count: " << TangoData::GetInstance().frame_count[index]
                << ", delta time (ms): "
                << TangoData::GetInstance().frame_delta_time[index]
                << ", position (m): ["
                << TangoData::GetInstance().tango_position[index].x << ", "
                << TangoData::GetInstance().tango_position[index].y << ", "
                << TangoData::GetInstance().tango_position[index].z << "]"
                << ", quat: ["
                << TangoData::GetInstance().tango_rotation[index].x << ", "
                << TangoData::GetInstance().tango_rotation[index].y << ", "
                << TangoData::GetInstance().tango_rotation[index].z << ", "
                << TangoData::GetInstance().tango_rotation[index].w << "]";
  pthread_mutex_unlock(&TangoData::GetInstance().pose_mutex);
  return env->NewStringUTF(string_stream.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_GetVersionString(
    JNIEnv* env, jobject) {
  if (TangoData::GetInstance().lib_version_string.empty()) {
    return env->NewStringUTF("No version string available.");
  } else {
    return env->NewStringUTF(
        TangoData::GetInstance().lib_version_string.c_str());
  }
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_GetEventString(
    JNIEnv* env, jobject) {
  if (TangoData::GetInstance().event_string.empty()) {
    return env->NewStringUTF("No event string available.");
  } else {
    pthread_mutex_lock(&TangoData::GetInstance().event_mutex);
    string event_string_cpy = TangoData::GetInstance().event_string;
    pthread_mutex_unlock(&TangoData::GetInstance().event_mutex);
    return env->NewStringUTF(event_string_cpy.c_str());
  }
}

// Touching GL interface.
JNIEXPORT void JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_StartSetCameraOffset(
    JNIEnv*, jobject) {
  if (cam != NULL) {
    cam_start_angle[0] = cam_cur_angle[0];
    cam_start_angle[1] = cam_cur_angle[1];
    cam_start_dist = cam->GetPosition().z;
  }
}

JNIEXPORT void JNICALL
Java_com_projecttango_areadescriptionnative_TangoJNINative_SetCameraOffset(
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
