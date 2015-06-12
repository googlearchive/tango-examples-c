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
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Point;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView;
import android.widget.Toast;
import java.util.Timer;
import java.util.TimerTask;
/**
 * Main activity shows augmented reality scene.
 */
public class AugmentedRealityActivity extends Activity implements View.OnClickListener{

    public static final String EXTRA_KEY_PERMISSIONTYPE = "PERMISSIONTYPE";
    public static final String EXTRA_VALUE_VIO = "MOTION_TRACKING_PERMISSION";
    public static final String EXTRA_VALUE_VIOADF = "ADF_LOAD_SAVE_PERMISSION";

    private GLSurfaceView arView;
    private TextView tangoPoseStatusText;

    private float[] touchStartPos = new float[2];
    private float[] touchCurPos = new float[2];
    private float touchStartDist = 0.0f;
    private float touchCurDist = 0.0f;
    private Point screenSize = new Point();
    private float screenDiagnal = 0.0f;
    private String appVersionString;
    private Timer timer;
    private TimerTask refresher;
    private int arElement = 0;
    private int interactionType = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

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

        setTitle(R.string.app_name);
        setContentView(R.layout.activity_augmented_reality);

        arView = (GLSurfaceView) findViewById(R.id.surfaceview);

        AugmentedRealityView arViewRenderer = new AugmentedRealityView();
        arViewRenderer.activity = AugmentedRealityActivity.this;
        arViewRenderer.isAutoRecovery = true;
        arView.setRenderer(arViewRenderer);
        arView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        tangoPoseStatusText = (TextView) findViewById(R.id.debug_info);

        PackageInfo pInfo;
        try {
            pInfo = this.getPackageManager().getPackageInfo(this.getPackageName(), 0);
            appVersionString = pInfo.versionName;
        } catch (NameNotFoundException e) {
            e.printStackTrace();
            appVersionString = " ";
        }

        // Create a timer to request a refresh at 30 Hz.
        timer = new Timer();
        refresher = new TimerTask() {
            public void run() {
                arView.requestRender();
            };
        };
        // Wait 2 seconds, then refresh at a 33 ms period.
        timer.scheduleAtFixedRate(refresher, 2000, 33);

        findViewById(R.id.reset).setOnClickListener(this);
        findViewById(R.id.third).setOnClickListener(this);
        findViewById(R.id.first).setOnClickListener(this);
        findViewById(R.id.top).setOnClickListener(this);
        findViewById(R.id.place).setOnClickListener(this);
/*
        new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    try {
                        Thread.sleep(10);

                        runOnUiThread(new Runnable() {
                            public void run() {
                                boolean isLocalized = TangoJNINative.getIsLocalized();
                                if(isLocalized) {
                                    findViewById(R.id.reset).setVisibility(View.GONE);
                                } else {
                                    findViewById(R.id.reset).setVisibility(View.VISIBLE);
                                }
                                tangoPoseStatusText.setText(
                                    "Service Version:" + TangoJNINative.getVersionNumber() +
                                    "\nApp Version:" + appVersionString +
                                    "\n" + TangoJNINative.getPoseString());
                            }
                        });

                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        }).start();
*/
    }
    @Override
    public void onClick(View v) {
        switch (v.getId()) {
        case R.id.reset:
            TangoJNINative.resetMotionTracking();
            break;
        case R.id.place:
            TangoJNINative.placeObject();
            break;
        case R.id.first:
            TangoJNINative.setCamera(0);
            break;
        case R.id.third:
            TangoJNINative.setCamera(1);
            break;
        case R.id.top:
            TangoJNINative.setCamera(2);
            break;
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        arView.onResume();
        getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                );
    }

    @Override
    protected void onPause() {
        super.onPause();
        arView.onPause();
        TangoJNINative.disconnectService();
    }

    protected void onDestroy() {
        super.onDestroy();
        TangoJNINative.onDestroy();
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
            TangoJNINative.updateARElement(arElement, interactionType);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int pointCount = event.getPointerCount();
        if (pointCount == 1) {
            switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN: {
                TangoJNINative.startSetCameraOffset();
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

                TangoJNINative.setCameraOffset(normalizedRotX, normalizedRotY,
                        touchCurDist / screenDiagnal);
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
                touchStartDist = (float) Math.sqrt(absX * absX + absY * absY);
                break;
            }
            case MotionEvent.ACTION_MOVE: {
                float absX = event.getX(0) - event.getX(1);
                float absY = event.getY(0) - event.getY(1);

                touchCurDist = touchStartDist
                        - (float) Math.sqrt(absX * absX + absY * absY);

                TangoJNINative.setCameraOffset(0.0f, 0.0f, touchCurDist
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
            if (resultCode == RESULT_CANCELED) {
                Toast.makeText(this,
                "Motion Tracking Permission Needed!", Toast.LENGTH_SHORT).show();
                finish();
            }
        }
        if (requestCode == 1) {
            if (resultCode == RESULT_CANCELED) {
                Toast.makeText(this,
                "ADF Permission Needed!", Toast.LENGTH_SHORT).show();
                finish();
            }
        }
    }
}

