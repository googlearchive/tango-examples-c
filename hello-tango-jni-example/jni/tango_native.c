#include <android/log.h>
#include <jni.h>

#include <tango_client_api.h>

#define LOG_TAG "hello-tango-jni"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

// Tango configuration file.
// Configuration file describs current states of
// Tango Service.
TangoConfig* config;

bool TangoInitialize() {
  // Initialize Tango Service.
  if (TangoService_initialize() != TANGO_SUCCESS) {
    LOGE("TangoService_initialize(): Failed");
    return false;
  }
  return true;
}

bool TangoSetConfig() {
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
  
  return true;
}

bool TangoLockConfig()
{
  // Lock in this configuration.
  if (TangoService_lockConfig(config) != TANGO_SUCCESS) {
    LOGE("TangoService_lockConfig(): Failed");
    return false;
  }
  return true;
}

bool TangoUnlockConfig()
{
  // Unlock current configuration.
  if (TangoService_unlockConfig() != TANGO_SUCCESS) {
    LOGE("TangoService_unlockConfig(): Failed");
    return false;
  }
  return true;
}

bool TangoConnect() {
  // Connect to the Tango Service.
  // Note: connecting Tango service will start the motion
  // tracking automatically.
  if (TangoService_connect() != TANGO_SUCCESS) {
    LOGE("TangoService_connect(): Failed");
    return false;
  }
  LOGI("HELLO TANGO, SERVICE CONNECTED!");
  return true;
}

void DisconnectTango()
{
  // Disconnect Tango Service.
  TangoService_disconnect();
}

JNIEXPORT void JNICALL Java_com_google_tango_hellotangojni_TangoJNINative_onCreate(JNIEnv * env, jobject obj)
{
  TangoInitialize();
  TangoSetConfig();
}

JNIEXPORT void JNICALL Java_com_google_tango_hellotangojni_TangoJNINative_onResume(JNIEnv * env, jobject obj)
{
  TangoLockConfig();
  TangoConnect();
}

JNIEXPORT void JNICALL Java_com_google_tango_hellotangojni_TangoJNINative_onPause(JNIEnv * env, jobject obj)
{
  TangoUnlockConfig();
  DisconnectTango();
}
