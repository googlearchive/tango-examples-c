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

#include <string.h>
#include <jni.h>

#include "axis.h"
#include "grid.h"
#include "camera.h"
#include "gl_util.h"
#include "pointcloud.h"
#include "tango_data.h"
#include "transform.h"

/// First person camera position and rotation.
/// Position: (0,0,0).
/// Rotation: identity.
const glm::vec3 kFirstPersonCameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::quat kFirstPersonCameraRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

/// Third person camera position and rotation.
/// Position: 1 unit on Y axis, 1 unit on z axis.
/// Rotation: -45 degree around x axis.
const glm::vec3 kThirdPersonCameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
const glm::quat kThirdPersonCameraRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

/// Top down camera position and rotation.
/// Position: 3 units about origin.
/// Rotation: -90 degree around x axis.
const glm::vec3 kTopDownCameraPos = glm::vec3(0.0f, 5.0f, 0.0f);
const glm::quat kTopDownCameraRot = glm::quat(0.70711f, -0.70711f, 0.0f, 0.0f);

// Screen size.
GLuint screen_width;
GLuint screen_height;

// Render camera's parent transformation.
// This object is a pivot transformtion for render camera to rotate around.
Transform* cam_parent_transform;

// Render camera.
Camera* cam;

// Point cloud drawable object.
Pointcloud* pointcloud;

// Axis is representing the pose from the Tango device.
Axis* axis;

// Grid on the ground.
// Each block is 1 meter by 1 meter in real world.
Grid* grid;

int cur_cam_index = 0;

float cam_start_angle[2];
float cam_cur_angle[2];
float cam_start_dist;
float cam_cur_dist;

bool SetupGraphics(int w, int h) {
  screen_width = w;
  screen_height = h;

  cam = new Camera();
  pointcloud = new Pointcloud();
  axis = new Axis();
  grid = new Grid();
  cam_parent_transform = new Transform();
  
  // Set the parent-child camera transfromation.
  cam->SetParent(cam_parent_transform);
  
  // Set up the aspect ratio for proper projection matrix.
  cam->SetAspectRatio((float)w / (float)h);

  // Put the grid at the resonable height since the motion
  // tracking pose always starts at (0, 0, 0).
  grid->SetPosition(glm::vec3(0.0f, -1.5f, 0.0f));
  
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);

  return true;
}

bool RenderFrame() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  
  /// Viewport set to full screen, and camera at origin
  /// facing on negative z direction, y axis is the up
  /// vector of the camera.
  glViewport(0, 0, screen_width, screen_height);
  
  // Get motion transformation.
  glm::mat4 oc_2_ow_mat_motion;
  
  // Get depth frame transfromation.
  glm::mat4 oc_2_ow_mat_depth;
  
  if (cur_cam_index != 0) {
    // Get parent camera's rotation from touch.
    // Note that the render camera is a child transformation
    // of the this transformation.
    // cam_cur_angle[0] is the x-axis touch, cooresponding to y-axis rotation.
    // cam_cur_angle[0] is the y-axis touch, cooresponding to x-axis rotation.
    glm::quat parent_cam_rot =
      glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), -cam_cur_angle[0], glm::vec3(0, 1, 0));
    parent_cam_rot = glm::rotate(parent_cam_rot, -cam_cur_angle[1], glm::vec3(1, 0, 0));
    
    // Get motion transformation.
    oc_2_ow_mat_motion = TangoData::GetInstance().GetOC2OWMat(false);
    
    // Get depth frame transfromation.
    oc_2_ow_mat_depth = TangoData::GetInstance().GetOC2OWMat(true);
    
    // Set render camera parent position and rotation.
    cam_parent_transform->SetRotation(parent_cam_rot);
    cam_parent_transform->SetPosition(GlUtil::GetTranslationFromMatrix(oc_2_ow_mat_motion));
    
    // Set camera view distance, based on touch interaction.
    cam->SetPosition(glm::vec3(0.0f,0.0f, cam_cur_dist));
  }
  else {
    // Get motion transformation.
    oc_2_ow_mat_motion = TangoData::GetInstance().GetOC2OWMat(false);
    // Set camera's pose to motion tracking's pose.
    cam->SetTransformationMatrix(oc_2_ow_mat_motion);
  }
  
  // Set axis transformation, axis representing device's pose.
  axis->SetTransformationMatrix(oc_2_ow_mat_motion);
  axis->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());

  // Render point cloud based on depth buffer and depth frame transformation.
  pointcloud->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix(),
                     oc_2_ow_mat_depth,
                     TangoData::GetInstance().GetDepthBufferSize(),
                     TangoData::GetInstance().GetDepthBuffer());
  
  // Render grid.
  grid->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());
  return true;
}

