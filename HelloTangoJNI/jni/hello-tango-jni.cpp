#include <stdio.h>
#include <stdlib.h>

#include <android/log.h>
#include <jni.h>

#include <tango-api/application-interface.h>
#include <tango-api/vio-interface.h>

#ifdef __cplusplus
extern "C"
{
#endif
    application_handle_t *app_handler;

    #define LOG_TAG    "JJ"
    #define LOG(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
    
    JNIEXPORT void JNICALL Java_com_google_tango_hellotangojni_TangoJNINative_initApplication(
		JNIEnv *env, jobject obj)
    {
        app_handler = ApplicationInitialize("[Superframes Small-Peanut]", 1);
        if(app_handler == NULL)
        {
            LOG("Application initialize failed\n");
            return;
        }
        CAPIErrorCodes ret_error;
        if((ret_error = VIOInitialize(app_handler, 0, NULL)) != kCAPISuccess)
        {
            LOG("VIO initialized failed: %d\n", ret_error);
            return;
        }
        LOG("Application and VIO initialized success");
    }

    JNIEXPORT void JNICALL Java_com_google_tango_hellotangojni_TangoJNINative_updateVIO(
		JNIEnv * env, jobject obj)
    {
        VIOStatus viostatus;
        CAPIErrorCodes ret_error;
        if((ret_error = ApplicationDoStep(app_handler)) != kCAPISuccess)
        {
            LOG("Application do step failed: %d\n", ret_error);
            return;
        }

        if((ret_error = VIOGetLatestPoseOpenGL(app_handler, &viostatus))
           != kCAPISuccess)
        {
            LOG("Application do step failed: %d\n", ret_error);
            return;
        }
        LOG("x = %f, y = %f, z = %f\n", viostatus.translation[0],
            viostatus.translation[1], viostatus.translation[2]);
    }
#ifdef __cplusplus
}
#endif
