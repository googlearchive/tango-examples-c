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

package com.projecttango.experiments.nativemotiontracking;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Point;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

/**
 * The main activity of the application which shows debug information and a
 * glSurfaceView that renders graphic content.
 */
public class MotionTrackingActivity extends Activity {
  // The minimum Tango Core version required from this application.
  private static final int MIN_TANGO_CORE_VERSION = 6804;

  // The package name of Tang Core, used for checking minimum Tango Core version.
  private static final String TANGO_PACKAGE_NAME = "com.projecttango.tango";

  // Tag for debug logging.
  private static final String TAG = MotionTrackingActivity.class.getSimpleName();

  // GLSurfaceView and its renderer, all of the graphic content is rendered
  // through OpenGL ES 2.0 in the native code.
  private MotionTrackingRenderer mRenderer;
  private GLSurfaceView mGLView;
  private boolean mIsConnectedService = false;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    setContentView(R.layout.activity_motion_tracking);
    
    // OpenGL view where all of the graphics are drawn
    mGLView = (GLSurfaceView) findViewById(R.id.gl_surface_view);
    
    // Configure OpenGL renderer
    mGLView.setEGLContextClientVersion(2);

    // Configure OpenGL renderer
    mRenderer = new MotionTrackingRenderer();
    mGLView.setRenderer(mRenderer);

    // Check that the installed version of the Tango Core is up to date.
    if (!TangoJNINative.checkTangoVersion(this, MIN_TANGO_CORE_VERSION)) {
      Toast.makeText(this, "Tango Core is out of date, please update in Play Store", 
                     Toast.LENGTH_LONG).show();
      finish();
      return;
    }

    // Check the current screen rotation and set it to the renderer.
    WindowManager mWindowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
    Display mDisplay = mWindowManager.getDefaultDisplay();

    TangoJNINative.setScreenRotation(mDisplay.getOrientation());
  }

  @Override
  protected void onResume() {
    super.onResume();
    mGLView.onResume();

    // Setup the configuration for the TangoService.
    TangoJNINative.setupConfig();
    
    // Connect the onPoseAvailable callback.
    TangoJNINative.connectCallbacks();

    // Connect to Tango Service (returns true on success).
    // Starts Motion Tracking and Area Learning.
    if (TangoJNINative.connect()) {
      mIsConnectedService = true;
    } else {
      // End the activity and let the user know something went wrong.
      Toast.makeText(this, "Connect Tango Service Error", Toast.LENGTH_LONG).show();
      finish();
    }
  }

  @Override
  protected void onPause() {
    super.onPause();
    mGLView.onPause();
    TangoJNINative.deleteResources();

    // Disconnect from Tango Service, release all the resources that the app is
    // holding from Tango Service.
    if (mIsConnectedService) {
      TangoJNINative.disconnect();
      mIsConnectedService = false;
    }
  }
}
