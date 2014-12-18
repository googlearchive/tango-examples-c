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

package com.projecttango.experiments.nativearealearning;

/**
 * Interfaces between C and Java.
 */
public class TangoJNINative {
  static {
    System.loadLibrary("area_description_jni_example");
  }

  public static native void initGlContent();

  public static native int initialize(Object activity);

  public static native void setupConfig(boolean isLearning, boolean isLoadedADF);

  public static native boolean connectCallbacks();

  public static native int connect();

  public static native void disconnect();

  public static native void freeGLContent();

  public static native void setupGraphic(int width, int height);

  public static native void render();

  public static native void setCamera(int cameraIndex);

  public static native double getCurrentTimestamp(int index);

  public static native boolean saveADF();

  public static native String getUUID();

  public static native String getAllUUIDs();

  public static native int getADFCount();

  public static native String getUUIDMetadataValue(String uuid, String key);

  public static native void setUUIDMetadataValue(String uuid, String key, int size, String value);

  public static native void deleteADF(String uuid);

  public static native String getPoseString(int index);
  
  public static native String getVersionString();
  
  public static native String getEventString();
  
  public static native float startSetCameraOffset();
  
  public static native float setCameraOffset(float rotX, float rotY, float zDistance);
}
