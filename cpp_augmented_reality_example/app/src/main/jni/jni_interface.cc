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

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <jni.h>
#include <tango-augmented-reality/augmented_reality_app.h>

static tango_augmented_reality::AugmentedRealityApp app;

#ifdef __cplusplus
extern "C" {
#endif
jint JNI_OnLoad(JavaVM* vm, void*) {
  // We need to store a reference to the Java VM so that we can call into the
  // Java layer to trigger rendering.
  app.SetJavaVM(vm);
  return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_augmentedreality_TangoJNINative_onCreate(
    JNIEnv* env, jobject, jobject activity) {
  app.OnCreate(env, activity);
}

JNIEXPORT jboolean JNICALL
Java_com_projecttango_examples_cpp_augmentedreality_TangoJNINative_onTangoServiceConnected(
    JNIEnv* env, jobject, jobject iBinder) {
  return app.OnTangoServiceConnected(env, iBinder);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_augmentedreality_TangoJNINative_onPause(
    JNIEnv*, jobject) {
  app.OnPause();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_augmentedreality_TangoJNINative_destroyActivity(
    JNIEnv*, jobject) {
  app.ActivityDestroyed();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_augmentedreality_TangoJNINative_onGlSurfaceCreated(
    JNIEnv* env, jobject, jobject j_asset_manager) {
  AAssetManager* aasset_manager = AAssetManager_fromJava(env, j_asset_manager);
  app.InitializeGLContent(aasset_manager);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_augmentedreality_TangoJNINative_setupGraphic(
    JNIEnv*, jobject, jint width, jint height) {
  app.SetViewPort(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_augmentedreality_TangoJNINative_render(
    JNIEnv*, jobject) {
  app.Render();
}

#ifdef __cplusplus
}
#endif
