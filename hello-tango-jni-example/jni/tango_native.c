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
TangoConfig* config;

static void onPoseAvailable(TangoPoseData* pose) {
  LOGI("Position: %f, %f, %f. Orientation: %f, %f, %f, %f",
       pose->translation[0], pose->translation[1], pose->translation[2],
       pose->orientation[0], pose->orientation[2], pose->orientation[3],
       pose->orientation[3]);
}

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
  
  // Set the onPoseAvailable callback fucntion.
  if (TangoService_connectOnPoseAvailable(TANGO_COORDINATE_FRAME_DEVICE,
                                          TANGO_COORDINATE_FRAME_START_OF_SERVICE,
                                          onPoseAvailable) != 0) {
    LOGI("TangoService_connectOnPoseAvailable(): Failed");
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
