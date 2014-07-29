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

bool StartTango() {
  // Initialize Tango Service.
  if (TangoService_initialize() != 0) {
    LOGE("TangoService_initialize(): Failed");
    return false;
  }
  
  // Allocate a TangoConfig object.
  if ((config = TangoConfig_alloc()) == NULL) {
    LOGE("TangoService_allocConfig(): Failed\n");
    return false;
  }
  
  // Get the default TangoConfig.
  if (TangoService_getConfig(TANGO_CONFIG_DEFAULT, config) != 0) {
    LOGE("TangoService_getConfig(): Failed\n");
    return false;
  }
  
  // Lock in this configuration.
  if (TangoService_lockConfig(config) != 0) {
    LOGE("TangoService_lockConfig(): Failed\n");
    return false;
  }

  // Connect to the Tango Service.
  // Note: connecting Tango service will start the motion
  // tracking automatically.
  if (TangoService_connect() != 0) {
    LOGE("TangoService_connect(): Failed");
    return false;
  }

  LOGI("HELLO TANGO, SERVICE CONNECTED!");
  
  return true;
}

bool LockConfig()
{
  // Lock in this configuration.
  if (TangoService_lockConfig(config) != 0) {
    LOGE("TangoService_lockConfig(): Failed\n");
    return false;
  }
  return true;
}

bool UnlockConfig()
{
  // Unlock current configuration.
  if (TangoService_unlockConfig() != 0) {
    LOGE("TangoService_unlockConfig(): Failed\n");
    return false;
  }
  return true;
}

void DisconnectTango()
{
  // Disconnect Tango Service.
  TangoService_disconnect();
}

JNIEXPORT void JNICALL Java_com_google_tango_hellotangojni_TangoJNINative_onCreate(JNIEnv * env, jobject obj)
{
  StartTango();
}

JNIEXPORT void JNICALL Java_com_google_tango_hellotangojni_TangoJNINative_onResume(JNIEnv * env, jobject obj)
{
  LockConfig();
}

JNIEXPORT void JNICALL Java_com_google_tango_hellotangojni_TangoJNINative_onPause(JNIEnv * env, jobject obj)
{
  UnlockConfig();
}

JNIEXPORT void JNICALL Java_com_google_tango_hellotangojni_TangoJNINative_onDestory(JNIEnv * env, jobject obj)
{
  DisconnectTango();
}
