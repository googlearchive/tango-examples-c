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

package com.projecttango.examples.cpp.planefitting;

import android.app.Activity;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.hardware.Camera;
import android.hardware.display.DisplayManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Display;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageButton;

import java.util.Iterator;
import java.util.Map;

import com.projecttango.examples.cpp.util.TangoInitializationHelper;

/**
 * Primary activity of the example.
 */
public class MainActivity extends Activity {
  private static final String TAG = MainActivity.class.getSimpleName();

  // GLSurfaceView and renderer, all of the graphic content is rendered
  // through OpenGL ES 2.0 in native code.
  private GLSurfaceView mGLView;
  private GLSurfaceRenderer mRenderer;

  private CustomDrawerLayout mDrawerLayout;
  private ImageButton mDrawerButton;
  private Button mSettingsButton;

  private SharedPreferences mPreferences;

  // Tango Service connection.
  ServiceConnection mTangoServiceConnection = new ServiceConnection() {
      public void onServiceConnected(ComponentName name, IBinder service) {
        TangoJNINative.onTangoServiceConnected(service);
        setAndroidOrientation();
      }

      public void onServiceDisconnected(ComponentName name) {
        // Handle this if you need to gracefully shutdown/retry
        // in the event that Tango itself crashes/gets upgraded while running.
      }
    };

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
      TangoJNINative.setRenderDebugPointCloud(prefs.getBoolean(key, false));
    } else {
      Log.w(TAG, "Unknown preference: " + key);
    }
  }

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    TangoJNINative.onCreate(this);

    getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                         WindowManager.LayoutParams.FLAG_FULLSCREEN);

    // Register for display orientation change updates.
    DisplayManager displayManager = (DisplayManager) getSystemService(DISPLAY_SERVICE);
    if (displayManager != null) {
      displayManager.registerDisplayListener(new DisplayManager.DisplayListener() {
        @Override
        public void onDisplayAdded(int displayId) {}

        @Override
        public void onDisplayChanged(int displayId) {
          synchronized (this) {
            setAndroidOrientation();
          }
        }

        @Override
        public void onDisplayRemoved(int displayId) {}
      }, null);
    }

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
            TangoJNINative.onTouchEvent(event.getX(), event.getY());
          }
        });
    }

    return super.onTouchEvent(event);
  }

  // Pass device's camera sensor rotation to native layer.
  // This parameter is important for Tango to render video overlay and
  // virtual objects in the correct device orientation.
  private void setAndroidOrientation() {
    Display display = getWindowManager().getDefaultDisplay();

    TangoJNINative.onDisplayChanged(display.getRotation());
  }

  private void configureGlSurfaceView() {
    // OpenGL view where all of the graphics are drawn.
    mGLView = (GLSurfaceView) findViewById(R.id.gl_surface_view);

    mGLView.setEGLContextClientVersion(2);
    // Configure the OpenGL renderer.
    mRenderer = new GLSurfaceRenderer(this);
    mGLView.setRenderer(mRenderer);
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

    TangoInitializationHelper.bindTangoService(this, mTangoServiceConnection);
  }

  @Override
  protected void onPause() {
    super.onPause();
    mGLView.onPause();
    TangoJNINative.onPause();
    unbindService(mTangoServiceConnection);
  }

  public void surfaceCreated() {
    TangoJNINative.onGlSurfaceCreated();
    // Update the last saved settings after the surface is created.
    Map<String, ?> allKeys = mPreferences.getAll();
    Iterator i = allKeys.entrySet().iterator();

    while (i.hasNext()) {
      Map.Entry pair = (Map.Entry) i.next();
      updatePreferences(mPreferences, (String) pair.getKey());
    }
  }
}
