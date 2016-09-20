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

package com.projecttango.examples.cpp.meshbuilder;

import android.app.Activity;
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.IBinder;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.projecttango.examples.cpp.util.TangoInitializationHelper;

/**
 * The main activity of the application which shows debug information
 * and a glSurfaceView that renders graphic content.
 */
public class MeshBuilderActivity extends Activity implements View.OnClickListener {
  // The minimum Tango Core version required from this application.
  private static final int MIN_TANGO_CORE_VERSION = 9377;

  // The package name of Tang Core, used for checking minimum Tango Core version.
  private static final String TANGO_PACKAGE_NAME = "com.projecttango.tango";

  // Tag for debug logging.
  private static final String TAG = MeshBuilderActivity.class.getSimpleName();

  // GLSurfaceView and its renderer, all of the graphic content is rendered
  // through OpenGL ES 2.0 in the native code.
  private MeshBuilderRenderer mRenderer;
  private GLSurfaceView mGLView;

  private Button mClearButton;
  private Button mToggleButton;

  private boolean m3drRunning = true;

  // Tango Service connection.
  ServiceConnection mTangoServiceConnection = new ServiceConnection() {
      public void onServiceConnected(ComponentName name, IBinder service) {
        // Synchronization around MeshBuilderActivity object is to avoid
        // Tango disconnect in the middle of the connecting operation.
        TangoJNINative.onTangoServiceConnected(service);
      }

      public void onServiceDisconnected(ComponentName name) {
        // Handle this if you need to gracefully shutdown/retry
        // in the event that Tango itself crashes/gets upgraded while running.
      }
    };

  public MeshBuilderActivity() {
    TangoJNINative.activityCtor(m3drRunning);
  }

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    setContentView(R.layout.activity_mesh_builder);

    // Setup UI elements and listeners.
    mClearButton = (Button) findViewById(R.id.clear_button);
    mClearButton.setOnClickListener(this);

    mToggleButton = (Button) findViewById(R.id.toggle_button);
    mToggleButton.setOnClickListener(this);

    // OpenGL view where all of the graphics are drawn
    mGLView = (GLSurfaceView) findViewById(R.id.gl_surface_view);

    // Configure OpenGL renderer
    mGLView.setEGLContextClientVersion(2);

    // Configure OpenGL renderer
    mRenderer = new MeshBuilderRenderer();
    mGLView.setRenderer(mRenderer);

    refreshUi();
    TangoJNINative.onCreate(this);
  }

  @Override
  public void onClick(View v) {
    switch (v.getId()) {
    case R.id.toggle_button:
      m3drRunning = !m3drRunning;
      mGLView.queueEvent(new Runnable() {
          @Override
          public void run() {
            TangoJNINative.onToggleButtonClicked(m3drRunning);
          }
        });
      break;
    case R.id.clear_button:
      mGLView.queueEvent(new Runnable() {
          @Override
          public void run() {
            TangoJNINative.onClearButtonClicked();
          }
        });
      break;
    }

    refreshUi();
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

  private void refreshUi() {
    int textId = m3drRunning ? R.string.pause : R.string.resume;
    mToggleButton.setText(textId);
  }
}
