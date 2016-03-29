/*
 * Copyright 2016 Google Inc. All Rights Reserved.
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

package com.projecttango.examples.cpp.hellomotiontracking;

import android.app.Activity;

/**
 * Interfaces between C and Java. 
 *
 * Note that these are the functions that call into native code, native code is
 * responsible for the communication between the application and Tango Service.
 */
public class TangoJniNative {
    static {
        System.loadLibrary("cpp_hello_motion_tracking_example");
    }

    /**
     * Interfaces to native OnCreate function.
     * 
     * @param callerActivity the caller activity of this function.
     */
    public static native void onCreate(Activity callerActivity);

    /**
     * Interfaces to native OnResume function. 
     */ 
    public static native void onResume();

    /**
     * Interfaces to native OnPause function.
     */ 
    public static native void onPause();
}
