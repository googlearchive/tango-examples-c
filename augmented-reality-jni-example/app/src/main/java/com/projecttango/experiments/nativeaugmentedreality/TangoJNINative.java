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
package com.projecttango.experiments.nativeaugmentedreality;

/**
 * Interfaces between C and Java.
 */
public class TangoJNINative {
    static {
        System.loadLibrary("augmented_reality_jni_example");
    }

    public static native int initialize(AugmentedRealityActivity activity);

    public static native void setupConfig(boolean isAutoRecovery);

    public static native void connectTexture();

    public static native int connectService();

    public static native void disconnectService();

    public static native void onDestroy();

    public static native void setupGraphic();

    public static native void setupViewport(int width, int height);

    public static native void render();

    public static native void setCamera(int cameraIndex);

    public static native void resetMotionTracking();

    public static native byte updateStatus();

    public static native String getPoseString();

    public static native String getVersionNumber();
    
    public static native boolean getIsLocalized();

    public static native void updateARElement(int arElement, int interactionType);

    public static native float startSetCameraOffset();

    public static native float setCameraOffset(float rotX, float rotY, float zDistance);

    public static native void placeObject();
}

