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
package com.projecttango.augmentedrealitynative;

public class TangoJNINative {
    static {
        System.loadLibrary("augmented_reality_jni_example");
    }

    public static native int Initialize(AugmentedRealityActivity activity);

    public static native void SetupConfig(boolean isAutoRecovery);

    public static native void ConnectTexture();

    public static native int ConnectService();

    public static native void DisconnectService();

    public static native void OnDestroy();

    public static native void SetupGraphic();

    public static native void SetupViewport(int width, int height);

    public static native void Render();

    public static native void SetCamera(int camera_index);

    public static native void ResetMotionTracking();

    public static native byte UpdateStatus();

    public static native String GetPoseString();

    public static native String GetVersionNumber();

    public static native void UpdateARElement(int arElement, int interactionType);

    public static native float StartSetCameraOffset();

    public static native float SetCameraOffset(float rotX, float rotY, float zDistance);
}

