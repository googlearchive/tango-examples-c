/*
 * Copyright 2015 Google Inc. All Rights Reserved.
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

package com.projecttango.experiments.nativeplanefitting;

import android.app.Activity;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.SharedPreferences;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.Toast;

import java.util.Iterator;
import java.util.Map;

// Primary activity of the example.
public class MainActivity extends Activity {
    // The input argument is invalid.
    private static final int TANGO_INVALID = -2;

    // This error code denotes some sort of hard error occurred.
    private static final int TANGO_ERROR = -1;

    // This code indicates success.
    private static final int TANGO_SUCCESS = 0;

    private static final String TAG = MainActivity.class.getSimpleName();

    // The minimum Tango Core version required from this application.
    private static final int  MIN_TANGO_CORE_VERSION = 6804;

    // The package name of Tang Core, used for checking minimum Tango Core version.
    private static final String TANGO_PACKAGE_NAME = "com.projecttango.tango";

    // A flag to check if the Tango Service is connected. This flag avoids the
    // program attempting to disconnect from the service while it is not
    // connected.This is especially important in the onPause() callback for the
    // activity class.
    private boolean mIsServiceConnected = false;

    // GLSurfaceView and renderer, all of the graphic content is rendered
    // through OpenGL ES 2.0 in native code.
    private GLSurfaceView mGLView;
    private GLSurfaceRenderer mRenderer;

    private CustomDrawerLayout mDrawerLayout;
    private ImageButton mDrawerButton;
    private Button mSettingsButton;

    private SharedPreferences mPreferences;

    // This is a callback to listen if there are any changes in the settings
    // menu of the app and update accordingly.
    private SharedPreferences.OnSharedPreferenceChangeListener listener =
            new SharedPreferences.OnSharedPreferenceChangeListener() {
                public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
                    updatePreferences(prefs, key);
                }
            };

    // Update settings of the app depending on the settings last saved.
    private void updatePreferences(SharedPreferences prefs, String key) {
        if (key.equals(getString(R.string.key_debug_point_cloud))) {
            JNIInterface.setRenderDebugPointCloud(prefs.getBoolean(key, false));
        } else {
            Log.w(TAG, "Unknown preference: " + key);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        // Check if the Tango Core is out dated.
        if (!CheckTangoCoreVersion(MIN_TANGO_CORE_VERSION)) {
            Toast.makeText(this, "Tango Core out dated, please update in Play Store", 
                    Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        initializeTango();

        setContentView(R.layout.activity_main);

        configureGlSurfaceView();

        mDrawerLayout = (CustomDrawerLayout) findViewById(R.id.drawer_layout);
        configureSettingsButton();
        configureDrawerButton();
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
        if (status != TANGO_SUCCESS) {
            if (status == TANGO_INVALID) {
                Toast.makeText(this, "Tango Service version mis-match",
                               Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "Tango Service initialize internal error",
                               Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void configureDrawerButton() {
        mDrawerButton = (ImageButton) findViewById(R.id.drawer_button);
        mDrawerButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                mDrawerLayout.openDrawer(Gravity.START);
            }
        });

        PreferenceManager.setDefaultValues(this, R.xml.settings, false);
        mPreferences = PreferenceManager.getDefaultSharedPreferences(this);
        mPreferences.registerOnSharedPreferenceChangeListener(listener);
    }

    private void configureSettingsButton() {
        mSettingsButton = (Button) findViewById(R.id.button_settings);
        mSettingsButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                SettingsFragment settingsFrag = new SettingsFragment();
                FragmentManager manager = getFragmentManager();
                FragmentTransaction transaction = manager.beginTransaction();
                transaction.replace(R.id.drawer_layout, settingsFrag);
                transaction.addToBackStack(null);
                transaction.commit();
                if (mDrawerLayout.isDrawerOpen(Gravity.START)) {
                    mDrawerLayout.closeDrawer(Gravity.START);
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
        mIsServiceConnected = true;

        if (ret != TANGO_SUCCESS) {
            mIsServiceConnected = false;
            Log.e(TAG, "Failed to set config and connect with code: " + ret);
            finish();
        }
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

        if (ret != TANGO_SUCCESS) {
            Log.e(TAG, "Failed to connect texture with code: " + ret);
        }

        // Update the last saved settings after the surface is created.
        Map<String, ?> allKeys = mPreferences.getAll();
        Iterator i = allKeys.entrySet().iterator();

        while (i.hasNext()) {
            Map.Entry pair = (Map.Entry) i.next();
            updatePreferences(mPreferences, (String) pair.getKey());
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
