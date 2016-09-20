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

package com.projecttango.experiments.cpp.videostabilization;

import android.app.Activity;
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Point;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.WindowManager;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Switch;

import com.projecttango.examples.cpp.util.TangoInitializationHelper;

/**
 * The main activity of the application which shows debug information and a
 * glSurfaceView that renders graphic content.
 */
public class VideoStabilizationActivity extends Activity {
  // GLSurfaceView and its renderer, all of the graphic content is rendered
  // through OpenGL ES 2.0 in the native code.
  private VideoStabilizationRenderer mRenderer;
  private GLSurfaceView mGLView;
  private Switch mVideoStabilizationSwitch;
  private Switch mCameraLockSwitch;

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

    TangoJNINative.onCreate(this);

    getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                         WindowManager.LayoutParams.FLAG_FULLSCREEN);

    // Querying screen size, used for computing the normalized touch point.
    Display display = getWindowManager().getDefaultDisplay();
    display.getSize(mScreenSize);

    // Setting content view of this activity and getting the mIsAutoRecovery
    // flag from StartActivity.
    setContentView(R.layout.activity_video_stabilization);

    // OpenGL view where all of the graphics are drawn
    mGLView = (GLSurfaceView) findViewById(R.id.gl_surface_view);

    // Configure OpenGL renderer
    mGLView.setEGLContextClientVersion(2);

    // Video stabilization switch.
    mVideoStabilizationSwitch = (Switch) findViewById(R.id.video_stabilization_switch);
    mVideoStabilizationSwitch.setChecked(true);
    mVideoStabilizationSwitch.setOnCheckedChangeListener(new OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            TangoJNINative.setEnableVideoStabilization(isChecked);
        }
    });
    TangoJNINative.setEnableVideoStabilization(mVideoStabilizationSwitch.isChecked());

    mCameraLockSwitch = (Switch) findViewById(R.id.camera_lock_switch);
    mCameraLockSwitch.setChecked(false);
    mCameraLockSwitch.setOnCheckedChangeListener(new OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            TangoJNINative.setCameraLocked(isChecked);
        }
    });
    TangoJNINative.setCameraLocked(mCameraLockSwitch.isChecked());

    // Configure OpenGL renderer. The RENDERMODE_WHEN_DIRTY is set explicitly
    // for reducing the CPU load. The request render function call is triggered
    // by the onTextureAvailable callback from the Tango Service in the native
    // code.
    mRenderer = new VideoStabilizationRenderer(getAssets());
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
    TangoJNINative.destroyActivity();
  }

  // Request render on the glSurfaceView. This function is called from the
  // native code, and it is triggered from the onTextureAvailable callback from
  // the Tango Service.
  public void requestRender() {
    if (mGLView.getRenderMode() != GLSurfaceView.RENDERMODE_CONTINUOUSLY) {
      mGLView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }
    mGLView.requestRender();
  }
}
