#ifndef TANGO_DATA_H
#define TANGO_DATA_H
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

  float* GetDepthBuffer();
  void SetDepthBuffer(float *buffer);
  int GetDepthBufferSize();
  void SetDepthBufferSize(int size);

 private:
  TangoConfig* config_;
  double pointcloud_timestamp_;
  float* depth_data_buffer_;
  int depth_buffer_size_;
};

#endif  // TANGO_DATA_H
