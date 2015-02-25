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

package com.projecttango.experiments.nativevideooverlay;

import com.google.tango.tangojnivideooverlay.R;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.widget.Toast;

/**
 * Main activity shows video overlay scene.
 */
public class VideoOverlayActivity extends Activity {
  GLSurfaceView glView;

  private static final int TANGO_INVALID = -2;
  private static final int TANGO_ERROR = -1;
  private static final int TANGO_SUCCESS = 0;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    // Initialize the Tango service
    int err = TangoJNINative.initialize(this);
    if (err != TANGO_SUCCESS) {
      if (err == TANGO_INVALID) {
        Toast.makeText(this, 
          "Tango Service version mis-match", Toast.LENGTH_SHORT).show();
      } else {
        Toast.makeText(this, 
          "Tango Service initialize internal error", Toast.LENGTH_SHORT).show();
      }
    }
    setContentView(R.layout.activity_video_overlay);
    glView = (GLSurfaceView) findViewById(R.id.surfaceview);
    glView.setRenderer(new Renderer());
  }
  
  @Override
  protected void onResume() {
    super.onResume();
    glView.onResume();
    TangoJNINative.setupConfig();
  }

  @Override
  protected void onPause() {
    super.onPause();
    glView.onPause();
    TangoJNINative.disconnect();
    TangoJNINative.freeGLContent();
  }

  protected void onDestroy() {
    super.onDestroy();
  }
}
