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

package com.projecttango.areadescriptionnative;

import com.projecttango.areadescriptionnative.SetADFNameDialog.SetNameAndUUIDCommunicator;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.app.Activity;
import android.app.FragmentManager;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Point;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

public class AreaDescriptionActivity extends Activity implements
    View.OnClickListener, SetNameAndUUIDCommunicator {
  public static int TANGO_ERROR_INVALID = -2;
  public static int TANGO_ERROR_ERROR = -1;
  public static int TANGO_ERROR_SUCCESS = -0;

  GLSurfaceView glView;
  RelativeLayout layout;

  TextView eventString;
  TextView versionString;
  TextView device2StartText;
  TextView device2ADFText;
  TextView start2ADFText;

  TextView uuidText;

  TextView appVersionText;

  Button saveADFButton;
  Button startButton;
  Button firstPersonCamButton;
  Button thirdPersonCamButton;
  Button topDownCamButton;

  ToggleButton isUsingADFToggleButton;
  ToggleButton isLearningToggleButton;

  boolean isUsingADF = false;
  boolean isLearning = false;

  private float[] touchStartPos = new float[2];
  private float[] touchCurPos = new float[2];
  private float touchStartDist = 0.0f;
  private float touchCurDist = 0.0f;
  private Point screenSize = new Point();
  private float screenDiagonal = 0.0f;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    Display display = getWindowManager().getDefaultDisplay();
    display.getSize(screenSize);
    screenDiagonal = (float) Math.sqrt(screenSize.x * screenSize.x
        + screenSize.y * screenSize.y);
    
    setContentView(R.layout.activity_area_description);
    glView = (GLSurfaceView) findViewById(R.id.gl_surface_view);
    glView.setRenderer(new Renderer());

    eventString = (TextView) findViewById(R.id.tangoevent);
    versionString = (TextView) findViewById(R.id.version);
    device2StartText = (TextView) findViewById(R.id.device_start);
    device2ADFText = (TextView) findViewById(R.id.adf_device);
    start2ADFText = (TextView) findViewById(R.id.adf_start);
    uuidText = (TextView) findViewById(R.id.uuid);

    // Text views for application versions.
    appVersionText = (TextView) findViewById(R.id.appversion);
    PackageInfo pInfo;
    try {
      pInfo = this.getPackageManager().getPackageInfo(this.getPackageName(), 0);
      appVersionText.setText(pInfo.versionName);
    } catch (NameNotFoundException e) {
      e.printStackTrace();
    }

    Intent intent = getIntent();
    isLearning = intent.getBooleanExtra(StartActivity.USE_AREA_LEARNING,
        false);
    isUsingADF = intent.getBooleanExtra(StartActivity.LOAD_ADF, false);

    saveADFButton = (Button) findViewById(R.id.saveAdf);
    saveADFButton.setOnClickListener(this);
    firstPersonCamButton = (Button) findViewById(R.id.first_person_button);
    firstPersonCamButton.setOnClickListener(this);
    thirdPersonCamButton = (Button) findViewById(R.id.third_person_button);
    thirdPersonCamButton.setOnClickListener(this);
    topDownCamButton = (Button) findViewById(R.id.top_down_button);
    topDownCamButton.setOnClickListener(this);

    if (!isLearning) {
      saveADFButton.setVisibility(View.GONE);
    }

    int err = TangoJNINative.Initialize(this);
    if (err != TANGO_ERROR_SUCCESS) {
      if (err == TANGO_ERROR_INVALID) {
        Toast.makeText(this, 
          "Tango Service version mismatch", Toast.LENGTH_SHORT).show();
      }
      else {
        Toast.makeText(this, 
          "Tango Service initialize internal error", Toast.LENGTH_SHORT).show();
      }
    }

    if (!TangoJNINative.ConnectCallbacks()) {
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
    glView.onResume();
    TangoJNINative.SetupConfig(isLearning, isUsingADF);
    // Connect Tango Service
    int err =  TangoJNINative.Connect();
    if (err != 0) {
      Toast.makeText(this, 
          "Tango Service connect error", Toast.LENGTH_SHORT).show();
    }
  }

  @Override
  protected void onPause() {
    super.onPause();
    glView.onPause();
    TangoJNINative.Disconnect();
    TangoJNINative.OnDestroy();
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
    case R.id.top_down_button:
      TangoJNINative.SetCamera(2);
      break;
    case R.id.third_person_button:
      TangoJNINative.SetCamera(1);
      break;
    case R.id.saveAdf:
      if(TangoJNINative.SaveADF()) {
        showSetNameDialog(TangoJNINative.GetUUID());
      } else {
        Toast toast = Toast.makeText(getApplicationContext(), "UUID Save Error.",
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
    String name = TangoJNINative.GetUUIDMetadataValue(mCurrentUUID, "name");
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
  public void SetNameAndUUID(String name, String uuid) {
    TangoJNINative.SetUUIDMetadataValue(uuid, "name", name.length(), name);
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

        if (screenSize.x != 0.0f && screenSize.y != 0.0f && screenDiagonal != 0.0f) {
          // Normalize to screen width.
          float normalizedRotX = (touchCurPos[0] - touchStartPos[0])
              / screenSize.x;
          float normalizedRotY = (touchCurPos[1] - touchStartPos[1])
              / screenSize.y;

          TangoJNINative.SetCameraOffset(normalizedRotX, normalizedRotY,
              touchCurDist / screenDiagonal);
        }
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
            / screenDiagonal);
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
  
  private void updateUIs() {
    eventString.setText(TangoJNINative.GetEventString());
    versionString.setText(TangoJNINative.GetVersionString());

    device2StartText.setText(TangoJNINative.GetPoseString(0));
    device2ADFText.setText(TangoJNINative.GetPoseString(1));
    start2ADFText.setText(TangoJNINative.GetPoseString(2));

    uuidText.setText("Number of UUID: " + TangoJNINative.GetUUID());
  }
}
