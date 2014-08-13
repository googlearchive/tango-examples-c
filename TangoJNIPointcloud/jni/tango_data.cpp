#include "tango_data.h"

TangoData::TangoData(){
  pointcloud_timestamp = 0.0;
  depth_data_buffer = new float[61440 * 3];
  depth_buffer_size = kMaxVertCount * 3;
}

static void onXYZijAvailable(TangoXYZij *XYZ_ij){
  memcpy(TangoData::GetInstance().depth_data_buffer, XYZ_ij->xyz, XYZ_ij->xyz_count*3*sizeof(float));
  TangoData::GetInstance().depth_buffer_size = XYZ_ij->xyz_count * 3;
  TangoData::GetInstance().pointcloud_timestamp = XYZ_ij->timestamp;
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
  
  // Attach the onXYZijAvailable callback.
	if(TangoService_connectOnXYZijAvailable(onXYZijAvailable)!=0) {
		LOGI("TangoService_connectOnXYZijAvailable(): Failed");
		return false;
	}
  
  return true;
}

bool TangoData::LockConfig()
{
  // Lock in this configuration.
  if (TangoService_lockConfig(config) != 0) {
    LOGE("TangoService_lockConfig(): Failed");
    return false;
  }
  return true;
}

bool TangoData::UnlockConfig()
{
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

void TangoData::Disconnect()
{
  // Disconnect Tango Service.
  TangoService_disconnect();
}


float* TangoData::GetDepthBuffer(){
  return depth_data_buffer;
}

int TangoData::GetDepthBufferSize(){
  return depth_buffer_size;
}

glm::vec3 TangoData::GetTangoPosition(){
  return tango_position;
}

glm::quat TangoData:: GetTangoRotation(){
  return tango_rotation;
}

TangoData::~TangoData()
{
  delete[] depth_data_buffer;
}