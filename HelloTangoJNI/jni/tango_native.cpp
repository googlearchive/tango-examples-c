#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include <tango_client_api.h>

#define  LOG_TAG    "hello-tango-jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

bool SetupTango() {
  TangoConfig* config;
  if (TangoService_initialize() != 0) {
    LOGE("TangoService_initialize(): Failed\n");
    return -1;
  }
  
  // Allocate a TangoConfig object.
  if ((config = TangoConfig_alloc()) == NULL) {
    LOGE("TangoService_allocConfig(): Failed\n");
    return -1;
  }
  
  // Get the default TangoConfig.
  if (TangoService_getConfig(TANGO_CONFIG_DEFAULT, config) != 0) {
    LOGE("TangoService_getConfig(): Failed\n");
    return -1;
  }
  
  // Set a parameter in TangoConfig.
  if (TangoConfig_setBool(config, "disable_opengl", true) != 0) {
    LOGE("TangoConfig_setBool(\"disable_opengl\", true): Failed\n");
    return -1;
  }
  
  // Report the current TangoConfig.
  LOGI("TangoConfig:\n%s\n", TangoConfig_toString(config));
  
  // Lock in this configuration.
  if (TangoService_lockConfig(config) != 0) {
    LOGE("TangoService_lockConfig(): Failed\n");
    return -1;
  }

  // Connect to the Tango Service.
  TangoService_connect();

  LOGI("HELLO TANGO, SERVICE CONNECTED!\n");
  
  return true;
}

#ifdef __cplusplus
extern "C" {
#endif
  JNIEXPORT void JNICALL Java_com_google_tango_hellotangojni_TangoJNINative_init(
     JNIEnv * env, jobject obj, jint width, jint height)
  {
    SetupTango();
  }
#ifdef __cplusplus
}
#endif
