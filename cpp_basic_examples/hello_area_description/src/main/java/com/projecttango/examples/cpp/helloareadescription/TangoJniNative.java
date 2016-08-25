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

package com.projecttango.examples.cpp.helloareadescription;

import android.app.Activity;
import android.os.IBinder;
import android.util.Log;

import com.projecttango.examples.cpp.util.TangoInitializationHelper;

/**
 * Interfaces between native C++ code and Java code.
 */
public class TangoJniNative {
  static {
    // This project depends on tango_client_api, so we need to make sure we load
    // the correct library first.
    if (TangoInitializationHelper.loadTangoSharedLibrary() ==
        TangoInitializationHelper.ARCH_ERROR) {
      Log.e("TangoJNINative", "ERROR! Unable to load libtango_client_api.so!");
    }
    System.loadLibrary("hello_area_description");
  }

  /**
   * Interfaces to native OnCreate function.
   *
   * @param callerActivity the caller activity of this function.
   */
  public static native void onCreate(Activity callerActivity);

  /*
   * Called when the Tango service is connected.
   *
   * @param binder The native binder object.
   */
  public static native void onTangoServiceConnected(IBinder binder, boolean isLearningMode,
                                                    boolean isLoadingAreaDescription);

  /**
   * Interfaces to native OnPause function.
   */
  public static native void onPause();

  /**
   * Signal that the activity has been destroyed and remove any references.
   */
  public static native void onDestroy();

  /**
   * Return true if Tango has relocalized to the current ADF at least once.
   */
  public static native boolean isRelocalized();

  /**
   * Get the loaded ADF's UUID.
   */
  public static native String getLoadedAdfUuidString();

  /**
   * Save ADF in learning mode.
   *
   * @return The ADF UUID.
   */
  public static native String saveAdf();

  /**
   * Query metadata from an exsiting ADF using the key.
   */
  public static native String getAdfMetadataValue(String uuid, String key);

  /**
   * Assign a key value of a specific ADF's metadata.
   */
  public static native void setAdfMetadataValue(String uuid, String key, String value);

  /**
   * Query all ADF file's UUID, the string includes all UUIDs saperated by comma.
   */
  public static native String getAllAdfUuids();

  /**
   * Delete a ADF from Tango space.
   */
  public static native void deleteAdf(String uuid);
}
