package com.google.tango.tangojnimotiontracking;

import android.app.Activity;
import android.os.Bundle;

public class MotionTrackingActivity extends Activity {

	MotionTrackingView motionTrackingView;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		motionTrackingView = new MotionTrackingView(getApplication());
		setContentView(motionTrackingView);
	}
}
