#ifndef TangoData_H
#define TangoData_H

#include <tango_client_api.h>
#include "gl_util.h"

class TangoData {
 public:
  static TangoData& GetInstance() {
    static TangoData instance;
    return instance;
  }
  TangoData();

  bool Initialize();
  bool SetConfig(bool is_learning, bool is_load_adf);
  bool LockConfig();
  bool UnlockConfig();
  bool Connect();
  void Disconnect();

  bool SaveADF();
  void RemoveAllAdfs();
  
  void LogAllUUIDs();
  
  // 0: device_wrt_start
  // 1: device_wrt_adf
  // 2: start_wrt_adf
  // 3: adf_wrt_start
  glm::vec3 tango_position_[3];
  glm::quat tango_rotation_[3];
  float current_timestamp_[3];
  int current_pose_status_[3];
  
  bool is_learning_mode_enabled;
  bool is_relocalized;
  
  char uuid_[5];
  
 private:
  TangoConfig* config_;
};

#endif  // TangoData_H
