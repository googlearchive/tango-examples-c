#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>


//#include <tango-api/application-interface.h>
//#include <tango-api/hardware-control-interface.h>
//#include <tango-api/depth-interface.h>
#include "tango_client_api.h"
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

//static const int kMaxVertCount = 61440;
//
//double pointcloud_timestamp = 0.0;
//float depth_data_buffer[kMaxVertCount * 3];
//int depth_buffer_size = kMaxVertCount * 3;

GLuint screen_width;
GLuint screen_height;

Camera cam;
Pointcloud pointcloud;

static int xyz_count = 0;
static GLfloat vertices[30000*3];

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

static void onXYZijAvailable(TangoXYZij *XYZ_ij) {
  float max_point[3] = {-1000, -1000, -1000};
  float min_point[3] = {1000, 1000, 1000};
  xyz_count = XYZ_ij->xyz_count;
  memcpy(vertices, XYZ_ij->xyz, xyz_count*3*sizeof(float));
  for (int i = 0 ; i < xyz_count ; i++) {
   for (int j = 0 ; j < 3 ; j++) {
     max_point[j] = MAX(max_point[j], XYZ_ij->xyz[i][j]);
     min_point[j] = MIN(min_point[j], XYZ_ij->xyz[i][j]);
   }
  }
  LOGI("pointcloud: %lf %d points [%f %f %f] -> [%f %f %f]", XYZ_ij->timestamp,
       xyz_count, max_point[0], max_point[1], max_point[2],
       min_point[0], min_point[1], min_point[2]);
}


bool SetupTango() {
  int i;
	TangoConfig* config;
	if (TangoService_initialize() != 0) {
		LOGI("TangoService_initialize(): Failed");
		return false;
	}
	// Allocate a TangoConfig instance
	if ((config = TangoConfig_alloc()) == NULL) {
		LOGI("TangoService_allocConfig(): Failed");
		return false;
	}
  
	// Report the current TangoConfig
	LOGI("TangoConfig:%s", TangoConfig_toString(config));
  
	// Lock in this configuration
	if(TangoService_lockConfig(config)!=0){
		LOGI("TangoService_lockConfig(): Failed");
		return false;
	}
  
//  LOGI("TangoService_connectOnXYZij");
  // Attach the onXYZijAvailable callback.
	if(TangoService_connectOnXYZijAvailable(onXYZijAvailable)!=0) {
		LOGI("TangoService_connectOnXYZijAvailable(): Failed");
		return false;
	}
	// Connect to the Tango Service
	TangoService_connect();
	return true;
}

bool SetupGraphics(int w, int h) {
  LOGI("setupGraphics(%d, %d)", w, h);
  
  screen_width = w;
  screen_height = h;
  cam.set_aspect_ratio((float)(w/h));

  return true;
}

float a = 0.0f;

bool RenderFrame() {
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  
  glViewport(0, 0, screen_width, screen_height);
  
  a+=0.01f;
  cam.Rotate(a, 0, 1, 0);
  pointcloud.Render(cam.get_projection_view_matrix(), 3 * xyz_count, vertices);
  
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