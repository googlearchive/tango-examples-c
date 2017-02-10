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

package com.projecttango.examples.cpp.pointtopoint;

import android.app.Activity;
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.hardware.Camera;
import android.hardware.display.DisplayManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.CheckBox;
import android.widget.TextView;

import com.projecttango.examples.cpp.util.TangoInitializationHelper;

/**
 * Primary activity of the example.
 */
public class MainActivity extends Activity {
  private static final String TAG = MainActivity.class.getSimpleName();

  // The interval at which we'll update our UI debug text in milliseconds.
  // This is the rate at which we query our native wrapper around the tango
  // service for pose and event information.
  private static final int UPDATE_UI_INTERVAL_MS = 10;

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

  // Tango Service connection.
  ServiceConnection mTangoServiceConnection = new ServiceConnection() {
      public void onServiceConnected(ComponentName name, IBinder service) {
        TangoJNINative.onTangoServiceConnected(service);
        setDisplayRotation();
      }

      public void onServiceDisconnected(ComponentName name) {
        // Handle this if you need to gracefully shutdown/retry
        // in the event that Tango itself crashes/gets upgraded while running.
      }
    };

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

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
            setDisplayRotation();
          }
        }

        @Override
        public void onDisplayRemoved(int displayId) {}
      }, null);
    }

    TangoJNINative.onCreate(this);

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
      // rendering state without locking.  This event triggers a point
      // placement.
      mGLView.queueEvent(new Runnable() {
          @Override
          public void run() {
            TangoJNINative.onTouchEvent(event.getX(), event.getY());
          }
        });
    }

    return super.onTouchEvent(event);
  }

  // Pass device's camera sensor rotation and display rotation to native layer.
  // These two parameter are important for Tango to render video overlay and
  // virtual objects in the correct device orientation.
  private void setDisplayRotation() {
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

  private void configureFilteringOption() {
    mBilateralBox = (CheckBox) findViewById(R.id.check_box);
    mBilateralBox.setOnClickListener(new OnClickListener() {
        @Override
        public void onClick(View view) {
          // Is the view now checked?
          mBilateralFiltering = ((CheckBox) view).isChecked();
          if (mBilateralFiltering) {
            TangoJNINative.setUpsampleViaBilateralFiltering(true);
          } else {
            TangoJNINative.setUpsampleViaBilateralFiltering(false);
          }
        }
      });
  }

  @Override
  protected void onResume() {
    super.onResume();
    mGLView.onResume();

    // Start the debug text UI update loop.
    mHandler.post(mUpdateUiLoopRunnable);
    
    TangoInitializationHelper.bindTangoService(this, mTangoServiceConnection); 
  }

  @Override
  protected void onPause() {
    super.onPause();
    mGLView.onPause();
    TangoJNINative.onPause();
    unbindService(mTangoServiceConnection);
    mHandler.removeCallbacksAndMessages(null);
  }

  public void surfaceCreated() {
    TangoJNINative.onGlSurfaceCreated();
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
      mDistanceMeasure.setText(TangoJNINative.getPointSeparation());
    } catch (Exception e) {
      e.printStackTrace();
      Log.e(TAG, "Exception updating UI elements");
    }
  }
}
