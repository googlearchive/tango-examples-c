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

package com.projecttango.experiments.nativemotiontracking;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;
import android.widget.ToggleButton;

/**
 * This activity set up the configuration for Motion Tracking application.
 */
public class StartActivity extends Activity implements View.OnClickListener {
  public static final String KEY_MOTIONTRACKING_AUTO_RECOVERY = 
    "com.google.tango.tangojnimotiontracking.useautorecovery";
  public static final String EXTRA_KEY_PERMISSIONTYPE = "PERMISSIONTYPE";
  public static final String EXTRA_VALUE_MOTION_TRACKING = 
    "MOTION_TRACKING_PERMISSION";

  private ToggleButton mAutoRecoveryButton;
  private Button mStartButton;
  private boolean mUseAutoRecovery;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    Intent intent = new Intent();
    intent.setAction("android.intent.action.REQUEST_TANGO_PERMISSION");
    intent.putExtra(EXTRA_KEY_PERMISSIONTYPE, EXTRA_VALUE_MOTION_TRACKING);
    startActivityForResult(intent, 0);

    setTitle(R.string.app_name);
    setContentView(R.layout.activity_start);
    mAutoRecoveryButton = (ToggleButton) findViewById(R.id.autorecoverybutton);
    mStartButton = (Button) findViewById(R.id.startbutton);
    mAutoRecoveryButton.setOnClickListener(this);
    mStartButton.setOnClickListener(this);
    mUseAutoRecovery = mAutoRecoveryButton.isChecked();
  }

  @Override
  public void onClick(View v) {
    switch (v.getId()) {
    case R.id.startbutton:
      startMotionTracking();
      break;
    case R.id.autorecoverybutton:
      mUseAutoRecovery = mAutoRecoveryButton.isChecked();
      break;
    }
  }

  private void startMotionTracking() {
    Intent startmotiontracking = new Intent(this,
        MotionTrackingActivity.class);
    startmotiontracking.putExtra(KEY_MOTIONTRACKING_AUTO_RECOVERY,
        mUseAutoRecovery);
    startActivity(startmotiontracking);
  }

  @Override
  protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    // Check which request we're responding to
    if (requestCode == 0) {
        // Make sure the request was successful
        if (resultCode == RESULT_CANCELED) {
          Toast.makeText(this, 
            "Motion Tracking Permission Needed!", Toast.LENGTH_SHORT).show();
          finish();
        }
    }
  }
}
