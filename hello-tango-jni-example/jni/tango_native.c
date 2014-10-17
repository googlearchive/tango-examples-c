/*
 * Copyright 2014 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android/log.h>
#include <jni.h>

#include <tango_client_api.h>

#define LOG_TAG "hello-tango-jni"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

// Tango configuration file.
// Configuration file describs current states of
// Tango Service.
TangoConfig config;

static void onPoseAvailable(void* context, const TangoPoseData* pose) {
  LOGI("Position: %f, %f, %f. Orientation: %f, %f, %f, %f",
       pose->translation[0], pose->translation[1], pose->translation[2],
       pose->orientation[0], pose->orientation[2], pose->orientation[3],
       pose->orientation[3]);
}

bool TangoInitialize(JNIEnv* env, jobject activity) {
  // Initialize Tango Service.
  // TODO(jguo): pass in env and jobject from activity.
  if (TangoService_initialize(env, activity) != TANGO_SUCCESS) {
    LOGE("TangoService_initialize(): Failed");
    return false;
  }
  return true;
}

bool TangoSetConfig() {
  // Get the default TangoConfig.
  config = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (config == NULL) {
    LOGE("TangoService_getConfig(): Failed");
    return false;
  }
  return true;
}

bool TangoConnect() {
  // Set listening pairs. Connenct pose callback.
  // Note: the callback function should be re-connected 
  // after the application resumed from background.
  TangoCoordinateFramePair pair = {TANGO_COORDINATE_FRAME_START_OF_SERVICE, TANGO_COORDINATE_FRAME_DEVICE };
  if (TangoService_connectOnPoseAvailable(1, &pair, onPoseAvailable)
      != TANGO_SUCCESS) {
    LOGI("TangoService_connectOnPoseAvailable(): Failed");
    return false;
  }

  // Connect to the Tango Service.
  // Note: connecting Tango service will start the motion
  // tracking automatically.
  if (TangoService_connect(NULL, config) != TANGO_SUCCESS) {
    LOGE("TangoService_connect(): Failed");
    return false;
  }
  return true;
}

void DisconnectTango()
{
  // Disconnect Tango Service.
  TangoService_disconnect();
}

JNIEXPORT void JNICALL Java_com_projecttango_hellotangonative_TangoJNINative_onCreate(JNIEnv* env, jobject obj, jobject activity)
{
  TangoInitialize(env, activity);
  TangoSetConfig();
}

JNIEXPORT void JNICALL Java_com_projecttango_hellotangonative_TangoJNINative_onResume(JNIEnv* env, jobject obj)
{
  TangoConnect();
}

JNIEXPORT void JNICALL Java_com_projecttango_hellotangonative_TangoJNINative_onPause(JNIEnv* env, jobject obj)
{
  DisconnectTango();
}
