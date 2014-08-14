#ifndef TangoData_H
#define TangoData_H

#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <tango_client_api.h>

#include "tango_data.h"

#define GLM_FORCE_RADIANS
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "gtc/type_ptr.hpp"

#define  LOG_TAG    "tango_motion_tracking"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

class TangoData{
public:
  static TangoData& GetInstance()
  {
    static TangoData instance;
    return instance;
  }
  TangoData();
  ~TangoData();
  
  bool Initialize();
  bool SetConfig();
  bool LockConfig();
  bool UnlockConfig();
  bool Connect();
  void Disconnect();
  
  float *GetDepthBuffer();
  int GetDepthBufferSize();
  
  glm::vec3 GetTangoPosition();
  glm::quat GetTangoRotation();
  
  //double pointcloud_timestamp;
  //float *depth_data_buffer;
  //int depth_buffer_size;

    glm::vec3 tango_position;
    glm::quat tango_rotation;
private:
  //static const int kMaxVertCount = 61440;
  
  TangoConfig* config;
};

#endif  // TangoData_H
