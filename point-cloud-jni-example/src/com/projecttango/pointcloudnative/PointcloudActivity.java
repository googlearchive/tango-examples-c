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

package com.projecttango.pointcloudnative;

import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import android.app.Activity;
import android.graphics.Point;

public class PointcloudActivity extends Activity implements OnClickListener {
  public static final String EXTRA_KEY_PERMISSIONTYPE = "PERMISSIONTYPE";
  public static final String EXTRA_VALUE_MOTION_TRACKING = "MOTION_TRACKING_PERMISSION";

  GLSurfaceView glView;

  private TextView mPoseDataTextView;
  private TextView mTangoEventTextView;
  private TextView mPointCountTextView;
  private TextView mVersionTextView;
  private TextView mAverageZTextView;
  private TextView mFrameDeltaTimeTextView;
  private TextView mAppVersion;

  private float[] touchStartPos = new float[2];
  private float[] touchCurPos = new float[2];
  private float touchStartDist = 0.0f;
  private float touchCurDist = 0.0f;
  private Point screenSize = new Point();
  private float screenDiagnal = 0.0f;

  private boolean mIsPermissionIntentCalled = false;

  private final int kTextUpdateIntervalms = 100;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    setTitle(R.string.app_name);

    Display display = getWindowManager().getDefaultDisplay();
    display.getSize(screenSize);
    screenDiagnal = (float) Math.sqrt(screenSize.x * screenSize.x
        + screenSize.y * screenSize.y);

    setContentView(R.layout.activity_pointcloud);

    // Text views for the status of the pose data and Tango library version.
    mVersionTextView = (TextView) findViewById(R.id.version);

    // Text views for the available points count.
    mPointCountTextView = (TextView) findViewById(R.id.pointCount);

    // Text view for average depth distance (in meters). 
    mAverageZTextView = (TextView) findViewById(R.id.averageZ);

    // Text view for fram delta time between two depth frame.
    mFrameDeltaTimeTextView = (TextView) findViewById(R.id.frameDelta);

    // Text views for displaying most recent Tango Event.
    mTangoEventTextView = (TextView) findViewById(R.id.tangoevent);

    // Text views for displaying translation and rotation data.
    mPoseDataTextView = (TextView) findViewById(R.id.pose_data_textview);

    // Text views for application versions.
    mAppVersion = (TextView) findViewById(R.id.appversion);
    PackageInfo pInfo;
    try {
      pInfo = this.getPackageManager().getPackageInfo(this.getPackageName(), 0);
      mAppVersion.setText(pInfo.versionName);
    } catch (NameNotFoundException e) {
      e.printStackTrace();
    }

    // Buttons for selecting camera view and Set up button click listeners.
    findViewById(R.id.first_person_button).setOnClickListener(this);
    findViewById(R.id.third_person_button).setOnClickListener(this);
    findViewById(R.id.top_down_button).setOnClickListener(this);

    // OpenGL view where all of the graphics are drawn.
    glView = (GLSurfaceView) findViewById(R.id.gl_surface_view);
    glView.setRenderer(new Renderer());

