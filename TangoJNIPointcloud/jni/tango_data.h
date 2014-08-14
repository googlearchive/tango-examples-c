#ifndef TangoData_H
#define TangoData_H
#define GLM_FORCE_RADIANS

#include <tango_client_api.h>

#include "gl_util.h"

class TangoData {
 public:
  static TangoData& GetInstance() {
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
  void SetDepthBuffer(float *buffer);
  int GetDepthBufferSize();
  void SetDepthBufferSize(int size);

 private:
  TangoConfig* config;
  double pointcloud_timestamp;
  float *depth_data_buffer;
  int depth_buffer_size;
};

#endif  // TangoData_H
