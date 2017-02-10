/*
 * Copyright 2016 Google Inc. All Rights Reserved.
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

#include "tango-point-to-point/point_to_point_application.h"

static tango_point_to_point::PointToPointApplication app;

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointtopoint_TangoJNINative_onCreate(
    JNIEnv* env, jobject, jobject activity) {
  app.OnCreate(env, activity);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointtopoint_TangoJNINative_onPause(
    JNIEnv*, jobject) {
  app.OnPause();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointtopoint_TangoJNINative_onTangoServiceConnected(
    JNIEnv* env, jobject /*obj*/, jobject binder) {
  app.OnTangoServiceConnected(env, binder);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointtopoint_TangoJNINative_onGlSurfaceCreated(
    JNIEnv* /*env*/, jobject /*obj*/) {
  app.OnSurfaceCreated();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointtopoint_TangoJNINative_setUpsampleViaBilateralFiltering(
    JNIEnv* /*env*/, jobject /*obj*/, jboolean on) {
  app.SetUpsampleViaBilateralFiltering(on);
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_examples_cpp_pointtopoint_TangoJNINative_getPointSeparation(
    JNIEnv* env, jobject) {
  return (env)->NewStringUTF(app.GetPointSeparation().c_str());
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointtopoint_TangoJNINative_onGlSurfaceChanged(
    JNIEnv* /*env*/, jobject /*obj*/, jint width, jint height) {
  app.OnSurfaceChanged(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointtopoint_TangoJNINative_onGlSurfaceDrawFrame(
    JNIEnv* /*env*/, jobject /*obj*/) {
  app.OnDrawFrame();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointtopoint_TangoJNINative_onTouchEvent(
    JNIEnv* /*env*/, jobject /*obj*/, jfloat x, jfloat y) {
  app.OnTouchEvent(x, y);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointtopoint_TangoJNINative_onDisplayChanged(
    JNIEnv* /*env*/, jobject /*obj*/, jint display_rotation) {
  app.OnDisplayChanged(display_rotation);
}

#ifdef __cplusplus
}
#endif
