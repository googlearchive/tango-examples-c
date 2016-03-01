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

import com.google.tango.hellotangojni.R;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.widget.Toast;

/**
 * Main activity controls Tango lifecycle.
 */
public class HelloTangoActivity extends Activity {
  // The minimum Tango Core version required from this application.
  private static final int  MIN_TANGO_CORE_VERSION = 6804;

  // A flag to check if the Tango Service is connected. This flag avoids the
  // program attempting to disconnect from the service while it is not
  // connected.This is especially important in the onPause() callback for the
  // activity class.
  private boolean mIsConnectedService = false;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    // Check that the installed version of the Tango Core is up to date.
    if (!TangoJNINative.checkTangoVersion(this, MIN_TANGO_CORE_VERSION)) {
      Toast.makeText(this, "Tango Core is out of date, please update in Play Store",
                     Toast.LENGTH_LONG).show();
      finish();
      return;
    }
  }
  
  @Override
  protected void onResume() {
    super.onResume();

    // Setup Tango configuraturation.
    TangoJNINative.setupConfig();
    TangoJNINative.connectCallbacks();

    // Connect to Tango Service (returns true on success).
    // Starts Motion Tracking and Area Learning.
    if (TangoJNINative.connect()) {
      mIsConnectedService = true;
    } else {
      // End the activity and let the user know something went wrong.
      Toast.makeText(this, "Connect Tango Service Error", Toast.LENGTH_LONG).show();
      finish();
    }
  }

  @Override
  protected void onPause() {
    // Note that this function will be called when the permission activity is
    // foregrounded.
    super.onPause();
    if (mIsConnectedService) {
      TangoJNINative.disconnect();
      mIsConnectedService = false;
    }
  }
}
