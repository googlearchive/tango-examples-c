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
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.graphics.Point;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.TextView;
import android.widget.Toast;

import com.projecttango.examples.cpp.util.TangoInitializationHelper;

/**
 * The main activity of the application. This activity shows debug information
 * and a glSurfaceView that renders graphic content.
 */
public class PointcloudActivity extends Activity implements OnClickListener {
  // The minimum Tango Core version required from this application.
  private static final int  MIN_TANGO_CORE_VERSION = 9377;

  // The package name of Tang Core, used for checking minimum Tango Core version.
  private static final String TANGO_PACKAGE_NAME = "com.projecttango.tango";

  // Tag for debug logging.
  private static final String TAG = PointcloudActivity.class.getSimpleName();

  // The interval at which we'll update our UI debug text in milliseconds.
  // This is the rate at which we query our native wrapper around the tango
  // service for pose and event information.
  private static final int UPDATE_UI_INTERVAL_MS = 100;

  // Total points count in the current depth frame.
  private TextView mPointCount;
  // Average depth value (in meteres) of all the points in the current frame.
  private TextView mAverageZ;

  // GLSurfaceView and renderer, all of the graphic content is rendered
  // through OpenGL ES 2.0 in native code.
  private Renderer mRenderer;
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

  // Tango Service connection.
  ServiceConnection mTangoServiceConnection = new ServiceConnection() {
    public void onServiceConnected(ComponentName name, IBinder service) {
      TangoJNINative.onTangoServiceConnected(service);
    }

    public void onServiceDisconnected(ComponentName name) {
      // Handle this if you need to gracefully shutdown/retry
      // in the event that Tango itself crashes/gets upgraded while running.
    }
  };

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    // Query screen size, the screen size is used for computing the normalized
    // touch point.
    Display display = getWindowManager().getDefaultDisplay();
    display.getSize(mScreenSize);
    TangoJNINative.setScreenRotation(display.getOrientation());

    // Setting content view of this activity.
    setContentView(R.layout.activity_pointcloud);

    // Text views for the available points count.
    mPointCount = (TextView) findViewById(R.id.point_count);

    // Text view for average depth distance (in meters).
    mAverageZ = (TextView) findViewById(R.id.average_depth);

    // Buttons for selecting camera view and Set up button click listeners.
    findViewById(R.id.first_person_button).setOnClickListener(this);
    findViewById(R.id.third_person_button).setOnClickListener(this);
    findViewById(R.id.top_down_button).setOnClickListener(this);

    // OpenGL view where all of the graphics are drawn.
    mGLView = (GLSurfaceView) findViewById(R.id.gl_surface_view);

    // Configure OpenGL renderer
    mGLView.setEGLContextClientVersion(2);

    // Configure the OpenGL renderer.
    mRenderer = new Renderer();
    mGLView.setRenderer(mRenderer);

    TangoJNINative.onCreate(this);
  }

  @Override
  protected void onResume() {
    super.onResume();
    mGLView.onResume();

    TangoInitializationHelper.bindTangoService(this, mTangoServiceConnection);

    // Start the debug text UI update loop.
    mHandler.post(mUpdateUiLoopRunnable);
  }

  @Override
  protected void onPause() {
    super.onPause();
    mGLView.onPause();
    TangoJNINative.onPause();

    // Stop the debug text UI update loop.
    mHandler.removeCallbacksAndMessages(null);
    unbindService(mTangoServiceConnection);
  }

  @Override
  public void onClick(View v) {
    // Handle button clicks.
    switch (v.getId()) {
    case R.id.first_person_button:
      TangoJNINative.setCamera(0);
      break;
    case R.id.third_person_button:
      TangoJNINative.setCamera(1);
      break;
    case R.id.top_down_button:
      TangoJNINative.setCamera(2);
      break;
    default:
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
      TangoJNINative.onTouchEvent(1, event.getActionMasked(),
                                  normalizedX, normalizedY, 0.0f, 0.0f);
    }
    if (pointCount == 2) {
      if (event.getActionMasked() == MotionEvent.ACTION_POINTER_UP) {
        int index = event.getActionIndex() == 0 ? 1 : 0;
        float normalizedX = event.getX(index) / mScreenSize.x;
        float normalizedY = event.getY(index) / mScreenSize.y;
        TangoJNINative.onTouchEvent(1, MotionEvent.ACTION_DOWN,
                                    normalizedX, normalizedY, 0.0f, 0.0f);
      } else {
        float normalizedX0 = event.getX(0) / mScreenSize.x;
        float normalizedY0 = event.getY(0) / mScreenSize.y;
        float normalizedX1 = event.getX(1) / mScreenSize.x;
        float normalizedY1 = event.getY(1) / mScreenSize.y;
        TangoJNINative.onTouchEvent(2, event.getActionMasked(),
                                    normalizedX0, normalizedY0,
                                    normalizedX1, normalizedY1);
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
      mPointCount.setText(String.valueOf(TangoJNINative.getVerticesCount()));
      mAverageZ.setText(String.format("%.3f", TangoJNINative.getAverageZ()));
    } catch (Exception e) {
      e.printStackTrace();
      Log.e(TAG, "Exception updateing UI elements");
    }
  }
}
