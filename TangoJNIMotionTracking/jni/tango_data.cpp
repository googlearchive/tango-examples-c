#include "tango_data.h"

TangoData::TangoData() {

}

static void onPoseAvailable(TangoPoseData *pose) {
  TangoData::GetInstance().tango_position = glm::vec3(pose->translation[0],
                                                      pose->translation[1],
                                                      pose->translation[2]);
  TangoData::GetInstance().tango_rotation = glm::quat(pose->orientation[3],
                                                      pose->orientation[0],
                                                      pose->orientation[1],
                                                      pose->orientation[2]);
//    LOGI("%4.2f,%4.2f,%4.2f",pose->translation[0],pose->translation[1],pose->translation[2]);
}

bool TangoData::Initialize() {
  // Initialize Tango Service.
  if (TangoService_initialize() != 0) {
    LOGE("TangoService_initialize(): Failed");
    return false;
  }
  return true;
}

bool TangoData::SetConfig() {
  // Allocate a TangoConfig object.
  if ((config = TangoConfig_alloc()) == NULL) {
    LOGE("TangoService_allocConfig(): Failed");
    return false;
  }

  // Get the default TangoConfig.
  if (TangoService_getConfig(TANGO_CONFIG_DEFAULT, config) != 0) {
    LOGE("TangoService_getConfig(): Failed");
    return false;
  }

  // Enable depth.
  if (TangoConfig_setBool(config, "config_enable_depth", true) != 0) {
    LOGI("config_enable_depth Failed");
    return false;
  }

  if (TangoService_connectOnPoseAvailable(onPoseAvailable) != 0) {
    LOGI("TangoService_connectOnXYZijAvailable(): Failed");
    return false;
  }

  return true;
}

bool TangoData::LockConfig() {
  // Lock in this configuration.
  if (TangoService_lockConfig(config) != 0) {
    LOGE("TangoService_lockConfig(): Failed");
    return false;
  }
  return true;
}

bool TangoData::UnlockConfig() {
  // Unlock current configuration.
  if (TangoService_unlockConfig() != 0) {
    LOGE("TangoService_unlockConfig(): Failed");
    return false;
  }
  return true;
}

bool TangoData::Connect() {
  // Connect to the Tango Service.
  // Note: connecting Tango service will start the motion
  // tracking automatically.
  if (TangoService_connect() != 0) {
    LOGE("TangoService_connect(): Failed");
    return false;
  }
  return true;
}

void TangoData::Disconnect() {
  // Disconnect Tango Service.
  TangoService_disconnect();
}

glm::vec3 TangoData::GetTangoPosition() {
  return tango_position;
}

glm::quat TangoData::GetTangoRotation() {
  return tango_rotation;
}

TangoData::~TangoData() {
}
