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

package com.projecttango.examples.cpp.hellovideo;

import android.app.Activity;
import android.os.IBinder;
import android.util.Log;

import com.projecttango.examples.cpp.util.TangoInitializationHelper;

/**
 * Interfaces between native C++ code and Java code.
 * <p/>
 * Note that these are the functions that call into native code, native code is
 * responsible for the communication between the application and Tango Service.
 */
public class TangoJniNative {
    static {
        if (TangoInitializationHelper.loadTangoSharedLibrary() ==
                TangoInitializationHelper.ARCH_ERROR) {
            Log.e("TangoJniNative", "ERROR! Unable to load libtango_client_api.so!");
        }
        System.loadLibrary("hello_video");
    }

    /**
     * Interfaces to native OnCreate function.
     *
     * @param callerActivity the caller activity of this function.
     */
    public static native void onCreate(Activity callerActivity, int activityOrientation,
                                       int cameraOrientation);

    /**
     * Called when the Tango service is connected.
     *
     * @param binder The native binder object.
     */
    public static native void onTangoServiceConnected(IBinder binder);

    /**
     * Interfaces to native OnPause function.
     */
    public static native void onPause();

    /**
     * Delegate {@code GLSurfaceView.onGlSurfaceCreated} to the native code code.
     */
    public static native void onGlSurfaceCreated();

    /**
     * Delegate {@code GLSurfaceView.onGlSurfaceChanged} to the native code code.
     */
    public static native void onGlSurfaceChanged(int width, int height);

    /**
     * Delegate {@code GLSurfaceView.onGlSurfaceDrawFrame} to the native code code.
     */
    public static native void onGlSurfaceDrawFrame();

    /**
     * Select the RGB camera texture rendering method.
     *
     * @param useYuvMethod If {@code true}, YUV buffer rendering method will be used, otherwise
     *                     the Texture ID rendering method will be used instead.
     */
    public static native void setYuvMethod(boolean useYuvMethod);
}
