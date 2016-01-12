/*
 * Copyright 2015 Google Inc. All Rights Reserved.
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

#include <jni.h>

#include "tango-plane-fitting/plane_fitting_application.h"

static tango_plane_fitting::PlaneFittingApplication app;

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativeplanefitting_JNIInterface_tangoInitialize(
    JNIEnv* env, jobject /*obj*/, jobject activity) {
  return app.TangoInitialize(env, activity);
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativeplanefitting_JNIInterface_tangoSetupAndConnect(
    JNIEnv* /*env*/, jobject /*obj*/) {
  return app.TangoSetupAndConnect();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeplanefitting_JNIInterface_tangoDisconnect(
    JNIEnv* /*env*/, jobject /*obj*/) {
  app.TangoDisconnect();
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativeplanefitting_JNIInterface_initializeGLContent(
    JNIEnv* /*env*/, jobject /*obj*/) {
  return app.InitializeGLContent();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeplanefitting_JNIInterface_setRenderDebugPointCloud(
    JNIEnv* /*env*/, jobject /*obj*/, jboolean on) {
  app.SetRenderDebugPointCloud(on);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeplanefitting_JNIInterface_setViewPort(
    JNIEnv* /*env*/, jobject /*obj*/, jint width, jint height) {
  app.SetViewPort(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeplanefitting_JNIInterface_render(
    JNIEnv* /*env*/, jobject /*obj*/) {
  app.Render();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeplanefitting_JNIInterface_deleteResources(
    JNIEnv* /*env*/, jobject /*obj*/) {
  app.DeleteResources();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativeplanefitting_JNIInterface_onTouchEvent(
    JNIEnv* /*env*/, jobject /*obj*/, jfloat x, jfloat y) {
  app.OnTouchEvent(x, y);
}

#ifdef __cplusplus
}
#endif
