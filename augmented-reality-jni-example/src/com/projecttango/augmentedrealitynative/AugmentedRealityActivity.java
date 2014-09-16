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

package com.projecttango.augmentedrealitynative;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
//import android.view.Menu;
//import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.ToggleButton;

public class AugmentedRealityActivity extends Activity {
	RelativeLayout layout;

	GLSurfaceView arView;
	TextView tangoPoseStatusText;
	TextView isAutoResetText;
	Button startButton;
	Button resetButton;
	Button thirdCamera;
	Button firstCamera;
	Button topCamera;
	ToggleButton isAutoResetButton;

	String[] poseStatuses = { "Initializing", "Valid", "Invalid", "Unknown" };
	boolean isAutoReset = false;
	boolean isStarted = false;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,WindowManager.LayoutParams.FLAG_FULLSCREEN);
		
		setContentView(R.layout.activity_augmented_reality);

		arView = (GLSurfaceView) findViewById(R.id.surfaceview);
		arView.setVisibility(View.GONE);

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
		
		thirdCamera=(Button)findViewById(R.id.third);
		thirdCamera.setVisibility(View.GONE);
		thirdCamera.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				TangoJNINative.SetCamera(1);
			}
		});
		
		firstCamera=(Button)findViewById(R.id.first);
		firstCamera.setVisibility(View.GONE);
		firstCamera.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				TangoJNINative.SetCamera(0);
			}
		});
		
		topCamera=(Button)findViewById(R.id.top);
		topCamera.setVisibility(View.GONE);
		topCamera.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				TangoJNINative.SetCamera(2);
			}
		});

		startButton = (Button) findViewById(R.id.start);
		startButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				if (!isAutoReset) {
					resetButton.setVisibility(View.VISIBLE);
				}
				AugmentedRealityView arViewRenderer = new AugmentedRealityView();
				arViewRenderer.isAutoReset = isAutoReset;
				arView.setRenderer(arViewRenderer);
				arView.setVisibility(View.VISIBLE);	
				
				startButton.setVisibility(View.GONE);
				isAutoResetText.setVisibility(View.GONE);
				isAutoResetButton.setVisibility(View.GONE);
				
				thirdCamera.setVisibility(View.VISIBLE);
				firstCamera.setVisibility(View.VISIBLE);
				topCamera.setVisibility(View.VISIBLE);
				tangoPoseStatusText.setVisibility(View.VISIBLE);

				isStarted = true;
			}
		});

		

		new Thread(new Runnable() {
			@Override
			public void run() {
				while (true) {
					try {
						Thread.sleep(10);
						// final byte statusIndex =
						// TangoJNINative.UpdateStatus();
						// final String tangoPoseStatusString =
						// poseStatuses[statusIndex];
						final String tangoEventString = TangoJNINative
								.EventToString();
						final String tangoPoseString = TangoJNINative
								.PoseToString();
						runOnUiThread(new Runnable() {
							@Override
							public void run() {
								try {
									tangoPoseStatusText
											.setText("Tango Service Version:"+TangoJNINative.GetVersionNumber()+"\nSample App Version: 2014-09-02-a\n\n"
													+ tangoPoseString
													+ "\n"
													+ tangoEventString);
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
}
