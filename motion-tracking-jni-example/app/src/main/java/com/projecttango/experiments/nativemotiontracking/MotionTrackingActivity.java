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
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

// The main activity of the application which shows debug information and a
// glSurfaceView that renders graphic content.
public class MotionTrackingActivity extends Activity implements
    View.OnClickListener {
  // The input argument is invalid.
  private static final int  TANGO_INVALID = -2;
  // This error code denotes some sort of hard error occurred.
  private static final int  TANGO_ERROR = -1;
  // This code indicates success.
  private static final int  TANGO_SUCCESS = 0;

  // The minimum Tango Core version required from this application.
  private static final int  MIN_TANGO_CORE_VERSION = 6804;

  // The package name of Tang Core, used for checking minimum Tango Core version.
  private static final String TANGO_PACKAGE_NAME = "com.projecttango.tango";

  // Tag for debug logging.
  private static final String TAG = MotionTrackingActivity.class.getSimpleName();

  // The interval at which we'll update our UI debug text in milliseconds.
  // This is the rate at which we query our native wrapper around the tango
  // service for pose and event information.
  private static final int UPDATE_UI_INTERVAL_MS = 100;

  // Debug information text.
  // Current frame's pose information.
  private TextView mPoseData;
  // Tango Core version.
  private TextView mVersion;
  // Application version.
  private TextView mAppVersion;
  // Latest Tango Event received.
  private TextView mEvent;

  // Button for manually resetting motion tracking. Resetting motion tracking
  // will restart the tracking pipeline, which also means the user will have to
  // wait for re-initialization of the motion tracking system.
  private Button mMotionReset;

  // Auto recovery flag received from StartActivity.
  private boolean mIsAutoRecovery;
  // GLSurfaceView and its renderer, all of the graphic content is rendered
  // through OpenGL ES 2.0 in the native code.
  private MotionTrackingRenderer mRenderer;
  private GLSurfaceView mGLView;

  // Screen size for normalizing the touch input for orbiting the render camera.
  private Point mScreenSize = new Point();

  // A flag to check if the Tango Service is connected. This flag avoids the
  // program attempting to disconnect from the service while it is not
  // connected.This is especially important in the onPause() callback for the
  // activity class.
  private boolean mIsConnectedService = false;

  // Handles the debug text UI update loop.
  private Handler mHandler = new Handler();

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setTitle(R.string.app_name);
    
    // Querying screen size, used for computing the normalized touch point.
    Display display = getWindowManager().getDefaultDisplay();
    display.getSize(mScreenSize);
    
    // Setting content view of this activity and getting the mIsAutoRecovery
    // flag from StartActivity.
    setContentView(R.layout.activity_motion_tracking);
    Intent intent = getIntent();
    mIsAutoRecovery = intent.getBooleanExtra(
        StartActivity.KEY_MOTIONTRACKING_AUTO_RECOVERY, false);
    
    // Text views for displaying translation and rotation data
    mPoseData = (TextView) findViewById(R.id.pose_data_textview);

    // Text views for displaying most recent Tango Event
    mEvent = (TextView) findViewById(R.id.tango_event_textview);

    // Text views for Tango library versions
    mVersion = (TextView) findViewById(R.id.version_textview);

    // Text views for application versions.
    mAppVersion = (TextView) findViewById(R.id.appversion);
    PackageInfo pInfo;
    try {
      pInfo = this.getPackageManager().getPackageInfo(this.getPackageName(), 0);
      mAppVersion.setText(pInfo.versionName);
    } catch (NameNotFoundException e) {
      e.printStackTrace();
    }

    // Buttons for selecting camera view and Set up button click listeners
    findViewById(R.id.first_person_button).setOnClickListener(this);
    findViewById(R.id.third_person_button).setOnClickListener(this);
    findViewById(R.id.top_down_button).setOnClickListener(this);

    // Button to reset motion tracking
    mMotionReset = (Button) findViewById(R.id.resetmotion);

    // OpenGL view where all of the graphics are drawn
    mGLView = (GLSurfaceView) findViewById(R.id.gl_surface_view);
    
    // Configure OpenGL renderer
    mGLView.setEGLContextClientVersion(2);

    // Set up button click listeners
    mMotionReset.setOnClickListener(this);

    // Configure OpenGL renderer
    mRenderer = new MotionTrackingRenderer();
    mGLView.setRenderer(mRenderer);

    // Check if the Tango Core is out dated.
    if (!CheckTangoCoreVersion(MIN_TANGO_CORE_VERSION)) {
      Toast.makeText(this, "Tango Core out dated, please update in Play Store", 
                     Toast.LENGTH_LONG).show();
      finish();
      return;
    }

    // Initialize Tango Service, this function starts the communication
    // between the application and Tango Service.
    // The activity object is used for checking if the API version is outdated.
    TangoJNINative.initialize(this);
  }

  @Override
  protected void onResume() {
    super.onResume();
    mGLView.onResume();

    // Setup the configuration for the TangoService, passing in our setting
    // for the auto-recovery option.
    TangoJNINative.setupConfig(mIsAutoRecovery);
    
    // Connect the onPoseAvailable callback.
    TangoJNINative.connectCallbacks();

    // Connect to Tango Service (returns true on success).
    // Starts Motion Tracking and Area Learning.
    if (TangoJNINative.connect() == TANGO_SUCCESS) {
      mIsConnectedService = true;
      // Take the TangoCore version number from Tango Service.
      mVersion.setText(TangoJNINative.getVersionNumber());
    } else {
      // End the activity and let the user know something went wrong.
      Toast.makeText(this, "Connect Tango Service Error", Toast.LENGTH_LONG).show();
      finish();
    }

    // Start the debug text UI update loop.
    mHandler.post(mUpdateUiLoopRunnable);
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

    // Stop the debug text UI update loop.
    mHandler.removeCallbacksAndMessages(null);
  }

  @Override
  public void onClick(View v) {
    // Handle button clicks.
    switch (v.getId()) {
      case R.id.first_person_button:
        TangoJNINative.setCamera(0);
        break;
      case R.id.top_down_button:
        TangoJNINative.setCamera(2);
        break;
      case R.id.third_person_button:
        TangoJNINative.setCamera(1);
        break;
      case R.id.resetmotion:
        TangoJNINative.resetMotionTracking();
        break;
      default:
        Log.w(TAG, "Unknown button click");
        return;
    }
  }

  @Override
  public boolean onTouchEvent(MotionEvent event) {
    // Pass the touch event to the native layer for camera control.
    // Single touch to rotate the camera around the device.
    // Two fingers to zoom in and out.
    int pointCount = event.getPointerCount();
    if (pointCount == 1) {
      float normalizedX = event.getX(0) / mScreenSize.x;
      float normalizedY = event.getY(0) / mScreenSize.y;
      TangoJNINative.onTouchEvent(1, 
          event.getActionMasked(), normalizedX, normalizedY, 0.0f, 0.0f);
    }
    if (pointCount == 2) {
      if (event.getActionMasked() == MotionEvent.ACTION_POINTER_UP) {
        int index = event.getActionIndex() == 0 ? 1 : 0;
        float normalizedX = event.getX(index) / mScreenSize.x;
        float normalizedY = event.getY(index) / mScreenSize.y;
        TangoJNINative.onTouchEvent(1, 
          MotionEvent.ACTION_DOWN, normalizedX, normalizedY, 0.0f, 0.0f);
      } else {
        float normalizedX0 = event.getX(0) / mScreenSize.x;
        float normalizedY0 = event.getY(0) / mScreenSize.y;
        float normalizedX1 = event.getX(1) / mScreenSize.x;
        float normalizedY1 = event.getY(1) / mScreenSize.y;
        TangoJNINative.onTouchEvent(2, event.getActionMasked(),
            normalizedX0, normalizedY0, normalizedX1, normalizedY1);
      }
    }
    return true;
  }

  // Debug text UI update loop, updating at 10Hz.
  private Runnable mUpdateUiLoopRunnable = new Runnable() {
    public void run() {
      updateUi();
      mHandler.postDelayed(this, UPDATE_UI_INTERVAL_MS);
    }
  };

  // Update the debug text UI.
  private void updateUi() {
    try {
      mEvent.setText(TangoJNINative.getEventString());
      mPoseData.setText(TangoJNINative.getPoseString());
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  private boolean CheckTangoCoreVersion(int minVersion) {
    int versionNumber = 0;
    String packageName = TANGO_PACKAGE_NAME;
    try {
      PackageInfo pi = getApplicationContext().getPackageManager().getPackageInfo(packageName,
          PackageManager.GET_META_DATA);
      versionNumber = pi.versionCode;
    } catch (NameNotFoundException e) {
      e.printStackTrace();
    }
    return (minVersion <= versionNumber);
  }
}
