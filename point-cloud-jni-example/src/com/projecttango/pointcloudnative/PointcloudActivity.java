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

package com.projecttango.pointcloudnative;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.app.Activity;
import android.graphics.Point;

public class PointcloudActivity extends Activity implements OnClickListener {
	GLSurfaceView glView;

	private TextView mPoseDataTextView;
	private TextView mTangoEventTextView;
	private TextView mPointCountTextView;
	private TextView mVersionTextView;
	private TextView mAverageZTextView;
	private TextView mFrameDeltaTimeTextView;

	private Button mFirstPersonButton;
	private Button mThirdPersonButton;
	private Button mTopDownButton;

	private float[] touchStartPos = new float[2];
	private float[] touchCurPos = new float[2];
	private float touchStartDist = 0.0f;
	private float touchCurDist = 0.0f;
	private Point screenSize = new Point();
	private float screenDiagnal = 0.0f;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		TangoJNINative.OnCreate();

		Display display = getWindowManager().getDefaultDisplay();
		display.getSize(screenSize);
		screenDiagnal = (float) Math.sqrt(screenSize.x * screenSize.x
				+ screenSize.y * screenSize.y);

		setContentView(R.layout.activity_pointcloud);
		glView = (GLSurfaceView) findViewById(R.id.gl_surface_view);
		glView.setRenderer(new Renderer());

		mVersionTextView = (TextView) findViewById(R.id.version);
		mPointCountTextView = (TextView) findViewById(R.id.pointCount);
		mAverageZTextView = (TextView) findViewById(R.id.averageZ);
		mFrameDeltaTimeTextView = (TextView) findViewById(R.id.frameDelta);
		mTangoEventTextView = (TextView) findViewById(R.id.tangoevent);
		mPoseDataTextView = (TextView) findViewById(R.id.pose_data_textview);

		mVersionTextView.setText(TangoJNINative.GetVersionNumber());

		mFirstPersonButton = (Button) findViewById(R.id.first_person_button);
		mFirstPersonButton.setOnClickListener(this);
		mThirdPersonButton = (Button) findViewById(R.id.third_person_button);
		mThirdPersonButton.setOnClickListener(this);
		mTopDownButton = (Button) findViewById(R.id.top_down_button);
		mTopDownButton.setOnClickListener(this);

		new Thread(new Runnable() {
			@Override
			public void run() {
				while (true) {
					try {
						Thread.sleep(100);
						runOnUiThread(new Runnable() {
							@Override
							public void run() {
								try {
									updateUI();
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
		TangoJNINative.OnResume();
	}

	@Override
	protected void onPause() {
		super.onPause();
		TangoJNINative.OnPause();
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
		case R.id.third_person_button:
			TangoJNINative.SetCamera(1);
			break;
		case R.id.top_down_button:
			TangoJNINative.SetCamera(2);
			break;
		default:
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

	private void updateUI() {
		mTangoEventTextView.setText(TangoJNINative.GetEventString());
		mPoseDataTextView.setText(TangoJNINative.GetPoseString());
		mPointCountTextView.setText(String.valueOf(TangoJNINative
				.GetVerticesCount()));
		mAverageZTextView.setText(String.valueOf(TangoJNINative.GetAverageZ()));
		mFrameDeltaTimeTextView.setText(String.valueOf(TangoJNINative
				.GetFrameDeltaTime()));
	}
}
