package com.google.tango.tangojnipointcloud;

import android.os.Bundle;
import android.app.Activity;

public class PointcloudActivity extends Activity {

	PointcloudView pointcloudView;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		pointcloudView = new PointcloudView(getApplication());
		setContentView(pointcloudView);
	}

}
