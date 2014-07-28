#include <android/log.h>
#include <jni.h>

#include <tango_client_api.h>

#define LOG_TAG "hello-tango-jni"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

bool StartTango() {
  if (TangoService_initialize() != 0) {
    LOGE("TangoService_initialize(): Failed");
    return -1;
  }

  // Connect to the Tango Service.
  // Note: connecting Tango service will start the motion
  // tracking automatically.
  if (TangoService_connect() != 0) {
    LOGE("TangoService_connect(): Failed");
    return -1;
  }
  else
  {
    LOGI("HELLO TANGO, SERVICE CONNECTED!");
  }
  return true;
}

JNIEXPORT void JNICALL Java_com_google_tango_hellotangojni_TangoJNINative_init(JNIEnv * env, jobject obj)
{
  StartTango();
}
