package com.google.tango.tangojnimotiontracking;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ViewGroup.LayoutParams;
import android.widget.TextView;

public class MotionTrackingActivity extends Activity {

	MotionTrackingView motionTrackingView;
	TextView tangoPoseStatusText;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		motionTrackingView = new MotionTrackingView(this);
		tangoPoseStatusText = new TextView(this);

		setContentView(motionTrackingView);
		addContentView(tangoPoseStatusText, new LayoutParams(LayoutParams.WRAP_CONTENT,
				LayoutParams.WRAP_CONTENT));
		TangoJNINative.OnCreate();

		new Thread(new Runnable() {
			@Override
			public void run() {
				while (true) {
					try {
						Thread.sleep(10);
						final byte tangoPoseStatus = TangoJNINative
								.UpdateStatus();
						runOnUiThread(new Runnable() {
							@Override
							public void run() {
								try {
									switch (tangoPoseStatus) {
									case 0:
										tangoPoseStatusText.setText("Pose Status: Unknown");
										break;
									case 1:
										tangoPoseStatusText.setText("Pose Status: Initializing");
										break;
									case 2:
										tangoPoseStatusText.setText("Pose Status: Valid");
										break;
									default:
										tangoPoseStatusText.setText("Pose Status: Invalid");
										break;
									}

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