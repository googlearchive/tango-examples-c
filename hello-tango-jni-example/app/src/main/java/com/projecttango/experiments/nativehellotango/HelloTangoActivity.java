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
import android.os.Bundle;
import android.widget.Toast;

/**
 * Main activity controls Tango lifecycle.
 */
public class HelloTangoActivity extends Activity {
  // The user has not given permission to use Motion Tracking functionality.
  private static final int TANGO_NO_MOTION_TRACKING_PERMISSION = -3;

  // The input argument is invalid.
  private static final int  TANGO_INVALID = -2;

  // This error code denotes some sort of hard error occurred.
  private static final int  TANGO_ERROR = -1;

  // This code indicates success.
  private static final int  TANGO_SUCCESS = 0;

  // The unique request code for permission intent.
  private static final int PERMISSION_REQUEST_CODE = 0;

  // The flag to check if Tango Service is connect.
  private boolean mIsTangoServiceConnected = false;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    setTitle(R.string.app_name);

    // Initialize Tango service.
    // The activity object is used for TangoService to check if the API version
    // is too old for current TangoService.
    int status = TangoJNINative.initialize(this);
    if (status != TANGO_SUCCESS) {
      if (status == TANGO_INVALID) {
        Toast.makeText(this, 
          "Tango Service version mis-match", Toast.LENGTH_SHORT).show();
      } else {
        Toast.makeText(this, 
          "Tango Service initialize internal error", Toast.LENGTH_SHORT).show();
      }
    }
  }
  
  @Override
  protected void onResume() {
    super.onResume();

    // Setup Tango configuraturation.
    TangoJNINative.setupConfig();

    // connectCallbacks() returns TANGO_NO_MOTION_TRACKING_PERMISSION error code
    // if the application doesn't have permissions to use Motion Tracking.
    //
    // Permission intent will be called if there is no permission. The intent is
    // used to invoke permission activity.
    //
    // If there is a permission to use Motion Tracking features, the application
    // will connect to TangoService.
    int status = 0;
    status = TangoJNINative.connectCallbacks();
    if (status == TANGO_NO_MOTION_TRACKING_PERMISSION) {
      callPermissionIntent();
    } else if (status == TANGO_SUCCESS) {
      TangoJNINative.connect();
      mIsTangoServiceConnected = true;
    }
  }

  @Override
  protected void onPause() {
    // Note that this function will be called when the permission activity is
    // foregrounded.
    super.onPause();
    if (mIsTangoServiceConnected) {
      TangoJNINative.disconnect();
    }
  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
  }

  private void callPermissionIntent() {
    // Start permission activity.
    //
    // All permission types can be found from:
    // https://developers.google.com/project-tango/apis/c/c-user-permissions
    Intent intent = new Intent();
    intent.setAction("android.intent.action.REQUEST_TANGO_PERMISSION");
    intent.putExtra("PERMISSIONTYPE", "MOTION_TRACKING_PERMISSION");
    startActivityForResult(intent, PERMISSION_REQUEST_CODE);
  }

  @Override
  protected void onActivityResult (int requestCode,
                                   int resultCode,
                                   Intent data) {
    // Check if this the request we sent for permission activity.
    //
    // Note that the onResume() will be called after permission activity
    // is dismissed, because the current activity (application) is foregrounded.
    if (requestCode == PERMISSION_REQUEST_CODE) {
      if (resultCode == RESULT_CANCELED) {
        mIsTangoServiceConnected = false;
        finish();
      }
    }
  }

}
