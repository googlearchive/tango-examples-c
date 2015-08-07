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

package com.projecttango.experiments.nativeaugmentedreality;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Point;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

// The main activity of the application which shows debug information and a
// glSurfaceView that renders graphic content.
public class AugmentedRealityActivity extends Activity implements
    View.OnClickListener {
  // The user has not given permission to use Motion Tracking functionality.
  private static final int TANGO_NO_MOTION_TRACKING_PERMISSION = -3;
  // The input argument is invalid.
  private static final int  TANGO_INVALID = -2;
  // This error code denotes some sort of hard error occurred.
  private static final int  TANGO_ERROR = -1;
  // This code indicates success.
  private static final int  TANGO_SUCCESS = 0;

  // Tag for debug logging.
  private static final String TAG =
      AugmentedRealityActivity.class.getSimpleName();

  // Motion Tracking permission request action.
  private static final String MOTION_TRACKING_PERMISSION_ACTION =
      "android.intent.action.REQUEST_TANGO_PERMISSION";

  // Key string for requesting and checking Motion Tracking permission.
  private static final String MOTION_TRACKING_PERMISSION =
      "MOTION_TRACKING_PERMISSION";

  // The interval at which we'll update our UI debug text in milliseconds.
  // This is the rate at which we query our native wrapper around the tango
  // service for pose and event information.
  private static final int kUpdateIntervalMs = 100;

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

  // GLSurfaceView and its renderer, all of the graphic content is rendered
  // through OpenGL ES 2.0 in the native code.
  private AugmentedRealityRenderer mRenderer;
  private GLSurfaceView mGLView;

  // A flag to check if the Tango Service is connected. This flag avoids the
  // program attempting to disconnect from the service while it is not
  // connected.This is especially important in the onPause() callback for the
  // activity class.
  private boolean mIsConnectedService = false;

  // Screen size for normalizing the touch input for orbiting the render camera.
  private Point mScreenSize = new Point();

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setTitle(R.string.app_name);

    getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                         WindowManager.LayoutParams.FLAG_FULLSCREEN);

    // Querying screen size, used for computing the normalized touch point.
    Display display = getWindowManager().getDefaultDisplay();
    display.getSize(mScreenSize);

    // Setting content view of this activity and getting the mIsAutoRecovery
    // flag from StartActivity.
    setContentView(R.layout.activity_augmented_reality);

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

    // Set up button click listeners
    mMotionReset.setOnClickListener(this);

    // Configure OpenGL renderer. The RENDERMODE_WHEN_DIRTY is set explicitly
    // for reducing the CPU load. The request render function call is triggered
    // by the onTextureAvailable callback from the Tango Service in the native
    // code.
    mRenderer = new AugmentedRealityRenderer();
    mGLView.setRenderer(mRenderer);
    mGLView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

    // Initialize Tango Service, this function starts the communication
    // between the application and Tango Service.
    // The activity object is used for checking if the API version is outdated.
    TangoJNINative.initialize(this);

    // UI thread handles the task of updating all debug text.
    startUIThread();
  }

  @Override
  protected void onResume() {
    super.onResume();
    mGLView.onResume();

    // In the onResume function, we first check if the MOTION_TRACKING_PERMISSION is
    // granted to this application, if not, we send a permission intent to
    // the Tango Service to launch the permission activity.
    // Note that the onPause() callback will be called once the permission
    // activity is foregrounded.
    if (!Util.hasPermission(getApplicationContext(),
                           MOTION_TRACKING_PERMISSION)) {
      getMotionTrackingPermission();
    } else {
      // If motion tracking permission is granted to the application, we can
      // connect to the Tango Service. For this example, we'll be calling
      // through the JNI to the C++ code that actually interfaces with the
      // service.

      // Setup the configuration for the TangoService.
      TangoJNINative.setupConfig();

      // Connect the onPoseAvailable callback.
      TangoJNINative.connectCallbacks();

      // Connect to Tango Service.
      // This function will start the Tango Service pipeline, in this case,
      // it will start Motion Tracking.
      TangoJNINative.connect();

      // Take the TangoCore version number from Tango Service.
      mVersion.setText(TangoJNINative.getVersionNumber());

      // Set the connected service flag to true.
      mIsConnectedService = true;
    }
  }

  @Override
  protected void onPause() {
    super.onPause();
    mGLView.onPause();
    TangoJNINative.freeGLContent();

    // If the service is connected, we disconnect it here.
    if (mIsConnectedService) {
      mIsConnectedService = false;
      // Disconnect from Tango Service, release all the resources that the app is
      // holding from Tango Service.
      TangoJNINative.disconnect();
    }
  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
    if (mIsConnectedService) {
      mIsConnectedService = false;
      TangoJNINative.disconnect();
    }
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

  // Call the permission intent for the Tango Service to ask for motion tracking
  // permissions. All permission types can be found here:
  //   https://developers.google.com/project-tango/apis/c/c-user-permissions
  private void getMotionTrackingPermission() {
    Intent intent = new Intent();
    intent.setAction(MOTION_TRACKING_PERMISSION_ACTION);
    intent.putExtra("PERMISSIONTYPE", MOTION_TRACKING_PERMISSION);

    // After the permission activity is dismissed, we will receive a callback
    // function onActivityResult() with user's result.
    startActivityForResult(intent, 0);
  }

  @Override
  protected void onActivityResult (int requestCode, int resultCode, Intent data) {
    // The result of the permission activity.
    //
    // Note that when the permission activity is dismissed, the
    // MotionTrackingActivity's onResume() callback is called. As the
    // TangoService is connected in the onResume() function, we do not call
    // connect here.
    if (requestCode == 0) {
      if (resultCode == RESULT_CANCELED) {
        mIsConnectedService = false;
        finish();
      }
    }
  }

  // Request render on the glSurfaceView. This function is called from the 
  // native code, and it is triggered from the onTextureAvailable callback from
  // the Tango Service.
  public void requestRender() {
    mGLView.requestRender();
  }

  // UI thread for handling debug text changes.
  private void startUIThread() {
    new Thread(new Runnable() {
      @Override
      public void run() {
        while (true) {
          try {
            Thread.sleep(kUpdateIntervalMs);
            runOnUiThread(new Runnable() {
              @Override
              public void run() {
                try {
                  mEvent.setText(TangoJNINative.getEventString());
                  mPoseData.setText(TangoJNINative.getPoseString());
                } catch (Exception e) {
                  e.printStackTrace();
                }
              }
            });

          } catch (Exception e) {
            e.printStackTrace();
          }
        }
      }
    }).start();
  }
}
