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

import android.opengl.GLSurfaceView;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Renderer renders graphic content.
 */
public class Renderer implements GLSurfaceView.Renderer {
  public void onDrawFrame(GL10 gl) {
    TangoJNINative.render();
  }

  public void onSurfaceChanged(GL10 gl, int width, int height) {
    TangoJNINative.setupGraphic(width, height);
  }

  // onSurfaceCreated() is called when the rendering thread starts.
  //
  // The ConnectTexture() function needs a texture ID which requires a GL context
  // to initialized. This surface lifecylcle tightened together with the activity 
  // lifecycle, each time we pause/resume from the application, the onSurfaceCreated()
  // will be called. 
  //
  // Couple of notes about connecting texture ID:
  // 1. Texutre ID needs to be connected before the service is connected.
  // 2. Each time the service is reconnected, the texture ID will need to be
  //    reconnected again.
  // 3. The graphic surface should be ready before connecting to texture.
  // 4. The texture and timestamp will be updated when TangoService_updateTexture() 
  //    is called. 
  // 5. Connect/update texture requires camera permission in the AndroidManifest.xml:
  //    <uses-permission android:name="android.permission.CAMERA" />
  public void onSurfaceCreated(GL10 gl, EGLConfig config) {
    TangoJNINative.initializeGlContent();
    TangoJNINative.connectTexture();
    TangoJNINative.connect();
  }
}
