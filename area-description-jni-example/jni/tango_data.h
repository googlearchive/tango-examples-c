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
  void SetTangoPoseStatus(int index, int status);
  int GetTangoPoseStatus(int index);
  bool SaveADF();
  void RemoveAllAdfs();
  
  double GetTimestamp(int index);
  void SetTimestamp(int index, double time);
  void SetUUID(char* id);
  char* GetUUID();
  int GetEnableLearning();
  
 private:
  glm::vec3 tango_position_;
  glm::quat tango_rotation_;

  TangoConfig* config_;
  int current_pose_status_[3];
  int is_recording_;
  float current_timestamp_[4];
  char uuid_[5];
  int learning_mode_enabled_;
};

#endif  // TangoData_H
