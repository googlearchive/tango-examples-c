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

package com.projecttango.experiments.nativepointcloud;

/**
 * Interfaces between C and Java.
 */
public class TangoJNINative {
  static {
    System.loadLibrary("point_cloud_jni_example");
  }

  public static native void initGlContent();

  public static native int initialize(PointcloudActivity activity);

  public static native void setupConfig();

  public static native void setupExtrinsics();

  public static native int connect();

  public static native void connectCallbacks();

  public static native void disconnect();

  public static native void freeGLContent();

  public static native void setupGraphic(int width, int height);

  public static native void render();

  public static native void setCamera(int cameraIndex);

  public static native String getVersionNumber();
  
  public static native String getEventString();
  
  public static native String getPoseString();

  public static native int getVerticesCount();
  
  public static native float getAverageZ();
  
  public static native float getFrameDeltaTime();
  
  public static native float startSetCameraOffset();
  
  public static native float setCameraOffset(float rotX, float rotY, float zDistance);
}
