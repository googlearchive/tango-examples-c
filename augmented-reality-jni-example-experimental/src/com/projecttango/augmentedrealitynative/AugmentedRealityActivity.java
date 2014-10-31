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

package com.projecttango.augmentedrealitynative;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Display;
import android.view.MotionEvent;
import android.graphics.Point;
import android.view.View;
import android.widget.Button;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

public class AugmentedRealityActivity extends Activity implements View.OnClickListener{

    public static final String EXTRA_KEY_PERMISSIONTYPE = "PERMISSIONTYPE";
    public static final String EXTRA_VALUE_VIO = "MOTION_TRACKING_PERMISSION";
    public static final String EXTRA_VALUE_VIOADF = "ADF_LOAD_SAVE_PERMISSION";

    private RelativeLayout layout;
    private GLSurfaceView arView;
    private TextView tangoPoseStatusText;
    private Button startButton;
    private Button resetButton;
    private Button thirdCamera;
    private Button firstCamera;
    private Button topCamera;
    private ToggleButton isAutoRecoveryButton;

    private float[] touchStartPos = new float[2];
    private float[] touchCurPos = new float[2];
    private float touchStartDist = 0.0f;
    private float touchCurDist = 0.0f;
    private Point screenSize = new Point();
    private float screenDiagnal = 0.0f;
    private String appVersionString;

    private boolean isAutoRecovery = false;
    private boolean isStarted = false;
    private int arElement = 0;
    private int interactionType = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Intent intent1 = new Intent();
        intent1.setAction("android.intent.action.REQUEST_TANGO_PERMISSION");
        intent1.putExtra(EXTRA_KEY_PERMISSIONTYPE, EXTRA_VALUE_VIO);
        startActivityForResult(intent1, 0);

        Intent intent2 = new Intent();
        intent2.setAction("android.intent.action.REQUEST_TANGO_PERMISSION");
        intent2.putExtra(EXTRA_KEY_PERMISSIONTYPE, EXTRA_VALUE_VIOADF);
        startActivityForResult(intent2, 0);

        Display display = getWindowManager().getDefaultDisplay();
        display.getSize(screenSize);
        screenDiagnal = (float) Math.sqrt(screenSize.x * screenSize.x
                + screenSize.y * screenSize.y);

        setContentView(R.layout.activity_augmented_reality);

        arView = (GLSurfaceView) findViewById(R.id.surfaceview);
        arView.setVisibility(View.GONE);

        AugmentedRealityView arViewRenderer = new AugmentedRealityView();
        arViewRenderer.activity = AugmentedRealityActivity.this;
        arViewRenderer.isAutoRecovery = isAutoRecovery;
        arView.setRenderer(arViewRenderer);

        tangoPoseStatusText = (TextView) findViewById(R.id.debug_info);
        tangoPoseStatusText.setVisibility(View.GONE);

        PackageInfo pInfo;
        try {
            pInfo = this.getPackageManager().getPackageInfo(this.getPackageName(), 0);
            appVersionString = pInfo.versionName;
        } catch (NameNotFoundException e) {
            e.printStackTrace();
            appVersionString = " ";
        }

        resetButton = (Button) findViewById(R.id.reset);
        isAutoRecoveryButton = (ToggleButton) findViewById(R.id.auto_recovery);
        isAutoRecoveryButton.setOnClickListener(this);

        resetButton.setOnClickListener(this);
        resetButton.setVisibility(View.GONE);

        thirdCamera=(Button)findViewById(R.id.third);
        thirdCamera.setVisibility(View.GONE);
        thirdCamera.setOnClickListener(this);

        firstCamera=(Button)findViewById(R.id.first);
        firstCamera.setVisibility(View.GONE);
        firstCamera.setOnClickListener(this);

        topCamera=(Button)findViewById(R.id.top);
        topCamera.setVisibility(View.GONE);
        topCamera.setOnClickListener(this);

        startButton = (Button) findViewById(R.id.start);
        startButton.setOnClickListener(this);

        new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    try {
                        Thread.sleep(10);

                        runOnUiThread(new Runnable() {
                            public void run() {
                                tangoPoseStatusText
                                        .setText("Service Version:" + TangoJNINative.GetVersionNumber() +
                                                 "\nApp Version:" + appVersionString +
                                                 "\n" + TangoJNINative.GetPoseString());
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
    public void onClick(View v) {
        switch (v.getId()) {
        case R.id.reset:
            TangoJNINative.ResetMotionTracking();
            break;
        case R.id.auto_recovery:
            isAutoRecovery = isAutoRecoveryButton.isChecked();
            break;
        case R.id.first:
            TangoJNINative.SetCamera(0);
            break;
        case R.id.third:
            TangoJNINative.SetCamera(1);
            break;
        case R.id.top:
            TangoJNINative.SetCamera(2);
            break;
        case R.id.start:
            if (!isAutoRecovery) {
                resetButton.setVisibility(View.VISIBLE);
            }
            arView.setVisibility(View.VISIBLE);
            startButton.setVisibility(View.GONE);
            isAutoRecoveryButton.setVisibility(View.GONE);
            thirdCamera.setVisibility(View.VISIBLE);
            firstCamera.setVisibility(View.VISIBLE);
            topCamera.setVisibility(View.VISIBLE);
            tangoPoseStatusText.setVisibility(View.VISIBLE);
            isStarted = true;
            break;
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        arView.onResume();
        getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
    }

    @Override
    protected void onPause() {
        super.onPause();
        arView.onPause();
        TangoJNINative.DisconnectService();
    }

    protected void onDestroy() {
        super.onDestroy();
        TangoJNINative.OnDestroy();
    }

    public void onRadioButtonClicked(View view) {
        switch (view.getId()) {
        case R.id.radio_world:
            arElement = 1;
            break;
        case R.id.radio_cube:
            arElement = 2;
            break;
        case R.id.radio_grid:
            arElement = 3;
            break;
        case R.id.radio_fx:
            arElement = 4;
            break;
        }
    }

    public void onDirectionButtonClicked(View view) {
        switch (view.getId()) {
        case R.id.radio_left:
            interactionType = 1;
            break;
        case R.id.radio_right:
            interactionType = 2;
            break;
        case R.id.radio_down:
            interactionType = 3;
            break;
        case R.id.radio_up:
            interactionType = 4;
            break;
        case R.id.radio_far:
            interactionType = 5;
            break;
        case R.id.radio_near:
            interactionType = 6;
            break;
        }
        if (arElement != 0) {
            TangoJNINative.UpdateARElement(arElement, interactionType);
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
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == 0) {
            if (resultCode == RESULT_CANCELED ){
                Toast.makeText(this,
                "Motion Tracking Permission Needed!", Toast.LENGTH_SHORT).show();
                finish();
            }
        }
        if (requestCode == 1) {
            if (resultCode == RESULT_CANCELED ){
                Toast.makeText(this,
                "ADF Permission Needed!", Toast.LENGTH_SHORT).show();
                finish();
            }
        }
    }
}

