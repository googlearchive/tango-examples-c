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

import android.app.Activity;
import android.app.FragmentManager;
import android.content.Intent;
import android.content.pm.PackageInfo;
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
import android.widget.ToggleButton;
import com.projecttango.experiments.nativearealearning.SetADFNameDialog.SetNameAndUUIDCommunicator;

/**
 * Main activity shows area learning scene.
 */
public class AreaDescriptionActivity extends Activity implements
    View.OnClickListener, SetNameAndUUIDCommunicator {
  public static final int TANGO_ERROR_INVALID = -2;
  public static final int TANGO_ERROR_ERROR = -1;
  public static final int TANGO_ERROR_SUCCESS = -0;

  private GLSurfaceView mGLView;

  private TextView mTangoServiceVersionTextView;
  private TextView mAppVersionTextView;
  private TextView mTangoEventTextView;
  private TextView mDeviceToStartPoseTextView;
  private TextView mDeviceToADFPoseTextView;
  private TextView mStartToADFPoseTextView;
  private TextView mADFUUIDTextView;
  
  private Button mSaveADFButton;
  private Button mFirstPersonCamButton;
  private Button mThirdPersonCamButton;
  private Button mTopDownCamButton;

  private boolean mIsUsingADF = false;
  private boolean mIsLearning = false;

  private float[] mTouchStartPositionition = new float[2];
  private float[] mTouchCurrentPosition = new float[2];
  private float mTouchStartDist = 0.0f;
  private float mTouchCurrentDist = 0.0f;
  private Point mScreenSize = new Point();
  private float mScreenDiagonalDist = 0.0f;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    // Calculate screen width for touch interaction.
    Display display = getWindowManager().getDefaultDisplay();
    display.getSize(mScreenSize);
    mScreenDiagonalDist = (float) Math.sqrt(mScreenSize.x * mScreenSize.x
        + mScreenSize.y * mScreenSize.y);
    
    setContentView(R.layout.activity_area_description);
    mGLView = (GLSurfaceView) findViewById(R.id.gl_surface_view);
    mGLView.setRenderer(new Renderer());

    mTangoEventTextView = (TextView) findViewById(R.id.tangoevent);
    mTangoServiceVersionTextView = (TextView) findViewById(R.id.version);
    mDeviceToStartPoseTextView = (TextView) findViewById(R.id.device_start);
    mDeviceToADFPoseTextView = (TextView) findViewById(R.id.adf_device);
    mStartToADFPoseTextView = (TextView) findViewById(R.id.adf_start);
    mADFUUIDTextView = (TextView) findViewById(R.id.uuid);

    // Text views for application versions.
    mAppVersionTextView = (TextView) findViewById(R.id.appversion);
    PackageInfo pInfo;
    try {
      pInfo = this.getPackageManager().getPackageInfo(this.getPackageName(), 0);
      mAppVersionTextView.setText(pInfo.versionName);
    } catch (NameNotFoundException e) {
      e.printStackTrace();
    }

    Intent initValueIntent = getIntent();
    mIsLearning = initValueIntent.getBooleanExtra(StartActivity.USE_AREA_LEARNING,
        false);
    mIsUsingADF = initValueIntent.getBooleanExtra(StartActivity.LOAD_ADF, false);

    mSaveADFButton = (Button) findViewById(R.id.saveAdf);
    mSaveADFButton.setOnClickListener(this);
    mFirstPersonCamButton = (Button) findViewById(R.id.first_person_button);
    mFirstPersonCamButton.setOnClickListener(this);
    mThirdPersonCamButton = (Button) findViewById(R.id.third_person_button);
    mThirdPersonCamButton.setOnClickListener(this);
    mTopDownCamButton = (Button) findViewById(R.id.top_down_button);
    mTopDownCamButton.setOnClickListener(this);

    if (!mIsLearning) {
      mSaveADFButton.setVisibility(View.GONE);
    }
    if (!mIsUsingADF) {
      mADFUUIDTextView.setVisibility(View.GONE);
    }

    int err = TangoJNINative.initialize(this);
    if (err != TANGO_ERROR_SUCCESS) {
      if (err == TANGO_ERROR_INVALID) {
        Toast.makeText(this, 
          "Tango Service version mismatch", Toast.LENGTH_SHORT).show();
      } else {
        Toast.makeText(this, 
          "Tango Service initialize internal error", Toast.LENGTH_SHORT).show();
      }
    }

    if (!TangoJNINative.connectCallbacks()) {
       Toast.makeText(this, 
          "Tango connect callback failed", Toast.LENGTH_SHORT).show();
     }

    new Thread(new Runnable() {
      @Override
      public void run() {
        while (true) {
          try {
            Thread.sleep(100);
            runOnUiThread(new Runnable() {
              @Override
              public void run() {
                try {
                  updateUIs();
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

  @Override
  protected void onResume() {
    super.onResume();
    mGLView.onResume();
    TangoJNINative.setupConfig(mIsLearning, mIsUsingADF);
    if (mIsUsingADF) {
      mADFUUIDTextView.setText("Number of ADFs: " + String.valueOf(TangoJNINative.getADFCount()) + 
          ", latest ADFs UUID: " + TangoJNINative.getUUID());
    }
    // Connect Tango Service
    int err =  TangoJNINative.connect();
    if (err != 0) {
      Toast.makeText(this, 
          "Tango Service connect error", Toast.LENGTH_SHORT).show();
    }
  }

  @Override
  protected void onPause() {
    super.onPause();
    mGLView.onPause();
    TangoJNINative.disconnect();
    TangoJNINative.freeGLContent();
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
    case R.id.saveAdf:
      if (TangoJNINative.saveADF()) {
        showSetNameDialog(TangoJNINative.getUUID());
      } else {
        Toast toast = Toast.makeText(getApplicationContext(), R.string.tango_invalid,
            Toast.LENGTH_SHORT);
        toast.show();
      }
      break;
    default:
      return;
    }
  }

  private void showSetNameDialog(String mCurrentUUID) {
    Bundle bundle = new Bundle();
    String name = TangoJNINative.getUUIDMetadataValue(mCurrentUUID, "name");
    if (name != null) {
      bundle.putString("name", name);
    }
    bundle.putString("id", mCurrentUUID);
    FragmentManager manager = getFragmentManager();
    SetADFNameDialog setADFNameDialog = new SetADFNameDialog();
    setADFNameDialog.setArguments(bundle);
    setADFNameDialog.show(manager, "ADFNameDialog");
  }

  @Override
  public void setNameAndUUID(String name, String uuid) {
    TangoJNINative.setUUIDMetadataValue(uuid, "name", name.length(), name);
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
  
  private void updateUIs() {
    mTangoEventTextView.setText(TangoJNINative.getEventString());
    mTangoServiceVersionTextView.setText(TangoJNINative.getVersionString());

    mDeviceToStartPoseTextView.setText(TangoJNINative.getPoseString(0));
    mDeviceToADFPoseTextView.setText(TangoJNINative.getPoseString(1));
    mStartToADFPoseTextView.setText(TangoJNINative.getPoseString(2));
  }
}
