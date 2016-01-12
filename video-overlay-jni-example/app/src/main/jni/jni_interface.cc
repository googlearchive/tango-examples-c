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
#include <tango-video-overlay/video_overlay_app.h>

static tango_video_overlay::VideoOverlayApp app;

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_initialize(
    JNIEnv* env, jobject, jobject activity) {
  return app.TangoInitialize(env, activity);
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_setupConfig(
    JNIEnv*, jobject) {
  return app.TangoSetupConfig();
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_connect(
    JNIEnv*, jobject) {
  return app.TangoConnect();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_disconnect(
    JNIEnv*, jobject) {
  app.TangoDisconnect();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_initGlContent(
    JNIEnv*, jobject) {
  app.InitializeGLContent();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_setupGraphic(
    JNIEnv*, jobject, jint width, jint height) {
  app.SetViewPort(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_render(
    JNIEnv*, jobject) {
  app.Render();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_freeBufferData(
    JNIEnv*, jobject) {
  app.FreeBufferData();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_setYUVMethod(
    JNIEnv*, jobject) {
  app.SetTextureMethod(0);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativevideooverlay_TangoJNINative_setTextureMethod(
    JNIEnv*, jobject) {
  app.SetTextureMethod(1);
}

#ifdef __cplusplus
}
#endif
