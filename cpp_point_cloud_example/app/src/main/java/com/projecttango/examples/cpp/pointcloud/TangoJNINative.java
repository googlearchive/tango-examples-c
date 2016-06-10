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

  // Check that the installed version of the Tango API is up to date.
  //
  // @return returns true if the application version is compatible with the
  //         Tango Core version.
  public static native boolean checkTangoVersion(PointcloudActivity activity,
                                                 int minTangoVersion);

  // Called when Tango Service is connected successfully.
  public static native boolean onTangoServiceConnected(IBinder binder);

  // Setup the configuration file of the Tango Service. We are also setting up
  // the auto-recovery option from here.
  public static native int setupConfig();

  // Connect the onPoseAvailable callback.
  public static native int connectCallbacks();

  // Connect to the Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start Motion Tracking and Depth sensing.
  public static native boolean connect();

  // Disconnect from the Tango Service, release all the resources that the app is
  // holding from the Tango Service.
  public static native void disconnect();

  // Release all non OpenGl resources that are allocated from the program.
  public static native void deleteResources();

  // Allocate OpenGL resources for rendering.
  public static native void initGlContent();

  // Setup the view port width and height.
  public static native void setupGraphic(int width, int height);

  // Main render loop.
  public static native void render();

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
