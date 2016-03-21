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
#include <tango-motion-tracking/motion_tracking_app.h>
#include <tango-motion-tracking/scene.h>

static tango_motion_tracking::MotiongTrackingApp app;

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jboolean JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_checkTangoVersion(
    JNIEnv* env, jobject, jobject activity, jint min_tango_version) {
  return app.CheckTangoVersion(env, activity, min_tango_version);
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_setupConfig(
    JNIEnv*, jobject) {
  return app.TangoSetupConfig();
}

JNIEXPORT jboolean JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_connect(
    JNIEnv*, jobject) {
  return app.TangoConnect();
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_connectCallbacks(
    JNIEnv*, jobject) {
  int ret = app.TangoConnectCallbacks();
  return ret;
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_disconnect(
    JNIEnv*, jobject) {
  app.TangoDisconnect();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_initGlContent(
    JNIEnv*, jobject) {
  app.InitializeGLContent();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_setupGraphic(
    JNIEnv*, jobject, jint width, jint height) {
  app.SetViewPort(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_render(
    JNIEnv*, jobject) {
  app.Render();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_deleteResources(
    JNIEnv*, jobject) {
  app.DeleteResources();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_setScreenRotation(
    JNIEnv*, jobject, int rotation_index) {
  app.SetScreenRotation(rotation_index);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_initializeTango(
    JNIEnv* env, jobject, jobject iBinder) {
  app.InitializeTango(env, iBinder);
}
#ifdef __cplusplus
}
#endif
