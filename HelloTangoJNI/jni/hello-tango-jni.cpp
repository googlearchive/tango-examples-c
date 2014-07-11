#include <stdio.h>
#include <stdlib.h>

#include <android/log.h>
#include <jni.h>

#include <tango-api/application-interface.h>
#include <tango-api/vio-interface.h>

#include <GLES2/gl2.h>
#include <GLES/gl.h>
#include <math.h>

#ifdef __cplusplus
extern "C"
{
#endif
    application_handle_t *app_handler;

    #define LOG_TAG    "JJ"
    #define LOG(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
    static float vioStatusArray[6] = {};
    
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
        if((ret_error = VIOInitialize(app_handler, 1, NULL)) != kCAPISuccess)
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

        vioStatusArray[0]=viostatus.translation[0];
        vioStatusArray[1]=viostatus.translation[1];
        vioStatusArray[2]=viostatus.translation[2];
        vioStatusArray[3]=viostatus.rotation[0];
        vioStatusArray[4]=viostatus.rotation[1];
        vioStatusArray[5]=viostatus.rotation[2];
    }

    JNIEXPORT void Java_com_google_tango_hellotangojni_TangoJNINative_onGlSurfaceCreated(JNIEnv * env, jclass cls) {
    	glClearColor(0, 0, 0, 1.0f);
    	glEnable(GL_CULL_FACE);
    	glEnable(GL_DEPTH_TEST);
    }

    JNIEXPORT void Java_com_google_tango_hellotangojni_TangoJNINative_onGLSurfaceChanged(JNIEnv * env, jclass cls, jint width, jint height) {
    	glViewport(0, 0, width, height);
    	glMatrixMode(GL_PROJECTION);
    	glLoadIdentity();
    	glFrustumf(-(GLfloat) width/height, (GLfloat) width/height, -1.0f, 1.0f, 1.0f, 10.0f);
    	glMatrixMode(GL_MODELVIEW);
    	glLoadIdentity();
    }

    JNIEXPORT void Java_com_google_tango_hellotangojni_TangoJNINative_onGLSurfaceDraw(JNIEnv * env, jclass cls) {
    	static GLfloat vertices[] =
		{
			0, 0, 1.0,
		    0, 0, 0,
		    0, 1.0, 0,
		    0, 1.0, 1.0,
		    1.0, 0, 1.0,
		    1.0, 0, 0,
		    1.0, 1.0, 0,
		    1.0, 1.0, 1.0
		};
    	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    	glMatrixMode(GL_MODELVIEW);
    	glLoadIdentity();
    	glTranslatef(vioStatusArray[0]*3.0, vioStatusArray[1]*3.0, -3.0f+vioStatusArray[2]*3.0);
    	glRotatef(vioStatusArray[3]*180.0, 1.0, 0.0, 0.0);
    	glRotatef(vioStatusArray[4]*180.0, 0.0, 0.0, -1.0);
    	glRotatef(vioStatusArray[5]*180.0, 0.0, 1.0, 0.0);

    	glTranslatef(-0.5, -0.5, -0.5);
    	glEnableClientState(GL_VERTEX_ARRAY);
    	glVertexPointer(3, GL_FLOAT, 0, vertices);

    	static GLbyte frontIndices[] = { 4, 5, 6, 7 };
    	static GLbyte rightIndices[] = { 1, 2, 6, 5 };
    	static GLbyte bottomIndices[] = { 0, 1, 5, 4 };
    	static GLbyte backIndices[] = { 0, 3, 2, 1 };
    	static GLbyte leftIndices[] = { 0, 4, 7, 3 };
    	static GLbyte topIndices[] = { 2, 3, 7, 6 };

    	glColor4f(0.3, 0.2, 0.8, 1.0);
    	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, frontIndices);
    	glColor4f(1.0, 0.3, 0.6, 1.0);
    	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, rightIndices);
    	glColor4f(0.3, 0.7, 0.9, 1.0);
    	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, bottomIndices);
    	glColor4f(0.9, 0.5, 0.2, 1.0);
    	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, backIndices);
    	glColor4f(0.1, 0.7, 0.6, 1.0);
    	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, leftIndices);
    	glColor4f(0.8, 0.8, 0.0, 1.0);
    	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, topIndices);
    	glDisableClientState(GL_VERTEX_ARRAY);

    	glFlush();
    }

#ifdef __cplusplus
}
#endif
