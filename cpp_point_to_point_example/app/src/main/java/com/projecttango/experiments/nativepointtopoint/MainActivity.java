/*
 * Copyright 2016 Google Inc. All Rights Reserved.
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

package com.projecttango.experiments.nativepointtopoint;

import android.app.Activity;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.TextView;
import android.widget.Toast;

import java.util.Iterator;
import java.util.Map;

/**
 * Primary activity of the example.
 */
public class MainActivity extends Activity {
    private static final String TAG = MainActivity.class.getSimpleName();

    // The minimum Tango Core version required from this application.
    private static final int  MIN_TANGO_CORE_VERSION = 6804;

    // The package name of Tang Core, used for checking minimum Tango Core version.
    private static final String TANGO_PACKAGE_NAME = "com.projecttango.tango";

    // The interval at which we'll update our UI debug text in milliseconds.
    // This is the rate at which we query our native wrapper around the tango
    // service for pose and event information.
    private static final int UPDATE_UI_INTERVAL_MS = 10;

    // A flag to check if the Tango Service is connected. This flag avoids the
    // program attempting to disconnect from the service while it is not
    // connected.This is especially important in the onPause() callback for the
    // activity class.
    private boolean mIsServiceConnected = false;

    // GLSurfaceView and renderer, all of the graphic content is rendered
    // through OpenGL ES 2.0 in native code.
    private GLSurfaceView mGLView;
    private GLSurfaceRenderer mRenderer;
    // Current frame's pose information.
    private TextView mDistanceMeasure;

    private CheckBox mBilateralBox;
    private boolean mBilateralFiltering;

    // Handles the debug text UI update loop.
    private Handler mHandler = new Handler();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        // Check if the Tango Core is out dated.
        if (!JNIInterface.checkTangoVersion(this, MIN_TANGO_CORE_VERSION)) {
            Toast.makeText(this, "Tango Core out dated, please update in Play Store", 
                    Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        initializeTango();

        setContentView(R.layout.activity_main);

        // Text views for Tango library versions
        mDistanceMeasure = (TextView) findViewById(R.id.distance_textview);

        configureGlSurfaceView();
        configureFilteringOption();
    }

    @Override
    public boolean onTouchEvent(final MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            // Ensure that handling of the touch event is run on the GL thread
            // rather than Android UI thread. This ensures we can modify
            // rendering state without locking.  This event triggers a plane
            // fit.
            mGLView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        JNIInterface.onTouchEvent(event.getX(), event.getY());
                    }
                });
        }

        return super.onTouchEvent(event);
    }

    private void configureGlSurfaceView() {
        // OpenGL view where all of the graphics are drawn.
        mGLView = (GLSurfaceView) findViewById(R.id.gl_surface_view);

        mGLView.setEGLContextClientVersion(2);
        // Configure the OpenGL renderer.
        mRenderer = new GLSurfaceRenderer(this);
        mGLView.setRenderer(mRenderer);
    }

    private void initializeTango() {
        int status = JNIInterface.tangoInitialize(this);
        if (status != 0) {
            if (status == -2) {
                Toast.makeText(this, "Tango Service version mis-match",
                               Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "Tango Service initialize internal error",
                               Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void configureFilteringOption() {
        mBilateralBox = (CheckBox) findViewById(R.id.check_box);
        mBilateralBox.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                // Is the view now checked?
                mBilateralFiltering = ((CheckBox) view).isChecked();
                if (mBilateralFiltering) {
                    JNIInterface.setUpsampleViaBilateralFiltering(true);
                } else {
                    JNIInterface.setUpsampleViaBilateralFiltering(false);
                }
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        mGLView.onResume();

        // Setup the configuration of the Tango Service.
        int ret = JNIInterface.tangoSetupAndConnect();
        
        if (ret != 0) {
            mIsServiceConnected = false;
            Log.e(TAG, "Failed to set config and connect with code: " + ret);
            finish();
        }
        mIsServiceConnected = true;

        // Start the debug text UI update loop.
        mHandler.post(mUpdateUiLoopRunnable);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mGLView.onPause();
        JNIInterface.deleteResources();

        // Disconnect from the Tango Service, release all the resources that
        // the app is holding from the Tango Service.
        if (mIsServiceConnected) {
            JNIInterface.tangoDisconnect();
            mIsServiceConnected = false;
        }
    }

    public void surfaceCreated() {
        int ret = JNIInterface.initializeGLContent();

        if (ret != 0) {
            Log.e(TAG, "Failed to connect texture with code: " + ret);
        }
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
            mDistanceMeasure.setText(JNIInterface.getPointSeparation());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