void SetCamera(int camera_index) {
  cur_cam_index = camera_index;
  cam_cur_angle[0] = cam_cur_angle[1] = cam_cur_dist = 0.0f;
  switch (camera_index) {
    case 0:
      cam_parent_transform->SetPosition(kFirstPersonCameraPos);
      cam_parent_transform->SetRotation(kFirstPersonCameraRot);
      break;
    case 1:
      cam_parent_transform->SetRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
      cam_cur_dist = 4.0f;
      cam_cur_angle[0] = -0.785f;
      cam_cur_angle[1] = 0.785f;
      break;
    case 2:
      cam_parent_transform->SetRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
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
JNIEXPORT void JNICALL Java_com_projecttango_pointcloudnative_TangoJNINative_OnCreate(
    JNIEnv* env, jobject obj) {
  LOGI("In onCreate: Initialing and setting config");
  if (!TangoData::GetInstance().Initialize())
  {
    LOGE("Tango initialization failed");
  }
  if (!TangoData::GetInstance().SetConfig())
  {
    LOGE("Tango set config failed");
  }
}

JNIEXPORT void JNICALL Java_com_projecttango_pointcloudnative_TangoJNINative_OnResume(
    JNIEnv* env, jobject obj) {
  LOGI("In OnResume: Locking config and connecting service");
  if (TangoData::GetInstance().LockConfig()){
    LOGE("Tango lock config failed");
  }
  if (TangoData::GetInstance().Connect()){
    LOGE("Tango connect failed");
  }
}

JNIEXPORT void JNICALL Java_com_projecttango_pointcloudnative_TangoJNINative_OnPause(
    JNIEnv* env, jobject obj) {
  LOGI("In OnPause: Unlocking config and disconnecting service");
  if (TangoData::GetInstance().UnlockConfig()){
    LOGE("Tango unlock file failed");
  }
  TangoData::GetInstance().Disconnect();
}

JNIEXPORT void JNICALL Java_com_projecttango_pointcloudnative_TangoJNINative_OnDestroy(
    JNIEnv* env, jobject obj) {
  delete cam;
  delete pointcloud;
  delete axis;
  delete cam_parent_transform;
}

JNIEXPORT void JNICALL Java_com_projecttango_pointcloudnative_TangoJNINative_SetupGraphic(
    JNIEnv* env, jobject obj, jint width, jint height) {
  SetupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_com_projecttango_pointcloudnative_TangoJNINative_Render(
    JNIEnv* env, jobject obj) {
  RenderFrame();
  TangoData::GetInstance().GetVersonString();
}

JNIEXPORT void JNICALL Java_com_projecttango_pointcloudnative_TangoJNINative_SetCamera(
    JNIEnv* env, jobject obj, int camera_index) {
  SetCamera(camera_index);
}

JNIEXPORT jstring JNICALL Java_com_projecttango_pointcloudnative_TangoJNINative_GetVersionNumber(
    JNIEnv* env, jobject obj) {
  return (env)->NewStringUTF(TangoData::GetInstance().GetVersonString());
}
  
JNIEXPORT jint JNICALL Java_com_projecttango_pointcloudnative_TangoJNINative_GetVerticesCount(
    JNIEnv* env, jobject obj) {
  return TangoData::GetInstance().GetDepthBufferSize()/3;
}

JNIEXPORT float JNICALL Java_com_projecttango_pointcloudnative_TangoJNINative_GetAverageZ(
     JNIEnv* env, jobject obj) {
  return TangoData::GetInstance().average_depth;
}
  
JNIEXPORT float JNICALL Java_com_projecttango_pointcloudnative_TangoJNINative_GetDepthFPS(
     JNIEnv* env, jobject obj) {
  return TangoData::GetInstance().depth_frame_delta_time;
}

JNIEXPORT void JNICALL Java_com_projecttango_pointcloudnative_TangoJNINative_StartSetCameraOffset(
    JNIEnv* env, jobject obj) {
  cam_start_angle[0] = cam_cur_angle[0];
  cam_start_angle[1] = cam_cur_angle[1];
  cam_start_dist = cam->GetPosition().z;
}

JNIEXPORT void JNICALL Java_com_projecttango_pointcloudnative_TangoJNINative_SetCameraOffset(
     JNIEnv* env, jobject obj, float rotation_x, float rotation_y, float dist) {
  LOGI("dist = %f", dist);
  cam_cur_angle[0] = cam_start_angle[0] + rotation_x;
  cam_cur_angle[1] = cam_start_angle[1] + rotation_y;
  cam_cur_dist = cam_start_dist + dist*5.0f;
}
#ifdef __cplusplus
}
#endif
