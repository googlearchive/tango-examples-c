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

package com.projecttango.areadescriptionnative;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.app.Activity;
import android.view.View;
import android.widget.Button;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

public class AreaDescriptionActivity extends Activity {
	GLSurfaceView glView;
	RelativeLayout layout;

	TextView device2StartText;
	TextView device2ADFText;
	TextView start2ADFText;

	TextView learningModeText;
	TextView uuidText;

	Button saveADFButton;
	Button startButton;
	Button firstPersonCamButton;
	Button thirdPersonCamButton;
	Button topDownCamButton;
	
	ToggleButton isUsingADFToggleButton;
	ToggleButton isLearningToggleButton;

	boolean isUsingADF = false;
	boolean isLearning = false;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_area_description);
		glView = (GLSurfaceView) findViewById(R.id.surfaceview);
		glView.setRenderer(new Renderer());

		device2StartText = (TextView) findViewById(R.id.device_start);
		device2ADFText = (TextView) findViewById(R.id.device_adf);
		start2ADFText = (TextView) findViewById(R.id.start_adf);

		learningModeText = (TextView) findViewById(R.id.learning_mode);
		uuidText = (TextView) findViewById(R.id.uuid);

		saveADFButton = (Button) findViewById(R.id.save_adf_button);
		saveADFButton.setVisibility(View.GONE);
		saveADFButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				String uuid = TangoJNINative.SaveADF();
				CharSequence text = "Saved Map: " + uuid;
				Toast toast = Toast.makeText(getApplicationContext(), text,
						Toast.LENGTH_SHORT);
				toast.show();
			}
		});

		startButton = (Button) findViewById(R.id.start_button);
		startButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				if (isLearning) {
					saveADFButton.setVisibility(View.VISIBLE);
				}
				TangoJNINative.Initialize(isLearning, isUsingADF);
				TangoJNINative.ConnectService();
				startButton.setVisibility(View.GONE);
				isUsingADFToggleButton.setVisibility(View.GONE);
				isLearningToggleButton.setVisibility(View.GONE);
			}
		});

		firstPersonCamButton = (Button) findViewById(R.id.first_person_cam_btn);
		firstPersonCamButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				TangoJNINative.SetCamera(0);
			}
		});
		thirdPersonCamButton = (Button) findViewById(R.id.third_poerson_cam_btn);
		thirdPersonCamButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				TangoJNINative.SetCamera(1);
			}
		});
		topDownCamButton = (Button) findViewById(R.id.top_down_cam_btn);
		topDownCamButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				TangoJNINative.SetCamera(2);
			}
		});
		
		isUsingADFToggleButton = (ToggleButton) findViewById(R.id.load_adf_toggle_button);
		isUsingADFToggleButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				isUsingADF = isUsingADFToggleButton.isChecked();
			}
		});
		isLearningToggleButton = (ToggleButton) findViewById(R.id.learning_mode_toggle_button);
		isLearningToggleButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				isLearning = isLearningToggleButton.isChecked();
			}
		});

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
									device2StartText.setText(TangoJNINative.GetPoseString(0));
									device2ADFText.setText(TangoJNINative.GetPoseString(1));
									start2ADFText.setText(TangoJNINative.GetPoseString(2));

									uuidText.setText(TangoJNINative.GetUUID());
									learningModeText.setText(isLearning?"Enabled":"Disabled");
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
