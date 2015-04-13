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

#include <jni.h>

#include "hello-tango-jni/tango_handler.h"

static hello_tango_jni::TangoHandler tango_handler;

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativehellotango_TangoJNINative_initialize(
    JNIEnv* env, jobject, jobject activity) {
  return static_cast<int>(tango_handler.Initialize(env, activity));
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativehellotango_TangoJNINative_setupConfig(
    JNIEnv*, jobject) {
  return static_cast<int>(tango_handler.SetupConfig());
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativehellotango_TangoJNINative_connectCallbacks(
    JNIEnv*, jobject) {
  return static_cast<int>(tango_handler.ConnectPoseCallback());
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativehellotango_TangoJNINative_connect(
    JNIEnv*, jobject) {
  return static_cast<int>(tango_handler.ConnectService());
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativehellotango_TangoJNINative_disconnect(
    JNIEnv*, jobject) {
  tango_handler.DisconnectService();
}
#ifdef __cplusplus
}
#endif
