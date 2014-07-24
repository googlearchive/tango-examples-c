#include <jni.h>
#include <android/log.h>
#include <tango_client_api.h>

#define  LOG_TAG    "hello-tango-jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

bool SetupTango() {
  if (TangoService_initialize() != 0) {
    LOGE("TangoService_initialize(): Failed\n");
    return -1;
  }

  // Connect to the Tango Service.
  if (TangoService_connect() != 0) {
    LOGE("TangoService_connect(): Failed\n");
    return -1;
  }
  else
  {
    LOGI("HELLO TANGO, SERVICE CONNECTED!\n");
  }
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
