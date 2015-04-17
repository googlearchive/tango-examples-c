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
#include "tango-gl/util.h"
#include "tango-gl/video_overlay.h"

#include "tango_data.h"

/// First person camera position and rotation.
/// Position: (0,0,0).
/// Rotation: identity.
const glm::vec3 kFirstPersonCameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::quat kFirstPersonCameraRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

/// Third person camera position and rotation.
/// Position: 1 unit on Y axis, 1 unit on z axis.
/// Rotation: -45 degree around x axis.
const glm::vec3 kThirdPersonCameraPos = glm::vec3(0.0f, 1.0f, 1.0f);
const glm::quat kThirdPersonCameraRot =
    glm::quat(0.92388f, -0.38268f, 0.0f, 0.0f);
/// Top down camera position and rotation.
/// Position: 3 units about origin.
/// Rotation: -90 degree around x axis.
const glm::vec3 kTopDownCameraPos = glm::vec3(0.0f, 3.0f, 0.0f);
const glm::quat kTopDownCameraRot = glm::quat(0.70711f, -0.70711f, 0.0f, 0.0f);

GLuint screen_width;
GLuint screen_height;

tango_gl::Camera* cam;
tango_gl::Axis* axis;
tango_gl::VideoOverlay* video_overlay;

bool InitializeGlContent() {
  cam = new tango_gl::Camera();
  video_overlay = new tango_gl::VideoOverlay();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

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

bool RenderFrame() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  /// Viewport set to full screen, and camera at origin
  /// facing on negative z direction, y axis is the up
  /// vector of the camera.
  glViewport(0, 0, screen_width, screen_height);

  // UpdateColorTexture() updates color camera's
  // texture and timestamp.
  TangoData::GetInstance().UpdateColorTexture();
  video_overlay->Render(glm::mat4(1.0f), glm::mat4(1.0f));

  return true;
}

void SetCamera(int camera_index) {
  switch (camera_index) {
    case 0:
      cam->SetPosition(kFirstPersonCameraPos);
      cam->SetRotation(kFirstPersonCameraRot);
      break;
    case 1:
      cam->SetPosition(kThirdPersonCameraPos);
      cam->SetRotation(kThirdPersonCameraRot);
      break;
    case 2:
      cam->SetPosition(kTopDownCameraPos);
      cam->SetRotation(kTopDownCameraRot);
      break;
    default:
      break;
  }
}

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_initializeGlContent(
    JNIEnv*, jobject) {
  if (!InitializeGlContent()) {
    LOGE("Tango InitializeGlContent failed");
  }
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_initialize(
    JNIEnv* env, jobject, jobject activity) {
  TangoErrorType err = TangoData::GetInstance().Initialize(env, activity);
  if (err != TANGO_SUCCESS) {
    if (err == TANGO_INVALID) {
      LOGE("Tango Service version mismatch");
    } else {
      LOGE("Tango Service initialize internal error");
    }
  }
  return static_cast<int>(err);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_setupConfig(
    JNIEnv*, jobject) {
  if (!TangoData::GetInstance().SetConfig()) {
    LOGE("Tango set config failed");
  }
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_connectTexture(
    JNIEnv*, jobject) {
  TangoData::GetInstance().ConnectTexture(video_overlay->GetTextureId());
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_connect(
    JNIEnv*, jobject) {
  if (!TangoData::GetInstance().Connect()) {
    LOGE("Tango connect failed");
  }
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_disconnect(
    JNIEnv*, jobject) {
  TangoData::GetInstance().Disconnect();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_freeGLContent(
    JNIEnv*, jobject) {
  delete cam;
  delete video_overlay;
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_setupGraphic(
    JNIEnv*, jobject, jint width, jint height) {
  SetupGraphics(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_render(
    JNIEnv*, jobject) {
  RenderFrame();
}

#ifdef __cplusplus
}
#endif
