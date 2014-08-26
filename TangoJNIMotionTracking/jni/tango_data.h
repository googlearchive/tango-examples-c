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
  bool SetConfig();
  bool LockConfig();
  bool UnlockConfig();
  bool Connect();
  void Disconnect();

  glm::vec3 GetTangoPosition();
  glm::quat GetTangoRotation();
  char GetTangoPoseStatus();

  void SetTangoPosition(glm::vec3 position);
  void SetTangoRotation(glm::quat rotation);
  void SetTangoPoseStatus(TangoPoseStatusType status);

 private:
  glm::vec3 tango_position_;
  glm::quat tango_rotation_;
  TangoPoseStatusType status_;
  TangoConfig* config_;
};

#endif  // TangoData_H
