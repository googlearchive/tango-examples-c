#include "tango_data.h"

TangoData::TangoData(){
  pointcloud_timestamp = 0.0;
  depth_data_buffer = new float[61440 * 3];
  depth_buffer_size = kMaxVertCount * 3;
  
  LOGI("tango data constructed");
}

static void onXYZijAvailable(TangoXYZij *XYZ_ij){
  memcpy(TangoData::GetInstance().depth_data_buffer, XYZ_ij->xyz, XYZ_ij->xyz_count*3*sizeof(float));
  TangoData::GetInstance().depth_buffer_size = XYZ_ij->xyz_count * 3;
  TangoData::GetInstance().pointcloud_timestamp = XYZ_ij->timestamp;
}

void TangoData::onPoseAvailable(TangoPoseData *pose) {
  tango_position = glm::vec3(pose->translation[0],
                             pose->translation[1],
                             pose->translation[2]);
  tango_rotation = glm::quat(pose->orientation[3],
                             pose->orientation[0],
                             pose->orientation[1],
                             pose->orientation[2]);
}

bool TangoData::SetupTango() {
//  TangoConfig* config;
//
//	if (TangoService_initialize() != 0) {
//		LOGI("TangoService_initialize(): Failed");
//		return false;
//  }
//  
//	// Allocate a TangoConfig instance
//	if ((config = TangoConfig_alloc()) == NULL) {
//		LOGI("TangoService_allocConfig(): Failed");
//		return false;
//	}
//  
//	// Report the current TangoConfig
//	LOGI("TangoConfig:%s", TangoConfig_toString(config));
//  
//	// Lock in this configuration
//	if(TangoService_lockConfig(config)!=0){
//		LOGI("TangoService_lockConfig(): Failed");
//		return false;
//	}
//  
//  // Attach the onXYZijAvailable callback.
//	if(TangoService_connectOnXYZijAvailable(onXYZijAvailable)!=0) {
//		LOGI("TangoService_connectOnXYZijAvailable(): Failed");
//		return false;
//	}
//	// Connect to the Tango Service
//	TangoService_connect();
	return true;
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