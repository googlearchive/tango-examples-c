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

package com.projecttango.examples.cpp.pointtopoint;

import android.app.Activity;
import android.os.IBinder;
import android.util.Log;

import com.projecttango.examples.cpp.util.TangoInitializationHelper;

/**
 * Interfaces between native C++ code and Java code.
 */
public class JNIInterface {
  static {
    // This project depends on tango_client_api, so we need to make sure we load
    // the correct library first.
    if (TangoInitializationHelper.loadTangoSharedLibrary() ==
        TangoInitializationHelper.ARCH_ERROR) {
      Log.e("TangoJNINative", "ERROR! Unable to load libtango_client_api.so!");
    }
    System.loadLibrary("cpp_point_to_point_example");
  }

  // Check that the installed version of the Tango API is up to date.
  //
  // @return returns true if the application version is compatible with the
  //         Tango Core version.
  public static native boolean checkTangoVersion(MainActivity activity,
                                                 int minTangoVersion);

  // Called when Tango Service is connected successfully.
  public static native void onTangoServiceConnected(IBinder binder);

  // Set up the configuration, callbacks, and connect to the Tango Service.
  public static native int tangoSetupAndConnect();

  // Disconnect from the Tango Service, release all the resources that
  // the app is holding from the Tango Service.
  public static native void tangoDisconnect();

  // Allocate OpenGL resources for rendering and register the color
  // camera texture.
  public static native int initializeGLContent();

  // Use bilateral filtering to upsample point cloud.
  public static native void setUpsampleViaBilateralFiltering(boolean bilateral);

  // Get the distance between the two selected points.
  public static native String getPointSeparation();

  // Setup the view port width and height.
  public static native void setViewPort(int width, int height);

  // Main render loop.
  public static native void render();

  // Release resources that are allocated.
  public static native void deleteResources();

  // Respond to a touch event.
  public static native void onTouchEvent(float x, float y);
}
