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
    private static final String TAG = MainActivity.class.getSimpleName();

    // The minimum Tango Core version required from this application.
    private static final int MIN_TANGO_CORE_VERSION = 6804;

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

        // Check that the installed version of the Tango Core is up to date.
        if (!JNIInterface.initialize(this, MIN_TANGO_CORE_VERSION)) {
            Toast.makeText(this, "Tango Core out of date, please update in Play Store", 
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

        if (!JNIInterface.tangoSetupConfig()) {
            Log.e(TAG, "Failed to set config.");
            finish();
        }

        if (!JNIInterface.tangoConnectCallbacks()) {
            Log.e(TAG, "Failed to set connect callbacks.");
            finish();
        }

        if (!JNIInterface.tangoConnect()) {
            Log.e(TAG, "Failed to set connect service.");
            finish();
        }
        mIsConnectedService = true;

        if (!JNIInterface.tangoSetIntrinsicsAndExtrinsics()) {
            Log.e(TAG, "Failed to set extrinsics and intrinsics.");
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
        if (!JNIInterface.tangoConnectTexture()) {
            Log.e(TAG, "Failed to connect texture.");
            finish();
        }
    }
}
