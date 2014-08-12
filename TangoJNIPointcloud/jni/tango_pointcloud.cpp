#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "tango_data.h"
#include "camera.h"
#include "gl_util.h"
#include "pointcloud.h"
#include "axis.h"
#include "grid.h"

#define GLM_FORCE_RADIANS
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "gtc/type_ptr.hpp"

#define  LOG_TAG    "tango_pointcloud"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

GLuint screen_width;
GLuint screen_height;

Camera cam;
Pointcloud pointcloud;
Axis axis;
Grid grid;

bool SetupGraphics(int w, int h) {
  LOGI("setupGraphics(%d, %d)", w, h);
  
  screen_width = w;
  screen_height = h;
  cam.SetAspectRatio((float)(w/h));

  return true;
}

float a = 0.0f;
bool RenderFrame() {
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  
  glViewport(0, 0, screen_width, screen_height);
  
  grid.Render(cam.GetCurrentProjectionViewMatrix());
  
//  cam.SetPosition(TangoData::GetInstance().GetTangoPosition());
//  cam.SetRotation(glm::quat(1,0,0,0));
//  cam.SetRotation(TangoData::GetInstance().GetTangoRotation());
  
//  a+=0.001f;
//  cam.SetPosition(glm::vec3(0.0, a, 2.0));
//  cam.SetRotation(glm::quat(0,0,0,0));
//  cam.SetRotation(glm::quat(0.90631f, 0.0f, 0.42262f, 0.0f));
  
//  pointcloud.Render(cam.GetCurrentProjectionViewMatrix(), TangoData::GetInstance().GetDepthBufferSize(), TangoData::GetInstance().GetDepthBuffer());
  
//  a += 0.01f;
  axis.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
//  axis.SetRotation(glm::quat(0.99905, 0.0f, 0.04362f, 0.0f));
//  axis.Rotate(glm::quat(0.00000001f, 0.0f, 1.0f, 0.0f));
//  axis.SetScale(glm::vec3(2.0f, 2.0f, 2.0f));
  axis.Render(cam.GetCurrentProjectionViewMatrix());
  return true;
}

void SetCamera(int camera_index)
{
  switch (camera_index) {
    case 0:
      cam.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
      cam.SetRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
      LOGI("setting to third cam");
      break;
    case 1:
      cam.SetPosition(glm::vec3(0.0f, 3.0f, 3.0f));
      cam.SetRotation(glm::quat(0.92388f, -0.38268f, 0.0f, 0.0f));
      LOGI("setting to third cam");
      break;
    case 2:
      cam.SetPosition(glm::vec3(0.0f, 3.0f, 0.0f));
      cam.SetRotation(glm::quat(0.70711f, -0.70711f, 0.0f, 0.0f));
      LOGI("setting to top down cam");
      break;
    default:
      break;
  }
}

#ifdef __cplusplus
extern "C" {
#endif
  JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_init(
    JNIEnv * env, jobject obj, jint width, jint height){
    SetupGraphics(width, height);
    TangoData::GetInstance().SetupTango();
  }
  
  JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_render(
    JNIEnv * env, jobject obj){
    RenderFrame();
  }
  
  JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_SetCamera(
    JNIEnv * env, jobject obj, int camera_index){
    SetCamera(camera_index);
  }
#ifdef __cplusplus
}
#endif