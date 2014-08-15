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
  void SetTangoPosition(glm::vec3 position);
  void SetTangoRotation(glm::quat rotation);

 private:
  glm::vec3 tango_position;
  glm::quat tango_rotation;

  TangoConfig* config;
};

#endif  // TangoData_H
