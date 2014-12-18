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

package com.projecttango.experiments.nativepointcloud;

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

  public void onSurfaceCreated(GL10 gl, EGLConfig config) {
    TangoJNINative.initGlContent();
  }
}
