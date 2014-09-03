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
import android.view.View;
import android.widget.Button;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.app.Activity;

public class PointcloudActivity extends Activity {
	GLSurfaceView glView;
	RelativeLayout layout;
	TextView versionText;
	TextView verticesCountText;
	TextView averageZText;
	TextView deltaTimeText;

	Button firstPersonCamButton;
	Button thirdPersonCamButton;
	Button topDownCamButton;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		TangoJNINative.OnCreate();

		setContentView(R.layout.activity_pointcloud);
		glView = (GLSurfaceView) findViewById(R.id.surfaceview);
		glView.setRenderer(new Renderer());

		versionText = (TextView) findViewById(R.id.version);
		verticesCountText = (TextView) findViewById(R.id.vertexCount);
		averageZText = (TextView) findViewById(R.id.averageZ);
		deltaTimeText = (TextView) findViewById(R.id.deltaTime);

		versionText.setText(TangoJNINative.GetVersionNumber());

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

		new Thread(new Runnable() {
			@Override
			public void run() {
				while (true) {
					try {
						Thread.sleep(10);
						final int verticesCount = TangoJNINative
								.GetVerticesCount();
						runOnUiThread(new Runnable() {
							@Override
							public void run() {
								try {
									verticesCountText.setText(String
											.valueOf(verticesCount));
									averageZText.setText(String
											.valueOf(TangoJNINative
													.GetAverageZ()));
									deltaTimeText.setText(String
											.valueOf(TangoJNINative
													.GetDepthFPS()));
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

	protected void onDestroy() {
		super.onDestroy();
		TangoJNINative.OnDestroy();
	}

}
