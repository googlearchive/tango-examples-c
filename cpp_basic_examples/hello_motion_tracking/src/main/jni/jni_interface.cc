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

#include "hello_motion_tracking/tango_handler.h"

static hello_motion_tracking::TangoHandler tango_handler;

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_hellomotiontracking_TangoJniNative_onCreate(
    JNIEnv* env, jobject /*obj*/, jobject caller_activity) {
  tango_handler.OnCreate(env, caller_activity);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_hellomotiontracking_TangoJniNative_onTangoServiceConnected(
    JNIEnv* env, jobject, jobject iBinder) {
  tango_handler.OnTangoServiceConnected(env, iBinder);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_hellomotiontracking_TangoJniNative_onPause(
    JNIEnv*, jobject) {
  tango_handler.OnPause();
}

#ifdef __cplusplus
}
#endif
