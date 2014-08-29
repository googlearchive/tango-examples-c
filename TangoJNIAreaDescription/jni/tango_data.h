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
  bool SetConfig(int is_recording);
  bool LockConfig();
  bool UnlockConfig();
  bool Connect();
  void Disconnect();

  glm::vec3 GetTangoPosition();
  glm::quat GetTangoRotation();
  void SetTangoPosition(glm::vec3 position);
  void SetTangoRotation(glm::quat rotation);
  void SetTangoPoseStatus(int status);
  int GetTangoPoseStatus();
  bool SaveADF();
  void RemoveAllAdfs();
  
 private:
  glm::vec3 tango_position_;
  glm::quat tango_rotation_;

  TangoConfig* config_;
  int current_pose_status_;
  int is_recording_;
};

#endif  // TangoData_H
