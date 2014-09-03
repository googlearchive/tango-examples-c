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
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.ToggleButton;

public class MotionTrackingActivity extends Activity {
	RelativeLayout layout;

	GLSurfaceView motionTrackingView;
	TextView tangoPoseStatusText;

	TextView isAutoResetText;
	Button startButton;
	Button resetButton;
	ToggleButton isAutoResetButton;

	String[] poseStatuses = { "Initializing", "Valid", "Invalid", "Unknown" };
	boolean isAutoReset = false;
	boolean isStarted = false;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_motion_tracking);

		motionTrackingView = (GLSurfaceView) findViewById(R.id.surfaceview);
		motionTrackingView.setRenderer(new MotionTrackingView());
		motionTrackingView.setVisibility(View.GONE);

		isAutoResetText = (TextView) findViewById(R.id.auto_reset_text);

		tangoPoseStatusText = (TextView) findViewById(R.id.debug_info);
		tangoPoseStatusText.setVisibility(View.GONE);

		resetButton = (Button) findViewById(R.id.reset);
		isAutoResetButton = (ToggleButton) findViewById(R.id.auto_reset);
		isAutoResetButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				isAutoReset = isAutoResetButton.isChecked();
			}
		});

		resetButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				TangoJNINative.ResetMotionTracking();
			}
		});
		resetButton.setVisibility(View.GONE);

		startButton = (Button) findViewById(R.id.start);
		startButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				startButton.setVisibility(View.GONE);
				isAutoResetText.setVisibility(View.GONE);
				isAutoResetButton.setVisibility(View.GONE);

				tangoPoseStatusText.setVisibility(View.VISIBLE);
				motionTrackingView.setVisibility(View.VISIBLE);
				if (!isAutoReset) {
					resetButton.setVisibility(View.VISIBLE);
				}
				System.out.println(isAutoReset);
				TangoJNINative.Initialize(isAutoReset);
				TangoJNINative.ConnectService();
				isStarted = true;
			}
		});

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
	}

	@Override
	protected void onPause() {
		super.onPause();
		TangoJNINative.DisconnectService();
	}

	protected void onDestroy() {
		super.onDestroy();
		TangoJNINative.OnDestroy();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
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
