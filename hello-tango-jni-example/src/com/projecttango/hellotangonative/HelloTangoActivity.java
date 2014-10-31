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

package com.projecttango.hellotangonative;

import com.google.tango.hellotangojni.R;

import android.app.Activity;
import android.content.Intent;
import android.widget.Toast;
import android.os.Bundle;

public class HelloTangoActivity extends Activity {
  public static final String EXTRA_KEY_PERMISSIONTYPE = "PERMISSIONTYPE";
  public static final String EXTRA_VALUE_MOTION_TRACKING = "MOTION_TRACKING_PERMISSION";


  private boolean mIsPermissionIntentCalled = false;

  TangoJNINative tangoJNINative;
  
  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
  }
  
  @Override
  protected void onResume()
  {
    super.onResume();
    if (!mIsPermissionIntentCalled) {
      Intent intent = new Intent();
      intent.setAction("android.intent.action.REQUEST_TANGO_PERMISSION");
      intent.putExtra(EXTRA_KEY_PERMISSIONTYPE, EXTRA_VALUE_MOTION_TRACKING);
      startActivityForResult(intent, 0);
    }
  }

  @Override
  protected void onPause()
  {
    super.onPause();
    TangoJNINative.Disconnect();
    mIsPermissionIntentCalled = false;
  }

  @Override
  protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    // Check which request we're responding to.
    if (requestCode == 0) {
        // Make sure the request was successful.
        if (resultCode == RESULT_CANCELED ) {
          Toast.makeText(this, 
            "Motion Tracking Permission Needed!", Toast.LENGTH_SHORT).show();
          finish();
        } else {
          TangoJNINative.Initialize(this);
          TangoJNINative.SetupConfig();
          TangoJNINative.Connect();
          mIsPermissionIntentCalled = true;
        }
    }
  }

}
