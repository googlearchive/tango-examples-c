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
#include <hello_area_description/hello_area_description_app.h>

static hello_area_description::AreaLearningApp app;

#ifdef __cplusplus
extern "C" {
#endif
jint JNI_OnLoad(JavaVM* vm, void*) {
  // We need to store a reference to the Java VM so that we can call into the
  // Java layer to show progress while saving ADFs
  app.SetJavaVM(vm);
  return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_helloareadescription_TangoJniNative_onCreate(
    JNIEnv* env, jobject /*obj*/, jobject caller_activity) {
  app.OnCreate(env, caller_activity);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_helloareadescription_TangoJniNative_onTangoServiceConnected(
    JNIEnv* env, jobject, jobject binder, jboolean is_area_learning_enabled,
    jboolean is_loading_area_description) {
  app.OnTangoServiceConnected(env, binder, is_area_learning_enabled,
                              is_loading_area_description);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_helloareadescription_TangoJniNative_onPause(
    JNIEnv*, jobject) {
  app.OnPause();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_helloareadescription_TangoJniNative_onDestroy(
    JNIEnv*, jobject) {
  app.OnDestroy();
}

JNIEXPORT jboolean JNICALL
Java_com_projecttango_examples_cpp_helloareadescription_TangoJniNative_isRelocalized(
    JNIEnv*, jobject) {
  return app.IsRelocalized();
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_examples_cpp_helloareadescription_TangoJniNative_getLoadedAdfUuidString(
    JNIEnv* env, jobject) {
  return (env)->NewStringUTF(app.GetLoadedAdfString().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_examples_cpp_helloareadescription_TangoJniNative_saveAdf(
    JNIEnv* env, jobject) {
  return (env)->NewStringUTF(app.SaveAdf().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_examples_cpp_helloareadescription_TangoJniNative_getAdfMetadataValue(
    JNIEnv* env, jobject, jstring uuid, jstring key) {
  std::string uuid_str(env->GetStringUTFChars(uuid, nullptr));
  std::string key_str(env->GetStringUTFChars(key, nullptr));
  return env->NewStringUTF(app.GetAdfMetadataValue(uuid_str, key_str).c_str());
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_helloareadescription_TangoJniNative_setAdfMetadataValue(
    JNIEnv* env, jobject, jstring uuid, jstring key, jstring value) {
  std::string uuid_str(env->GetStringUTFChars(uuid, nullptr));
  std::string key_str(env->GetStringUTFChars(key, nullptr));
  std::string value_str(env->GetStringUTFChars(value, nullptr));
  app.SetAdfMetadataValue(uuid_str, key_str, value_str);
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_examples_cpp_helloareadescription_TangoJniNative_getAllAdfUuids(
    JNIEnv* env, jobject) {
  return env->NewStringUTF(app.GetAllAdfUuids().c_str());
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_helloareadescription_TangoJniNative_deleteAdf(
    JNIEnv* env, jobject, jstring uuid) {
  std::string uuid_str(env->GetStringUTFChars(uuid, nullptr));
  return app.DeleteAdf(uuid_str);
}

#ifdef __cplusplus
}
#endif