    startUIThread();
  }

  @Override
  protected void onPause() {
    super.onPause();
    glView.onPause();
    // Disconnect Tango Service.
    TangoJNINative.Disconnect();
    TangoJNINative.OnDestroy();
    mIsPermissionIntentCalled = false;
  }

  @Override
  protected void onResume() {
    Log.i("tango_jni", "on Resume");
    super.onResume();
    glView.onResume();
    if (!mIsPermissionIntentCalled) {
      Intent intent = new Intent();
      intent.setAction("android.intent.action.REQUEST_TANGO_PERMISSION");
      intent.putExtra(EXTRA_KEY_PERMISSIONTYPE, EXTRA_VALUE_MOTION_TRACKING);
      startActivityForResult(intent, 0);
    }
  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
  }

  @Override
  public void onClick(View v) {
    switch (v.getId()) {
    case R.id.first_person_button:
      TangoJNINative.SetCamera(0);
      break;
    case R.id.third_person_button:
      TangoJNINative.SetCamera(1);
      break;
    case R.id.top_down_button:
      TangoJNINative.SetCamera(2);
      break;
    default:
      return;
    }
  }

  @Override
  protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    // Check which request we're responding to.
    if (requestCode == 0) {
        // Make sure the request was successful.
        if (resultCode == RESULT_CANCELED ) {
          Toast.makeText(this, 
            "Motion Tracking Permission Needed!", Toast.LENGTH_SHORT).show();
          finish();
        } else {
          // Initialize the Tango service.
          int err = TangoJNINative.Initialize(this);
          if (err != 0) {
            if (err == -2) {
              Toast.makeText(this,
                "Tango Service version mis-match", Toast.LENGTH_SHORT).show();
            } else {
              Toast.makeText(this,
                "Tango Service initialize internal error", Toast.LENGTH_SHORT).show();
            }
          }

          // Set up Tango configuration with auto-reset on.
          TangoJNINative.SetupConfig();

          // Set Tango Service's version number.
          mVersionTextView.setText(TangoJNINative.GetVersionNumber());

          // Connect Tango Service
          err =  TangoJNINative.Connect();
          if (err != 0) {
            Toast.makeText(this, 
                "Tango Service connect error", Toast.LENGTH_SHORT).show();
          }
          TangoJNINative.SetupExtrinsics();
          mIsPermissionIntentCalled = true;
        }
    }
  }

  @Override
  public boolean onTouchEvent(MotionEvent event) {
    int pointCount = event.getPointerCount();
    if (pointCount == 1) {
      switch (event.getActionMasked()) {
      case MotionEvent.ACTION_DOWN: {
        TangoJNINative.StartSetCameraOffset();
        touchCurDist = 0.0f;
        touchStartPos[0] = event.getX(0);
        touchStartPos[1] = event.getY(0);
        break;
      }
      case MotionEvent.ACTION_MOVE: {
        touchCurPos[0] = event.getX(0);
        touchCurPos[1] = event.getY(0);

        // Normalize to screen width.
        float normalizedRotX = (touchCurPos[0] - touchStartPos[0])
            / screenSize.x;
        float normalizedRotY = (touchCurPos[1] - touchStartPos[1])
            / screenSize.y;

        TangoJNINative.SetCameraOffset(normalizedRotX, normalizedRotY,
            touchCurDist / screenDiagnal);
        break;
      }
      }
    }
    if (pointCount == 2) {
      switch (event.getActionMasked()) {
      case MotionEvent.ACTION_POINTER_DOWN: {
        TangoJNINative.StartSetCameraOffset();
        float absX = event.getX(0) - event.getX(1);
        float absY = event.getY(0) - event.getY(1);
        touchStartDist = (float) Math.sqrt(absX * absX + absY * absY);
        break;
      }
      case MotionEvent.ACTION_MOVE: {
        float absX = event.getX(0) - event.getX(1);
        float absY = event.getY(0) - event.getY(1);

        touchCurDist = touchStartDist
            - (float) Math.sqrt(absX * absX + absY * absY);

        TangoJNINative.SetCameraOffset(0.0f, 0.0f, touchCurDist
            / screenDiagnal);
        break;
      }
      case MotionEvent.ACTION_POINTER_UP: {
        int index = event.getActionIndex() == 0 ? 1 : 0;
        touchStartPos[0] = event.getX(index);
        touchStartPos[1] = event.getY(index);
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
              Thread.sleep(kTextUpdateIntervalms);
              runOnUiThread(new Runnable() {
                @Override
                public void run() {
                  try {
                    mTangoEventTextView.setText(TangoJNINative.GetEventString());
                    mPoseDataTextView.setText(TangoJNINative.GetPoseString());
                    mPointCountTextView.setText(String.valueOf(TangoJNINative.GetVerticesCount()));
                    mAverageZTextView.setText(String.format("%.3f", TangoJNINative.GetAverageZ()));
                    mFrameDeltaTimeTextView.setText(String.format("%.3f", TangoJNINative.GetFrameDeltaTime()));
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
