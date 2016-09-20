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

#define GLM_FORCE_RADIANS

#include <jni.h>
#include "mesh_builder/mesh_builder_app.h"
#include "mesh_builder/scene.h"

static mesh_builder::MeshBuilderApp app;

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_meshbuilder_TangoJNINative_activityCtor(
    jboolean t3dr_running) {
  app.ActivityCtor(t3dr_running);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_meshbuilder_TangoJNINative_onCreate(
    JNIEnv* env, jobject, jobject activity) {
  app.OnCreate(env, activity);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_meshbuilder_TangoJNINative_onTangoServiceConnected(
    JNIEnv* env, jobject, jobject iBinder) {
  app.OnTangoServiceConnected(env, iBinder);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_meshbuilder_TangoJNINative_onPause(JNIEnv*,
                                                                      jobject) {
  app.OnPause();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_meshbuilder_TangoJNINative_onGlSurfaceCreated(
    JNIEnv*, jobject) {
  app.OnSurfaceCreated();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_meshbuilder_TangoJNINative_onGlSurfaceChanged(
    JNIEnv*, jobject, jint width, jint height) {
  app.OnSurfaceChanged(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_meshbuilder_TangoJNINative_onGlSurfaceDrawFrame(
    JNIEnv*, jobject) {
  app.OnDrawFrame();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_meshbuilder_TangoJNINative_onToggleButtonClicked(
    JNIEnv*, jobject, jboolean t3dr_is_running) {
  app.OnToggleButtonClicked(t3dr_is_running);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_meshbuilder_TangoJNINative_onClearButtonClicked(
    JNIEnv*, jobject) {
  app.OnClearButtonClicked();
}
#ifdef __cplusplus
}
#endif
