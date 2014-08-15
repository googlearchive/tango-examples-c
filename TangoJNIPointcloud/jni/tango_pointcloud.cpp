#define GLM_FORCE_RADIANS

#include "axis.h"
#include "camera.h"
#include "grid.h"
#include "gl_util.h"
#include "glm.hpp"
#include "pointcloud.h"
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
const glm::quat kThirdPersonCameraRot = glm::quat(0.92388f, -0.38268f, 0.0f,
                                                  0.0f);
/// Top down camera position and rotation.
/// Position: 3 units about origin.
/// Rotation: -90 degree around x axis.
const glm::vec3 kTopDownCameraPos = glm::vec3(0.0f, 3.0f, 0.0f);
const glm::quat kTopDownCameraRot = glm::quat(0.70711f, -0.70711f, 0.0f, 0.0f);

GLuint screen_width;
GLuint screen_height;

Camera* cam;
Pointcloud* pointcloud;
Axis* axis;
Grid* grid;

bool SetupGraphics(int w, int h) {
  screen_width = w;
  screen_height = h;

  cam = new Camera();
  pointcloud = new Pointcloud();
  axis = new Axis();
  grid = new Grid();

  cam->SetAspectRatio((float)w / (float)h);

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
  
  grid->SetPosition(glm::vec3(0.0f, -1.0f, 0.0f));
  grid->Render(cam->GetCurrentProjectionViewMatrix());
  axis->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
  axis->Render(cam->GetCurrentProjectionViewMatrix());

  pointcloud->Render(cam->GetCurrentProjectionViewMatrix(),
                     TangoData::GetInstance().GetDepthBufferSize(),
                     TangoData::GetInstance().GetDepthBuffer());

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
JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_OnCreate(
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

JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_OnResume(
    JNIEnv* env, jobject obj) {
  LOGI("In OnResume: Locking config and connecting service");
  if (TangoData::GetInstance().LockConfig()){
    LOGE("Tango lock config failed");
  }
  if (TangoData::GetInstance().Connect()){
    LOGE("Tango connect failed");
  }
}

JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_OnPause(
    JNIEnv* env, jobject obj) {
  LOGI("In OnPause: Unlocking config and disconnecting service");
  if (TangoData::GetInstance().UnlockConfig()){
    LOGE("Tango unlock file failed");
  }
  TangoData::GetInstance().Disconnect();
}

JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_OnDestroy(
    JNIEnv* env, jobject obj) {
  delete cam;
  delete pointcloud;
  delete axis;
  delete grid;
}

JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_SetupGraphic(
    JNIEnv* env, jobject obj, jint width, jint height) {
  SetupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_Render(
    JNIEnv* env, jobject obj) {
  RenderFrame();
}

JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_SetCamera(
    JNIEnv* env, jobject obj, int camera_index) {
  SetCamera(camera_index);
}
#ifdef __cplusplus
}
#endif
