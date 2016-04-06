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

package com.projecttango.examples.cpp.videooverlay;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;
import android.widget.ToggleButton;

/**
 * Main activity shows video overlay scene.
 */
public class VideoOverlayActivity extends Activity
    implements View.OnClickListener {
  // The minimum Tango Core version required from this application.
  private static final int MIN_TANGO_CORE_VERSION = 6804;
  
  /**
   * Enum for texture pipeline method.
   */
  public enum TextureMethod {
    YUV,
    TEXTURE_ID
  }

  private boolean mIsConnected = false;

  private GLSurfaceView glView;
  private ToggleButton mYuvRenderSwitcher;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    setContentView(R.layout.activity_video_overlay);
    glView = (GLSurfaceView) findViewById(R.id.surfaceview);
    
    // Configure OpenGL renderer
    glView.setEGLContextClientVersion(2);

    glView.setRenderer(new Renderer());

    mYuvRenderSwitcher = (ToggleButton) findViewById(R.id.yuv_switcher);
    mYuvRenderSwitcher.setOnClickListener(this);

    // Check that the installed version of the Tango Core is up to date.
    if (!TangoJNINative.checkTangoVersion(this, MIN_TANGO_CORE_VERSION)) {
      Toast.makeText(this, "Tango Core out of date, please update in Play Store", 
                     Toast.LENGTH_LONG).show();
      finish();
      return;
    }
  }
  
  @Override
  protected void onResume() {
    super.onResume();
    glView.onResume();
    // Setup the configuration for the TangoService.
    TangoJNINative.setupConfig();

    // Connect to Tango Service.
    // This function will start the Tango Service pipeline, in this case,
    // it will start Motion Tracking.
    if (!mIsConnected) {
      TangoJNINative.connect();
      mIsConnected = true;
    }

    enableYuvTexture(mYuvRenderSwitcher.isChecked());
  }

  @Override
  protected void onPause() {
    super.onPause();
    glView.onPause();
    // Disconnect from Tango Service, release all the resources that the app is
    // holding from Tango Service.
    if (mIsConnected) {
      TangoJNINative.disconnect();
      mIsConnected = false;
    }
    TangoJNINative.freeBufferData();
  }

  @Override
  public void onClick(View v) {
    switch (v.getId()) {
    case R.id.yuv_switcher:
      enableYuvTexture(mYuvRenderSwitcher.isChecked());
      break;
    }
  }

  private void enableYuvTexture(boolean isEnabled) {
    if (isEnabled) {
        // Turn on YUV
        TangoJNINative.setYUVMethod();
      } else {
        TangoJNINative.setTextureMethod();
      }
  }
}
