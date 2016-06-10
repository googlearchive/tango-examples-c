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
#include <tango-point-cloud/point_cloud_app.h>
#include <tango-point-cloud/scene.h>

static tango_point_cloud::PointCloudApp app;

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jboolean JNICALL
Java_com_projecttango_examples_cpp_pointcloud_TangoJNINative_checkTangoVersion(
    JNIEnv* env, jobject, jobject activity, jint min_tango_version) {
  return app.CheckTangoVersion(env, activity, min_tango_version);
}

JNIEXPORT jboolean JNICALL
Java_com_projecttango_examples_cpp_pointcloud_TangoJNINative_onTangoServiceConnected(
    JNIEnv* env, jobject /*caller_object*/, jobject binder) {
  return app.OnTangoServiceConnected(env, binder);
}

JNIEXPORT jint JNICALL
Java_com_projecttango_examples_cpp_pointcloud_TangoJNINative_setupConfig(
    JNIEnv*, jobject) {
  return app.TangoSetupConfig();
}

JNIEXPORT jboolean JNICALL
Java_com_projecttango_examples_cpp_pointcloud_TangoJNINative_connect(JNIEnv*,
                                                                     jobject) {
  return app.TangoConnect();
}

JNIEXPORT jint JNICALL
Java_com_projecttango_examples_cpp_pointcloud_TangoJNINative_connectCallbacks(
    JNIEnv*, jobject) {
  return app.TangoConnectCallbacks();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointcloud_TangoJNINative_disconnect(
    JNIEnv*, jobject) {
  app.TangoDisconnect();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointcloud_TangoJNINative_initGlContent(
    JNIEnv*, jobject) {
  app.InitializeGLContent();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointcloud_TangoJNINative_setupGraphic(
    JNIEnv*, jobject, jint width, jint height) {
  app.SetViewPort(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointcloud_TangoJNINative_render(JNIEnv*,
                                                                    jobject) {
  app.Render();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointcloud_TangoJNINative_deleteResources(
    JNIEnv*, jobject) {
  app.DeleteResources();
}

JNIEXPORT jint JNICALL
Java_com_projecttango_examples_cpp_pointcloud_TangoJNINative_getVerticesCount(
    JNIEnv*, jobject) {
  return app.GetPointCloudVerticesCount();
}

JNIEXPORT jfloat JNICALL
Java_com_projecttango_examples_cpp_pointcloud_TangoJNINative_getAverageZ(
    JNIEnv*, jobject) {
  return app.GetAverageZ();
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointcloud_TangoJNINative_setCamera(
    JNIEnv*, jobject, int camera_index) {
  tango_gl::GestureCamera::CameraType cam_type =
      static_cast<tango_gl::GestureCamera::CameraType>(camera_index);
  app.SetCameraType(cam_type);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointcloud_TangoJNINative_onTouchEvent(
    JNIEnv*, jobject, int touch_count, int event, float x0, float y0, float x1,
    float y1) {
  tango_gl::GestureCamera::TouchEvent touch_event =
      static_cast<tango_gl::GestureCamera::TouchEvent>(event);
  app.OnTouchEvent(touch_count, touch_event, x0, y0, x1, y1);
}

JNIEXPORT void JNICALL
Java_com_projecttango_examples_cpp_pointcloud_TangoJNINative_setScreenRotation(
    JNIEnv*, jobject, int rotation_index) {
  app.SetScreenRotation(rotation_index);
}
#ifdef __cplusplus
}
#endif
