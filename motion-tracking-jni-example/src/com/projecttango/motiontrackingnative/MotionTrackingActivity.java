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

package com.projecttango.motiontrackingnative;

import android.app.Activity;
import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MotionTrackingActivity extends Activity implements
		View.OnClickListener {

	private static String TAG = MotionTrackingActivity.class.getSimpleName();

	private TextView mPoseData;
	private TextView mVersion;
	private TextView mEvent;

	private Button mMotionReset;

	private boolean mIsAutoReset;
	private MotionTrackingRenderer mRenderer;
	private GLSurfaceView mGLView;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_motion_tracking);
		Intent intent = getIntent();
		mIsAutoReset = intent.getBooleanExtra(
				StartActivity.KEY_MOTIONTRACKING_AUTORESET, false);
		
		// Text views for displaying translation and rotation data
		mPoseData = (TextView) findViewById(R.id.pose_data_textview);

		// Text views for displaying most recent Tango Event
		mEvent = (TextView) findViewById(R.id.tango_event_textview);

		// Text views for the status of the pose data and Tango library versions
		mVersion = (TextView) findViewById(R.id.version_textview);

		// Buttons for selecting camera view and Set up button click listeners
		findViewById(R.id.first_person_button).setOnClickListener(this);
		findViewById(R.id.third_person_button).setOnClickListener(this);
		findViewById(R.id.top_down_button).setOnClickListener(this);

		// Button to reset motion tracking
		mMotionReset = (Button) findViewById(R.id.resetmotion);

		// OpenGL view where all of the graphics are drawn
		mGLView = (GLSurfaceView) findViewById(R.id.gl_surface_view);

		// Set up button click listeners
		mMotionReset.setOnClickListener(this);

		// Configure OpenGL renderer
		mRenderer = new MotionTrackingRenderer();
		mGLView.setRenderer(mRenderer);
		mMotionReset.setVisibility(View.GONE);

		// Initialize the Tango service
		TangoJNINative.Initialize();

		// Set up Tango configuration file with auto-reset on
		TangoJNINative.SetupConfig(mIsAutoReset);

		mVersion.setText(TangoJNINative.GetVersionNumber());

		startUIThread();
	}

	@Override
	protected void onPause() {
		super.onPause();
		TangoJNINative.UnlockConfig();
		TangoJNINative.DisconnectService();
	}

	@Override
	protected void onResume() {
		super.onPause();
		// Lock Tango configuration file
		TangoJNINative.LockConfig();

		// Connect Tango Service
		TangoJNINative.ConnectService();
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		TangoJNINative.UnlockConfig();
		TangoJNINative.OnDestroy();
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.first_person_button:
			TangoJNINative.SetCamera(0);
			break;
		case R.id.top_down_button:
			TangoJNINative.SetCamera(2);
			break;
		case R.id.third_person_button:
			TangoJNINative.SetCamera(1);
			break;
		case R.id.resetmotion:
			TangoJNINative.ResetMotionTracking();
			break;
		default:
			Log.w(TAG, "Unknown button click");
			return;
		}
	}

	private void startUIThread() {
		new Thread(new Runnable() {
			@Override
			public void run() {
				while (true) {
					try {
						Thread.sleep(10);
						final String tangoEventString = TangoJNINative
								.GetEventString();
						final String tangoPoseString = TangoJNINative
								.GetPoseString();
						runOnUiThread(new Runnable() {
							@Override
							public void run() {
								try {
									mEvent.setText(tangoEventString);
									mPoseData.setText(tangoPoseString);
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
}
