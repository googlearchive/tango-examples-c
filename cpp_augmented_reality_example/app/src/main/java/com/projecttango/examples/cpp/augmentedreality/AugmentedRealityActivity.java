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

package com.projecttango.examples.cpp.augmentedreality;

import android.app.Activity;
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.graphics.Point;
import android.hardware.Camera;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.IBinder;
import android.view.Display;
import android.view.WindowManager;

import com.projecttango.examples.cpp.util.TangoInitializationHelper;

/**
 * The main activity of the application which shows debug information and a
 * glSurfaceView that renders graphic content.
 */
public class AugmentedRealityActivity extends Activity {
  // GLSurfaceView and its renderer, all of the graphic content is rendered
  // through OpenGL ES 2.0 in the native code.
  private AugmentedRealityRenderer mRenderer;
  private GLSurfaceView mGLView;

  // Screen size for normalizing the touch input for orbiting the render camera.
  private Point mScreenSize = new Point();

  // Tango Service connection.
  ServiceConnection mTangoServiceConnection = new ServiceConnection() {
      @Override
      public void onServiceConnected(ComponentName name, IBinder service) {
        // The following code block does setup and connection to Tango.
        TangoJNINative.onTangoServiceConnected(service);
      }

    @Override
    public void onServiceDisconnected(ComponentName name) {
        // Handle this if you need to gracefully shutdown/retry
        // in the event that Tango itself crashes/gets upgraded while running.
      }
    };

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    WindowManager windowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
    Display display = windowManager.getDefaultDisplay();
    display.getSize(mScreenSize);

    TangoJNINative.onCreate(this, display.getRotation());

    getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                         WindowManager.LayoutParams.FLAG_FULLSCREEN);

    // Setting content view of this activity and getting the mIsAutoRecovery
    // flag from StartActivity.
    setContentView(R.layout.activity_augmented_reality);

    // OpenGL view where all of the graphics are drawn
    mGLView = (GLSurfaceView) findViewById(R.id.gl_surface_view);

    // Configure OpenGL renderer
    mGLView.setEGLContextClientVersion(2);

    // Configure OpenGL renderer. The RENDERMODE_WHEN_DIRTY is set explicitly
    // for reducing the CPU load. The request render function call is triggered
    // by the onTextureAvailable callback from the Tango Service in the native
    // code.
    mRenderer = new AugmentedRealityRenderer(getAssets());
    mGLView.setRenderer(mRenderer);
  }

  @Override
  protected void onResume() {
    super.onResume();
    mGLView.onResume();

    mGLView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

    TangoInitializationHelper.bindTangoService(this, mTangoServiceConnection);
  }

  @Override
  protected void onPause() {
    super.onPause();
    mGLView.onPause();

    TangoJNINative.onPause();
    unbindService(mTangoServiceConnection);   
  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
    TangoJNINative.onDestroy();
  }

  @Override
  public void onConfigurationChanged(Configuration newConfig) {
    super.onConfigurationChanged(newConfig);

    WindowManager windowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
    Display display = windowManager.getDefaultDisplay();
    display.getSize(mScreenSize);

    TangoJNINative.onConfigurationChanged(display.getRotation());
  }

  // Request onGlSurfaceDrawFrame on the glSurfaceView. This function is called from the
  // native code, and it is triggered from the onTextureAvailable callback from
  // the Tango Service.
  public void requestRender() {
    if (mGLView.getRenderMode() != GLSurfaceView.RENDERMODE_CONTINUOUSLY) {
      mGLView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }
    mGLView.requestRender();
  }
}
