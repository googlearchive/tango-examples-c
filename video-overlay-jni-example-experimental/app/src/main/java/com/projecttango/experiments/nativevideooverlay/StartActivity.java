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

package com.projecttango.experiments.nativevideooverlay;

import com.google.tango.tangojnivideooverlay.R;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;
import android.widget.ToggleButton;

/**
 * Landing scene for the application.
 */
public class StartActivity extends Activity implements View.OnClickListener {
  public static final String KEY_MOTIONTRACKING_AUTO_RECOVERY = 
      "com.google.tango.tangojnimotiontracking.useautorecovery";
  public static final String EXTRA_KEY_PERMISSIONTYPE = "PERMISSIONTYPE";
  public static final String EXTRA_VALUE_MOTION_TRACKING = "MOTION_TRACKING_PERMISSION";

  public static final int REQUEST_CODE = 0;

  private Button mStartButton;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    Intent intent = new Intent();
    intent.setAction("android.intent.action.REQUEST_TANGO_PERMISSION");
    intent.putExtra(EXTRA_KEY_PERMISSIONTYPE, EXTRA_VALUE_MOTION_TRACKING);
    startActivityForResult(intent, 0);

    setTitle(R.string.app_name);
    setContentView(R.layout.activity_start);
    mStartButton = (Button) findViewById(R.id.startbutton);
    mStartButton.setOnClickListener(this);
  }

  @Override
  public void onClick(View v) {
    switch (v.getId()) {
    case R.id.startbutton:
      startVideoOverlay();
      break;
    }
  }

  private void startVideoOverlay() {
    Intent videoOverlayActivity = new Intent(this,
        VideoOverlayActivity.class);
    startActivity(videoOverlayActivity);
  }

  @Override
  protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    // Check which request we're responding to.
    if (requestCode == REQUEST_CODE) {
        // Make sure the request was successful.
        if (resultCode == RESULT_CANCELED) {
          Toast.makeText(this, 
            "Motion Tracking Permission Needed!", Toast.LENGTH_SHORT).show();
          finish();
        }
    }
  }
}
