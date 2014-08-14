#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "tango_data.h"
#include "camera.h"
#include "gl_util.h"
#include "axis.h"
#include "grid.h"
#include "frustum.h"
#include "trace.h"

#define GLM_FORCE_RADIANS
#include "glm.hpp"

#define  LOG_TAG    "tango_motion_tracking"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

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

bool RenderFrame() {
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glViewport(0, 0, screen_width, screen_height);

  grid->Render(cam->GetCurrentProjectionViewMatrix());

  glm::vec3 position = GlUtil::CorrectPosition(
      TangoData::GetInstance().GetTangoPosition());
  glm::quat rotation = GlUtil::CorrectRotation(
      TangoData::GetInstance().GetTangoRotation());

  if (camera_type != FIRST_PERSON) {
    frustum->SetPosition(position);
    trace->UpdateVerticesArray(position);
    trace->Render(cam->GetCurrentProjectionViewMatrix());

    frustum->SetRotation(rotation);
    frustum->Render(cam->GetCurrentProjectionViewMatrix());
  } else {
    cam->SetPosition(position);
    cam->SetRotation(rotation);
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
      cam->SetPosition(glm::vec3(0.0f, 3.0f, 3.0f));
      cam->SetRotation(glm::quat(0.92388f, -0.38268f, 0.0f, 0.0f));
      LOGI("setting to Third Person Camera");
      break;
    case TOP_DOWN:
      cam->SetPosition(glm::vec3(0.0f, 3.0f, 0.0f));
      cam->SetRotation(glm::quat(0.70711f, -0.70711f, 0.0f, 0.0f));
      LOGI("setting to Top Down Camera");
      break;
    default:
      break;
  }
}

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_com_google_tango_tangojnimotiontracking_TangoJNINative_OnCreate(
    JNIEnv * env, jobject obj) {
  TangoData::GetInstance().Initialize();
  TangoData::GetInstance().SetConfig();
}

JNIEXPORT void JNICALL Java_com_google_tango_tangojnimotiontracking_TangoJNINative_OnResume(
    JNIEnv * env, jobject obj) {
  TangoData::GetInstance().LockConfig();
  TangoData::GetInstance().Connect();
}

JNIEXPORT void JNICALL Java_com_google_tango_tangojnimotiontracking_TangoJNINative_OnPause(
    JNIEnv * env, jobject obj) {
  TangoData::GetInstance().UnlockConfig();
  TangoData::GetInstance().Disconnect();
}

JNIEXPORT void JNICALL Java_com_google_tango_tangojnimotiontracking_TangoJNINative_OnDestroy(
    JNIEnv * env, jobject obj) {
  delete cam;
  delete grid;
}

JNIEXPORT void JNICALL Java_com_google_tango_tangojnimotiontracking_TangoJNINative_SetupGraphic(
    JNIEnv * env, jobject obj, jint width, jint height) {
  SetupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_com_google_tango_tangojnimotiontracking_TangoJNINative_Render(
    JNIEnv * env, jobject obj) {
  RenderFrame();
}

JNIEXPORT void JNICALL Java_com_google_tango_tangojnimotiontracking_TangoJNINative_SetCamera(
    JNIEnv * env, jobject obj, int camera_index) {
  SetCamera(camera_index);
}
#ifdef __cplusplus
}
#endif
