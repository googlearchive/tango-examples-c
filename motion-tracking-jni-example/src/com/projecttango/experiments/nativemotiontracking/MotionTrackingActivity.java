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
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

/**
 * Main activity shows motion tracking scene.
 */
public class MotionTrackingActivity extends Activity implements
    View.OnClickListener {
  final int kUpdatIntervalMs = 100;

  private static final String TAG = 
      MotionTrackingActivity.class.getSimpleName();

  private TextView mPoseData;
  private TextView mVersion;
  private TextView mAppVersion;
  private TextView mEvent;

  private Button mMotionReset;

  private boolean mIsAutoRecovery;
  private MotionTrackingRenderer mRenderer;
  private GLSurfaceView mGLView;
  
  private float[] mTouchStartPositionition = new float[2];
  private float[] mTouchCurrentPosition = new float[2];
  private float mTouchStartDist = 0.0f;
  private float mTouchCurrentDist = 0.0f;
  private Point mScreenSize = new Point();
  private float mScreenDiagonalDist = 0.0f;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    
    setTitle(R.string.app_name);
    
    Display display = getWindowManager().getDefaultDisplay();
    display.getSize(mScreenSize);
    mScreenDiagonalDist = (float) Math.sqrt(mScreenSize.x * mScreenSize.x +
        mScreenSize.y * mScreenSize.y);
    
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

    // Set up button click listeners
    mMotionReset.setOnClickListener(this);

    // Configure OpenGL renderer
    mRenderer = new MotionTrackingRenderer();
    mGLView.setRenderer(mRenderer);

    // Initialize the Tango service
    int err = TangoJNINative.initialize(this);
    if (err != 0) {
      if (err == -2) {
        Toast.makeText(this, 
          "Tango Service version mis-match", Toast.LENGTH_SHORT).show();
      } else {
        Toast.makeText(this, 
          "Tango Service initialize internal error", Toast.LENGTH_SHORT).show();
      }
    }
    TangoJNINative.connectCallbacks();
    startUIThread();
  }

  @Override
  protected void onPause() {
    Log.i("tango_jni", "MotionTrackingActivity onPause");
    super.onPause();
    mGLView.onPause();
    TangoJNINative.disconnect();
    TangoJNINative.freeGLContent();
  }

  @Override
  protected void onResume() {
    super.onPause();

    mGLView.onResume();

    // Set up Tango configuration file with auto-reset on
    TangoJNINative.setupConfig(mIsAutoRecovery);

    // Connect Tango Service
    int err =  TangoJNINative.connect();
    if (err != 0) {
      Toast.makeText(this, 
          "Tango Service connect error", Toast.LENGTH_SHORT).show();
    }
    mVersion.setText(TangoJNINative.getVersionNumber());
  }
  
  @Override
  protected void onDestroy() {
    super.onDestroy();
  }

  @Override
  public void onClick(View v) {
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
    int pointCount = event.getPointerCount();
    if (pointCount == 1) {
      switch (event.getActionMasked()) {
      case MotionEvent.ACTION_DOWN: {
        TangoJNINative.startSetCameraOffset();
        mTouchCurrentDist = 0.0f;
        mTouchStartPositionition[0] = event.getX(0);
        mTouchStartPositionition[1] = event.getY(0);
        break;
      }
      case MotionEvent.ACTION_MOVE: {
        mTouchCurrentPosition[0] = event.getX(0);
        mTouchCurrentPosition[1] = event.getY(0);

        if (mScreenSize.x != 0.0f && mScreenSize.y != 0.0f && mScreenDiagonalDist != 0.0f) {
          // Normalize to screen width.
          float normalizedRotX = (mTouchCurrentPosition[0] - mTouchStartPositionition[0])
              / mScreenSize.x;
          float normalizedRotY = (mTouchCurrentPosition[1] - mTouchStartPositionition[1])
              / mScreenSize.y;

          TangoJNINative.setCameraOffset(normalizedRotX, normalizedRotY,
              mTouchCurrentDist / mScreenDiagonalDist);
        }
        break;
      }
      }
    }
    if (pointCount == 2) {
      switch (event.getActionMasked()) {
      case MotionEvent.ACTION_POINTER_DOWN: {
        TangoJNINative.startSetCameraOffset();
        float absX = event.getX(0) - event.getX(1);
        float absY = event.getY(0) - event.getY(1);
        mTouchStartDist = (float) Math.sqrt(absX * absX + absY * absY);
        break;
      }
      case MotionEvent.ACTION_MOVE: {
        float absX = event.getX(0) - event.getX(1);
        float absY = event.getY(0) - event.getY(1);

        mTouchCurrentDist = mTouchStartDist
            - (float) Math.sqrt(absX * absX + absY * absY);

        TangoJNINative.setCameraOffset(0.0f, 0.0f, mTouchCurrentDist
            / mScreenDiagonalDist);
        break;
      }
      case MotionEvent.ACTION_POINTER_UP: {
        int index = event.getActionIndex() == 0 ? 1 : 0;
        mTouchStartPositionition[0] = event.getX(index);
        mTouchStartPositionition[1] = event.getY(index);
        break;
      }
      }
    }
    return true;
  }

  private void startUIThread() {
    new Thread(new Runnable() {
      @Override
      public void run() {
        while (true) {
          try {
            Thread.sleep(kUpdatIntervalMs);
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
