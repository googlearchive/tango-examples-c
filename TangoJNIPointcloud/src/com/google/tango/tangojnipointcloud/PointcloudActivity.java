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
package com.google.tango.tangojnipointcloud;

import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.app.Activity;

public class PointcloudActivity extends Activity {

	PointcloudView pointcloudView;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		pointcloudView = new PointcloudView(getApplication());
		setContentView(pointcloudView);
		TangoJNINative.OnCreate();
	}

	@Override
	protected void onResume() {
		super.onResume();
		pointcloudView.onResume();
		TangoJNINative.OnResume();
	}

	@Override
	protected void onPause() {
		super.onPause();
		pointcloudView.onPause();
		TangoJNINative.OnPause();
	}

	protected void onDestroy() {
		super.onDestroy();
		TangoJNINative.OnDestroy();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.pointcloud, menu);
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
