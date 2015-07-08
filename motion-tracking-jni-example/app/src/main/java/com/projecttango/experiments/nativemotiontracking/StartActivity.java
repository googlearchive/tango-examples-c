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

// This is the launcher activity which sets up the configuration for the Motion
// Tracking application.

// The only configuration we are setting up in this example is the Auto Recovery
// flag.
public class StartActivity extends Activity implements View.OnClickListener {
  // Key string for passing user's selection to next activity (MotionTracking).
  public static final String KEY_MOTIONTRACKING_AUTO_RECOVERY =
      "com.google.tango.tangojnimotiontracking.useautorecovery";

  // Toggle button for selecting Auto Recovery on/off.
  private ToggleButton mAutoRecoveryButton;

  // Start button launchs the MotionTrackingActivity. 
  private Button mStartButton;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    // Configuring the layouts.
    setTitle(R.string.app_name);
    setContentView(R.layout.activity_start);
    mAutoRecoveryButton = (ToggleButton) findViewById(R.id.autorecoverybutton);
    mStartButton = (Button) findViewById(R.id.startbutton);
    mAutoRecoveryButton.setOnClickListener(this);
    mStartButton.setOnClickListener(this);
  }

  @Override
  public void onClick(View v) {
    // Handle button clicks.
    switch (v.getId()) {
    case R.id.startbutton:
      startMotionTracking();
      break;
    }
  }

  private void startMotionTracking() {
    // Save user's selection and launch the MotionTrackingActivity.
    Intent startmotiontracking = new Intent(this, MotionTrackingActivity.class);
    startmotiontracking.putExtra(KEY_MOTIONTRACKING_AUTO_RECOVERY,
        mAutoRecoveryButton.isChecked());
    startActivity(startmotiontracking);
  }
}
