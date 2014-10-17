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
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Point;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class MotionTrackingActivity extends Activity implements
		View.OnClickListener {
	final int kUpdatIntervalMs = 100;

	private static String TAG = MotionTrackingActivity.class.getSimpleName();

	private TextView mPoseData;
	private TextView mVersion;
	private TextView mAppVersion;
	private TextView mEvent;

	private Button mMotionReset;

	private boolean mIsAutoRecovery;
	private MotionTrackingRenderer mRenderer;
	private GLSurfaceView mGLView;
	
	private float[] touchStartPos = new float[2];
	private float[] touchCurPos = new float[2];
	private float touchStartDist = 0.0f;
	private float touchCurDist = 0.0f;
	private Point screenSize = new Point();
	private float screenDiagnal = 0.0f;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		Display display = getWindowManager().getDefaultDisplay();
		display.getSize(screenSize);
		screenDiagnal = (float) Math.sqrt(screenSize.x * screenSize.x
				+ screenSize.y * screenSize.y);
		
		setContentView(R.layout.activity_motion_tracking);
		Intent intent = getIntent();
		mIsAutoRecovery = intent.getBooleanExtra(
				StartActivity.KEY_MOTIONTRACKING_AUTO_RECOVERY, false);
		
		// Text views for displaying translation and rotation data
		mPoseData = (TextView) findViewById(R.id.pose_data_textview);

		// Text views for displaying most recent Tango Event
		mEvent = (TextView) findViewById(R.id.tango_event_textview);

		// Text views for Tango library versions
		mVersion = (TextView) findViewById(R.id.version_textview);

		// Text views for application versions.
		mAppVersion = (TextView) findViewById(R.id.appversion);
		PackageInfo pInfo;
		try {
			pInfo = this.getPackageManager().getPackageInfo(this.getPackageName(), 0);
			mAppVersion.setText(pInfo.versionName);
		} catch (NameNotFoundException e) {
			e.printStackTrace();
		}

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

		// Initialize the Tango service
		int err = TangoJNINative.Initialize(this);
		if (err != 0) {
			if (err == -2) {
				Toast.makeText(this, 
					"Tango Service version mis-match", Toast.LENGTH_SHORT).show();
			}
			else {
				Toast.makeText(this, 
					"Tango Service initialize internal error", Toast.LENGTH_SHORT).show();
			}
		}

		startUIThread();
	}

	@Override
	protected void onPause() {
		super.onPause();
		TangoJNINative.Disconnect();
	}

	@Override
	protected void onResume() {
		super.onPause();

		// Set up Tango configuration file with auto-reset on
		TangoJNINative.SetupConfig(mIsAutoRecovery);

		// Connect Tango Service
		int err =  TangoJNINative.Connect();
		if (err != 0) {
			Toast.makeText(this, 
					"Tango Service connect error", Toast.LENGTH_SHORT).show();
		}

		mVersion.setText(TangoJNINative.GetVersionNumber());
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
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

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		int pointCount = event.getPointerCount();
		if (pointCount == 1) {
			switch (event.getActionMasked()) {
			case MotionEvent.ACTION_DOWN: {
				TangoJNINative.StartSetCameraOffset();
				touchCurDist = 0.0f;
				touchStartPos[0] = event.getX(0);
				touchStartPos[1] = event.getY(0);
				break;
			}
			case MotionEvent.ACTION_MOVE: {
				touchCurPos[0] = event.getX(0);
				touchCurPos[1] = event.getY(0);

				// Normalize to screen width.
				float normalizedRotX = (touchCurPos[0] - touchStartPos[0])
						/ screenSize.x;
				float normalizedRotY = (touchCurPos[1] - touchStartPos[1])
						/ screenSize.y;

				TangoJNINative.SetCameraOffset(normalizedRotX, normalizedRotY,
						touchCurDist / screenDiagnal);
				Log.i("tango_jni", String.valueOf(touchCurDist / screenDiagnal));
				break;
			}
			}
		}
		if (pointCount == 2) {
			switch (event.getActionMasked()) {
			case MotionEvent.ACTION_POINTER_DOWN: {
				TangoJNINative.StartSetCameraOffset();
				float absX = event.getX(0) - event.getX(1);
				float absY = event.getY(0) - event.getY(1);
				touchStartDist = (float) Math.sqrt(absX * absX + absY * absY);
				break;
			}
			case MotionEvent.ACTION_MOVE: {
				float absX = event.getX(0) - event.getX(1);
				float absY = event.getY(0) - event.getY(1);

				touchCurDist = touchStartDist
						- (float) Math.sqrt(absX * absX + absY * absY);

				TangoJNINative.SetCameraOffset(0.0f, 0.0f, touchCurDist
						/ screenDiagnal);
				break;
			}
			case MotionEvent.ACTION_POINTER_UP: {
				int index = event.getActionIndex() == 0 ? 1 : 0;
				touchStartPos[0] = event.getX(index);
				touchStartPos[1] = event.getY(index);
				break;
			}
			}
		}
		return true;
	}
	
	private void startUIThread() {
		new Thread(new Runnable() {
			@Override
			public void run() {
				while (true) {
					try {
						Thread.sleep(kUpdatIntervalMs);
						runOnUiThread(new Runnable() {
							@Override
							public void run() {
								try {
									mEvent.setText(TangoJNINative.GetEventString());
									mPoseData.setText(TangoJNINative.GetPoseString());
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
