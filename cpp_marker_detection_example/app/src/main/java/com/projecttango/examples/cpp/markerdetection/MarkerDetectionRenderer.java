/*
 * Copyright 2017 Google Inc. All Rights Reserved.
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

package com.projecttango.examples.cpp.markerdetection;

import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * MarkerDetectionRenderer renders graphic content. This includes the ground grid,
 * camera frustum, camera axis, and trajectory based on the Tango device's pose.
 */
public class MarkerDetectionRenderer implements GLSurfaceView.Renderer {

  private AssetManager mAssetManager;

  public MarkerDetectionRenderer(AssetManager assetManager){
    mAssetManager = assetManager;
  }

  // Render loop of the Gl context.
  public void onDrawFrame(GL10 gl) {
    TangoJNINative.onGlSurfaceDrawFrame();
  }

  // Called when the surface size changes.
  public void onSurfaceChanged(GL10 gl, int width, int height) {
    TangoJNINative.onGlSurfaceChanged(width, height);
  }

  // Called when the surface is created or recreated.
  public void onSurfaceCreated(GL10 gl, EGLConfig config) {
    TangoJNINative.onGlSurfaceCreated(mAssetManager);
  }
}
