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

package com.projecttango.experiments.rgbdepthsync;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.database.Cursor;
import android.graphics.Point;
import android.net.Uri;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.WindowManager;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.Toast;

/**
 * Activity that load up the main screen of the app, this is the launcher activity.
 */
public class MainActivity extends Activity {
    /// The input argument is invalid.
    private static final int  TANGO_INVALID = -2;
    /// This error code denotes some sort of hard error occurred.
    private static final int  TANGO_ERROR = -1;
    /// This code indicates success.
    private static final int  TANGO_SUCCESS = 0;

    private static final String TAG = MainActivity.class.getSimpleName();

    // The minimum Tango Core version required from this application.
    private static final int  MIN_TANGO_CORE_VERSION = 6804;

    // The package name of Tang Core, used for checking minimum Tango Core version.
    private static final String TANGO_PACKAGE_NAME = "com.projecttango.tango";

    private GLSurfaceRenderer mRenderer;
    private GLSurfaceView mGLView;

    private SeekBar mDepthOverlaySeekbar;
    private CheckBox mdebugOverlayCheckbox;
    private CheckBox mGPUUpsampleCheckbox;

    // A flag to check if the Tango Service is connected. This flag avoids the
    // program attempting to disconnect from the service while it is not
    // connected.This is especially important in the onPause() callback for the
    // activity class.
    private boolean mIsConnectedService = false;

    private class DepthOverlaySeekbarListener implements SeekBar.OnSeekBarChangeListener {
        @Override
        public void onProgressChanged(SeekBar seekBar, int progress,
                boolean fromUser) {
            JNIInterface.setDepthAlphaValue((float) progress / (float) seekBar.getMax());
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {}

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {}
    }

    private class DebugOverlayCheckboxListener implements CheckBox.OnCheckedChangeListener {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            if (buttonView == mdebugOverlayCheckbox) {
                if (isChecked) {
                    float progress = mDepthOverlaySeekbar.getProgress();
                    float max = mDepthOverlaySeekbar.getMax();
                    JNIInterface.setDepthAlphaValue(progress / max);
                    mDepthOverlaySeekbar.setVisibility(View.VISIBLE);
                } else {
                    JNIInterface.setDepthAlphaValue(0.0f);
                    mDepthOverlaySeekbar.setVisibility(View.GONE);
                }
            }
        }
    }

    private class GPUUpsampleListener implements CheckBox.OnCheckedChangeListener {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            JNIInterface.setGPUUpsample(isChecked);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Display display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        int status = JNIInterface.tangoInitialize(this);
        if (status != TANGO_SUCCESS) {
          if (status == TANGO_INVALID) {
            Toast.makeText(this, 
              "Tango Service version mis-match", Toast.LENGTH_SHORT).show();
          } else {
            Toast.makeText(this, 
              "Tango Service initialize internal error",
              Toast.LENGTH_SHORT).show();
          }
        }

        status = JNIInterface.tangoSetupConfig();
        if (status != TANGO_SUCCESS) {
            Log.e(TAG, "Failed to set config with code: "  + status);
            finish();
        }

        status = JNIInterface.tangoSetIntrinsicsAndExtrinsics();
        if (status != TANGO_SUCCESS) {
            Log.e(TAG, "Failed to set extrinsics and intrinsics code: "  + status);
            finish();
        }

        setContentView(R.layout.activity_main);

        mDepthOverlaySeekbar = (SeekBar) findViewById(R.id.depth_overlay_alpha_seekbar);
        mDepthOverlaySeekbar.setOnSeekBarChangeListener(new DepthOverlaySeekbarListener());
        mDepthOverlaySeekbar.setVisibility(View.GONE);

        mdebugOverlayCheckbox = (CheckBox) findViewById(R.id.debug_overlay_checkbox);
        mdebugOverlayCheckbox.setOnCheckedChangeListener(new DebugOverlayCheckboxListener());

        mGPUUpsampleCheckbox = (CheckBox) findViewById(R.id.gpu_upsample_checkbox);
        mGPUUpsampleCheckbox.setOnCheckedChangeListener(new GPUUpsampleListener());

        // OpenGL view where all of the graphics are drawn
        mGLView = (GLSurfaceView) findViewById(R.id.gl_surface_view);

        // Configure OpenGL renderer
        mGLView.setEGLContextClientVersion(2);
        mRenderer = new GLSurfaceRenderer(this);
        mGLView.setRenderer(mRenderer);

        // Check if the Tango Core is out dated.
        if (!CheckTangoCoreVersion(MIN_TANGO_CORE_VERSION)) {
            Toast.makeText(this, "Tango Core out dated, please update in Play Store", 
                    Toast.LENGTH_LONG).show();
            finish();
            return;
        }
    }

    @Override
    protected void onResume() {
        // We moved most of the onResume lifecycle calls to the surfaceCreated,
        // surfaceCreated will be called after the GLSurface is created.
        super.onResume();

        mGLView.onResume();

        int ret = JNIInterface.tangoConnect();
        if (ret != TANGO_SUCCESS) {
            mIsConnectedService = true;
            Log.e(TAG, "Failed to set connect service with code: "  + ret);
            finish();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        mGLView.onPause();
        if (mIsConnectedService) {
            JNIInterface.tangoDisconnect();
            mIsConnectedService = false;
        }
    }

    public void surfaceCreated() {
        JNIInterface.initializeGLContent();
        int ret = JNIInterface.tangoConnectTexture();
        if (ret != TANGO_SUCCESS) {
            Log.e(TAG, "Failed to connect texture with code: "  + ret);
            finish();
        }

        ret = JNIInterface.tangoConnectCallbacks();
        if (ret != TANGO_SUCCESS) {
            Log.e(TAG, "Failed to set connect cbs with code: "  + ret);
            finish();
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
