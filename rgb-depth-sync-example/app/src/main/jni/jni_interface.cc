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

#define GLM_FORCE_RADIANS

#include <jni.h>

#include "rgb-depth-sync/rgb_depth_sync_application.h"

static rgb_depth_sync::SynchronizationApplication app;

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_rgbdepthsync_JNIInterface_tangoInitialize(
    JNIEnv* env, jobject, jobject activity) {
  return app.TangoInitialize(env, activity);
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_rgbdepthsync_JNIInterface_tangoSetupConfig(
    JNIEnv*, jobject) {
  return app.TangoSetupConfig();
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_rgbdepthsync_JNIInterface_tangoConnectTexture(
    JNIEnv*, jobject) {
  return app.TangoConnectTexture();
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_rgbdepthsync_JNIInterface_tangoConnectCallbacks(
    JNIEnv*, jobject) {
  return app.TangoConnectCallbacks();
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_rgbdepthsync_JNIInterface_tangoConnect(
    JNIEnv*, jobject) {
  return app.TangoConnect();
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_rgbdepthsync_JNIInterface_tangoSetIntrinsicsAndExtrinsics(
    JNIEnv*, jobject) {
  return app.TangoSetIntrinsicsAndExtrinsics();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_rgbdepthsync_JNIInterface_tangoDisconnect(
    JNIEnv*, jobject) {
  app.TangoDisconnect();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_rgbdepthsync_JNIInterface_initializeGLContent(
    JNIEnv*, jobject) {
  app.InitializeGLContent();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_rgbdepthsync_JNIInterface_setViewPort(
    JNIEnv*, jobject, jint width, jint height) {
  app.SetViewPort(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_rgbdepthsync_JNIInterface_render(JNIEnv*,
                                                                   jobject) {
  app.Render();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_rgbdepthsync_JNIInterface_setDepthAlphaValue(
    JNIEnv*, jobject, jfloat alpha) {
  return app.SetDepthAlphaValue(alpha);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_rgbdepthsync_JNIInterface_setGPUUpsample(
    JNIEnv*, jobject, jboolean on) {
  return app.SetGPUUpsample(on);
}

#ifdef __cplusplus
}
#endif
