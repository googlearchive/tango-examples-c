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

package com.projecttango.examples.cpp.pointcloud;

import android.app.Activity;
import android.os.IBinder;
import android.util.Log;

import com.projecttango.examples.cpp.util.TangoInitializationHelper;

/**
 * Interfaces between C and Java.
 */
public class TangoJNINative {
  static {
    // This project depends on tango_client_api, so we need to make sure we load
    // the correct library first.
    if (TangoInitializationHelper.loadTangoSharedLibrary() ==
        TangoInitializationHelper.ARCH_ERROR) {
      Log.e("TangoJNINative", "ERROR! Unable to load libtango_client_api.so!");
    }
    System.loadLibrary("cpp_point_cloud_example");
  }

  /**
   * Interfaces to native OnCreate function.
   *
   * @param callerActivity the caller activity of this function.
   */
  public static native void onCreate(Activity callerActivity);

  /**
   * Called when the Tango service is connected successfully.
   *
   * @param nativeTangoServiceBinder The native binder object.
   */
  public static native void onTangoServiceConnected(IBinder nativeTangoServiceBinder);

  /**
   * Interfaces to native OnPause function.
   */
  public static native void onPause();

  // Allocate OpenGL resources for rendering.
  public static native void onGlSurfaceCreated();

  // Setup the view port width and height.
  public static native void onGlSurfaceChanged(int width, int height);

  // Main render loop.
  public static native void onGlSurfaceDrawFrame();

  // Set the render camera's viewing angle:
  //   first person, third person, or top down.
  public static native void setCamera(int cameraIndex);

  // Get total point count in current depth frame.
  public static native int getVerticesCount();

  // Get average depth (in meters) in current depth frame.
  public static native float getAverageZ();

  // Get depth frame delta time between current frame and previous frame.
  public static native float getFrameDeltaTime();

  // Pass touch events to the native layer.
  public static native void onTouchEvent(int touchCount, int event0,
                                         float x0, float y0, float x1, float y1);

  public static native void setScreenRotation(int orienationIndex);
}
