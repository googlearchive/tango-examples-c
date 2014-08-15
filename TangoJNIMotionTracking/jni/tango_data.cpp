#include "tango_data.h"

TangoData::TangoData() {
}

// This callback function is called when new POSE updates become available,
// pose data must be allocated by the caller.
static void onPoseAvailable(TangoPoseData* pose) {
  TangoData::GetInstance().SetTangoPosition(
      glm::vec3(pose->translation[0], pose->translation[1],
                pose->translation[2]));
  TangoData::GetInstance().SetTangoRotation(
      glm::quat(pose->orientation[3], pose->orientation[0],
                pose->orientation[1], pose->orientation[2]));
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

  if (TangoService_connectOnPoseAvailable(onPoseAvailable) != 0) {
    LOGI("TangoService_connectOnPoseAvailable(): Failed");
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

// Connect to Tango Service, service will start running, and
// POSE can be queried.
bool TangoData::Connect() {
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

void TangoData::SetTangoPosition(glm::vec3 position) {
  tango_position = position;
}

void TangoData::SetTangoRotation(glm::quat rotation) {
  tango_rotation = rotation;
}

TangoData::~TangoData() {
}
