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

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_planefitting_TangoJniNative_onCreate(
    JNIEnv* env, jobject, jobject activity) {
  app.OnCreate(env, activity);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_planefitting_TangoJniNative_onPause(
    JNIEnv*, jobject) {
  app.OnPause();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_planefitting_TangoJniNative_onTangoServiceConnected(
    JNIEnv* env, jobject /*obj*/, jobject binder) {
  app.OnTangoServiceConnected(env, binder);
}

JNIEXPORT jboolean JNICALL
Java_com_projecttango_examples_cpp_planefitting_TangoJniNative_onGlSurfaceCreated(
    JNIEnv* /*env*/, jobject /*obj*/) {
  return app.InitializeGLContent();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_planefitting_TangoJniNative_setRenderDebugPointCloud(
    JNIEnv* /*env*/, jobject /*obj*/, jboolean on) {
  app.SetRenderDebugPointCloud(on);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_planefitting_TangoJniNative_onGlSurfaceChanged(
    JNIEnv* /*env*/, jobject /*obj*/, jint width, jint height) {
  app.SetViewPort(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_planefitting_TangoJniNative_onGlSurfaceDrawFrame(
    JNIEnv* /*env*/, jobject /*obj*/) {
  app.Render();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_planefitting_TangoJniNative_onTouchEvent(
    JNIEnv* /*env*/, jobject /*obj*/, jfloat x, jfloat y) {
  app.OnTouchEvent(x, y);
}

#ifdef __cplusplus
}
#endif
