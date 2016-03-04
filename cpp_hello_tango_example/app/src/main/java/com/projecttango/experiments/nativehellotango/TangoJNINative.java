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

package com.projecttango.experiments.nativehellotango;

/**
 * Interfaces between C and Java. 
 *
 * Note that these are the functions that call into native code, native code is
 * responsible for the communication between the application and Tango Service.
 */
public class TangoJNINative {
  static {
    System.loadLibrary("cpp_hello_tango_example");
  }

  // Check that the installed version of the Tango API is up to date.
  public static native boolean checkTangoVersion(HelloTangoActivity activity, int minTangoVersion);

  // Setup the configuration file of Tango Service.
  public static native int setupConfig();

  // Connect the onPoseAvailable callback.
  public static native int connectCallbacks();

  // Connect to Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start Motion Tracking.
  public static native boolean connect();

  // Disconnect from Tango Service, release all the resources that the app is
  // holding from Tango Service.
  public static native void disconnect();
}
