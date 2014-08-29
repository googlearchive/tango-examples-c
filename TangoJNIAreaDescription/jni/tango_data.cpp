#include "tango_data.h"

TangoData::TangoData():config_(nullptr), tango_position_(glm::vec3(0.0f, 0.0f, 0.0f)),
  tango_rotation_(glm::quat(1.0f,0.0f,0.0f,0.0f)){
}

// This callback function is called when new POSE updates become available.
static void onPoseAvailable(const TangoPoseData* pose) {
  TangoData::GetInstance().SetTangoPosition(
      glm::vec3(pose->translation[0], pose->translation[1],
                pose->translation[2]));
  TangoData::GetInstance().SetTangoRotation(
      glm::quat(pose->orientation[3], pose->orientation[0],
                pose->orientation[1], pose->orientation[2]));
  TangoData::GetInstance().SetTangoPoseStatus(pose->status_code);
}

bool TangoData::Initialize() {
  // Initialize Tango Service.
  if (TangoService_initialize() != TANGO_SUCCESS) {
    LOGE("TangoService_initialize(): Failed");
    return false;
  }
  return true;
}

bool TangoData::SetConfig(int is_recording) {
  is_recording_ = is_recording;
  
  // Allocate a TangoConfig object.
  if ((config_ = TangoConfig_alloc()) == NULL) {
    LOGE("TangoService_allocConfig(): Failed");
    return false;
  }

  // Get the default TangoConfig.
  if (TangoService_getConfig(TANGO_CONFIG_DEFAULT, config_) != TANGO_SUCCESS) {
    LOGE("TangoService_getConfig(): Failed");
    return false;
  }

  // Define is recording or loading a map.
  if (is_recording_) {
    if (TangoConfig_setBool(config_, "config_enable_learning_mode", true) != TANGO_SUCCESS) {
      LOGI("config_enable_learning_mode Failed");
      return false;
    }
  }
  else {
    UUID_list uuid_list;
    TangoService_getAreaDescriptionUUIDList(&uuid_list);
    if (TangoConfig_setBool(config_,
                            "config_load_area_description_uuid",
                            uuid_list.uuid[uuid_list.count].data) != TANGO_SUCCESS) {
      LOGI("config_enable_learning_mode Failed");
      return false;
    }
    LOGI("Loaded map: %s", uuid_list.uuid[uuid_list.count].data);
  }
  
  if (TangoService_connectOnPoseAvailable(onPoseAvailable) != TANGO_SUCCESS) {
    LOGI("TangoService_connectOnPoseAvailable(): Failed");
    return false;
  }
  return true;
}

bool TangoData::LockConfig() {
  // Lock in this configuration.
  if (TangoService_lockConfig(config_) != TANGO_SUCCESS) {
    LOGE("TangoService_lockConfig(): Failed");
    return false;
  }
  return true;
}

bool TangoData::UnlockConfig() {
  // Unlock current configuration.
  if (TangoService_unlockConfig() != TANGO_SUCCESS) {
    LOGE("TangoService_unlockConfig(): Failed");
    return false;
  }
  return true;
}

// Connect to Tango Service, service will start running, and
// POSE can be queried.
bool TangoData::Connect() {
  if (TangoService_connect() != TANGO_SUCCESS) {
    LOGE("TangoService_connect(): Failed");
    return false;
  }
  
  TangoCoordinateFramePair pairs;
  if(is_recording_) {
    pairs = {TANGO_COORDINATE_FRAME_START_OF_SERVICE,
             TANGO_COORDINATE_FRAME_DEVICE};
  }
  else {
    pairs = {TANGO_COORDINATE_FRAME_AREA_DESCRIPTION,
             TANGO_COORDINATE_FRAME_DEVICE};
  }
  if (TangoService_setPoseListenerFrames(1, &pairs) != TANGO_SUCCESS) {
    LOGE("TangoService_setPoseListenerFrames(): Failed");
    return false;
  }
  
  UUID_list uuid_list;
  TangoService_getAreaDescriptionUUIDList(&uuid_list);
  
  LOGI("List of maps: ");
  for (int i = 0; i<uuid_list.count; i++) {
    LOGI("%d: %s", i, uuid_list.uuid[i].data);
  }
  
  return true;
}

bool TangoData::SaveADF(){
  UUID uuid;
  if (TangoService_saveAreaDescription(&uuid) != TANGO_SUCCESS) {
    LOGE("TangoService_saveAreaDescription(): Failed");
    return false;
  }
  LOGI("ADF Saved, uuid: %s", uuid.data);
}

void TangoData::RemoveAllAdfs(){
  LOGI("Removing all ADFs");
  UUID_list uuid_list;
  TangoService_getAreaDescriptionUUIDList(&uuid_list);
  TangoService_destroyAreaDescriptionUUIDList(&uuid_list);
  
  TangoService_getAreaDescriptionUUIDList(&uuid_list);
  LOGI("List of maps: ");
  for (int i = 0; i<uuid_list.count; i++) {
    LOGI("%d: %s", i, uuid_list.uuid[i].data);
  }
  LOGI("Removed all ADFs");
}

void TangoData::Disconnect() {
  // Disconnect Tango Service.
  TangoService_disconnect();
}

glm::vec3 TangoData::GetTangoPosition() {
  return tango_position_;
}

glm::quat TangoData::GetTangoRotation() {
  return tango_rotation_;
}

void TangoData::SetTangoPosition(glm::vec3 position) {
  tango_position_ = position;
}

void TangoData::SetTangoRotation(glm::quat rotation) {
  tango_rotation_ = rotation;
}

void TangoData::SetTangoPoseStatus(int status) {
  current_pose_status_ = status;
}

int TangoData::GetTangoPoseStatus() {
  return current_pose_status_;
}
