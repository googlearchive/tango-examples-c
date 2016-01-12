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
#include <tango-area-learning/area_learning_app.h>
#include <tango-area-learning/scene.h>

static tango_area_learning::AreaLearningApp app;

#ifdef __cplusplus
extern "C" {
#endif
jint JNI_OnLoad(JavaVM* vm, void*) {
  // We need to store a reference to the Java VM so that we can call into the
  // Java layer to trigger rendering.
  app.SetJavaVM(vm);
  return JNI_VERSION_1_6;
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_initialize(
    JNIEnv* env, jobject, jobject activity) {
  return app.TangoInitialize(env, activity);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_destroyActivity(
    JNIEnv*, jobject) {
  app.ActivityDestroyed();
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_setupConfig(
    JNIEnv*, jobject, bool is_area_learningEnabled, bool is_loading_adf) {
  return app.TangoSetupConfig(is_area_learningEnabled, is_loading_adf);
}

JNIEXPORT jboolean JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_connect(
    JNIEnv*, jobject) {
  return app.TangoConnect();
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_connectCallbacks(
    JNIEnv*, jobject) {
  int ret = app.TangoConnectCallbacks();
  return ret;
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_disconnect(
    JNIEnv*, jobject) {
  app.TangoDisconnect();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_resetMotionTracking(
    JNIEnv*, jobject) {
  app.TangoResetMotionTracking();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_initGlContent(
    JNIEnv*, jobject) {
  app.InitializeGLContent();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_setupGraphics(
    JNIEnv*, jobject, jint width, jint height) {
  app.SetViewPort(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_render(
    JNIEnv*, jobject) {
  app.Render();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_deleteResources(
    JNIEnv*, jobject) {
  app.DeleteResources();
}

JNIEXPORT jboolean JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_isRelocalized(
    JNIEnv*, jobject) {
  return app.IsRelocalized();
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_getStartServiceTDeviceString(
    JNIEnv* env, jobject) {
  return (env)->NewStringUTF(app.GetStartServiceTDeviceString().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_getAdfTDeviceString(
    JNIEnv* env, jobject) {
  return (env)->NewStringUTF(app.GetAdfTDeviceString().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_getAdfTStartServiceString(
    JNIEnv* env, jobject) {
  return (env)->NewStringUTF(app.GetAdfTStartServiceString().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_getEventString(
    JNIEnv* env, jobject) {
  return (env)->NewStringUTF(app.GetEventString().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_getVersionNumber(
    JNIEnv* env, jobject) {
  return (env)->NewStringUTF(app.GetVersionString().c_str());
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_setCamera(
    JNIEnv*, jobject, int camera_index) {
  using namespace tango_gl;
  GestureCamera::CameraType cam_type =
      static_cast<GestureCamera::CameraType>(camera_index);
  app.SetCameraType(cam_type);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_onTouchEvent(
    JNIEnv*, jobject, int touch_count, int event, float x0, float y0, float x1,
    float y1) {
  using namespace tango_gl;
  GestureCamera::TouchEvent touch_event =
      static_cast<GestureCamera::TouchEvent>(event);
  app.OnTouchEvent(touch_count, touch_event, x0, y0, x1, y1);
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_getLoadedADFUUIDString(
    JNIEnv* env, jobject) {
  return (env)->NewStringUTF(app.GetLoadedAdfString().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_saveAdf(
    JNIEnv* env, jobject) {
  return (env)->NewStringUTF(app.SaveAdf().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_getAdfMetadataValue(
    JNIEnv* env, jobject, jstring uuid, jstring key) {
  std::string uuid_str(env->GetStringUTFChars(uuid, nullptr));
  std::string key_str(env->GetStringUTFChars(key, nullptr));
  return env->NewStringUTF(app.GetAdfMetadataValue(uuid_str, key_str).c_str());
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_setAdfMetadataValue(
    JNIEnv* env, jobject, jstring uuid, jstring key, jstring value) {
  std::string uuid_str(env->GetStringUTFChars(uuid, nullptr));
  std::string key_str(env->GetStringUTFChars(key, nullptr));
  std::string value_str(env->GetStringUTFChars(value, nullptr));
  app.SetAdfMetadataValue(uuid_str, key_str, value_str);
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_getAllAdfUuids(
    JNIEnv* env, jobject) {
  return env->NewStringUTF(app.GetAllAdfUuids().c_str());
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_deleteAdf(
    JNIEnv* env, jobject, jstring uuid) {
  std::string uuid_str(env->GetStringUTFChars(uuid, nullptr));
  return app.DeleteAdf(uuid_str);
}

#ifdef __cplusplus
}
#endif
