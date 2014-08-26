package com.google.tango.hellotangojni;

import android.os.Bundle;
import android.app.Activity;

public class MainActivity extends Activity {

	TangoJNINative tangoJNINative;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		TangoJNINative.onCreate();
	}
	
	@Override
	protected void onResume()
	{
		super.onResume();
		TangoJNINative.onResume();
	}

	@Override
	protected void onPause()
	{
		super.onPause();
		TangoJNINative.onPause();
	}

}
