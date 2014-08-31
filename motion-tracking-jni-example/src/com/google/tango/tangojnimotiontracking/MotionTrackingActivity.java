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

package com.google.tango.tangojnimotiontracking;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.widget.Button;
import android.widget.TextView;

public class MotionTrackingActivity extends Activity {

	MotionTrackingView motionTrackingView;
	TextView tangoPoseStatusText;
	String[] poseStatuses = { "Initializing", "Valid", "Invalid", "Unknown" };

	Button resetButton;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		motionTrackingView = new MotionTrackingView(this);

		tangoPoseStatusText = new TextView(this);

		resetButton = new Button(this);
		//resetButton.setVisibility(View.GONE);
		setContentView(motionTrackingView);
		resetButton.setText("Reset");
		resetButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				TangoJNINative.ResetMotionTracking();
			}
		});

		addContentView(tangoPoseStatusText, new LayoutParams(
				LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
		addContentView(resetButton, new LayoutParams(LayoutParams.WRAP_CONTENT,
				LayoutParams.WRAP_CONTENT));
		
		TangoJNINative.OnCreate();

		new Thread(new Runnable() {
			@Override
			public void run() {
				while (true) {
					try {
						Thread.sleep(10);
						final byte statusIndex = TangoJNINative.UpdateStatus();
						final String tangoPoseStatusString = poseStatuses[statusIndex];
						final String tangoPoseString = TangoJNINative
								.PoseToString();
						runOnUiThread(new Runnable() {
							@Override
							public void run() {
								try {
									tangoPoseStatusText.setText("Pose Status: "
											+ tangoPoseStatusString + "\n"
											+ tangoPoseString);
								} catch (Exception e) {
									e.printStackTrace();
								}
							}
						});

					} catch (Exception e) {
						e.printStackTrace();
					}
				}
			}
		}).start();
	}

	@Override
	protected void onResume() {
		super.onResume();
		motionTrackingView.onResume();
		TangoJNINative.OnResume();
	}

	@Override
	protected void onPause() {
		super.onPause();
		motionTrackingView.onPause();
		TangoJNINative.OnPause();
	}

	protected void onDestroy() {
		super.onDestroy();
		TangoJNINative.OnDestroy();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.motion_tracking, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.action_first_camera:
			TangoJNINative.SetCamera(0);
			return true;
		case R.id.action_third_camera:
			TangoJNINative.SetCamera(1);
			return true;
		case R.id.action_top_camera:
			TangoJNINative.SetCamera(2);
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}
}
