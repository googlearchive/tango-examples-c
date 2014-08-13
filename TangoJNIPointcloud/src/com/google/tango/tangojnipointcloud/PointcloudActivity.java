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
	protected void onResume()
	{
		super.onResume();
		pointcloudView.onResume();
		TangoJNINative.OnResume();
	}

	@Override
	protected void onPause()
	{
		super.onPause();
		pointcloudView.onPause();
		TangoJNINative.OnPause();
	}
	
	protected void onDestroy(){
		super.onDestroy();
		TangoJNINative.OnDestroy();
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
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
