#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>


#include <tango-api/application-interface.h>
#include <tango-api/hardware-control-interface.h>
#include <tango-api/depth-interface.h>

#include "camera.h"
#include "gl_util.h"
#include "pointcloud.h"

#define GLM_FORCE_RADIANS
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "gtc/type_ptr.hpp"

#define  LOG_TAG    "tango_pointcloud"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static const int kMaxVertCount = 61440;

double pointcloud_timestamp = 0.0;
float depth_data_buffer[kMaxVertCount * 3];
int depth_buffer_size = kMaxVertCount * 3;

application_handle_t *app_handler;

GLuint screen_width;
GLuint screen_height;

Camera cam;
Pointcloud pointcloud;

bool SetupTango() {
  app_handler = ApplicationInitialize("[Superframes Small-Peanut]", 1);
  if (app_handler == NULL) {
    LOGI("Application initialize failed\n");
    return false;
  }
  CAPIErrorCodes ret_error;
  if ((ret_error = DepthStartBuffering(app_handler)) != kCAPISuccess) {
    LOGI("DepthStartBuffering failed: %d\n", ret_error);
    return false;
  }
  return true;
}

bool SetupGraphics(int w, int h) {
  LOGI("setupGraphics(%d, %d)", w, h);
  
  screen_width = w;
  screen_height = h;
  cam.set_aspect_ratio((float)(w/h));

  return true;
}

bool UpdateTango() {
  ApplicationDoStep(app_handler);
  
  depth_buffer_size = kMaxVertCount * 3;
  pointcloud_timestamp = 0.0f;
  CAPIErrorCodes ret_error;
  if ((ret_error = DepthGetPointCloudUnity(
                    app_handler, &pointcloud_timestamp,
                    0.5f, depth_data_buffer, &depth_buffer_size)) != kCAPISuccess) {
    LOGI("DepthGetPointCloud failed: %d\n", ret_error);
    return false;
  }
  return true;
}

float a = 0.0f;

bool RenderFrame() {
  UpdateTango();
  
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  
  glViewport(0, 0, screen_width, screen_height);
  
  depth_buffer_size = glm::clamp(depth_buffer_size, 0, kMaxVertCount);
  for (int i = 0; i < 3 * depth_buffer_size; i++) {
    depth_data_buffer[i] = depth_data_buffer[i] * 0.001f;
  }
  
  a+=0.01f;
  cam.Rotate(a, 0, 1, 0);
  pointcloud.Render(cam.get_projection_view_matrix(), 3 * depth_buffer_size, depth_data_buffer);
  
  return true;
}

#ifdef __cplusplus
extern "C" {
#endif
  JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_init(
    JNIEnv * env, jobject obj, jint width, jint height){
    SetupGraphics(width, height);
    SetupTango();
  }
  
  JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_render(
    JNIEnv * env, jobject obj){
    RenderFrame();
  }
#ifdef __cplusplus
}
#endif