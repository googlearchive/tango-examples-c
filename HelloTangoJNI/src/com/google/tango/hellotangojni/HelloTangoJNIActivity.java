package com.google.tango.hellotangojni;

import android.os.Bundle;
import android.app.Activity;

public class HelloTangoJNIActivity extends Activity {

	TangoJNINative nativeJni;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_hello_tango_jni);
		
		nativeJni = new TangoJNINative();
        nativeJni.initApplication();
        new Thread(new Runnable() 
        {
			@Override
			public void run() 
			{
				while(true)
				{
					try 
					{
						Thread.sleep(10);
						nativeJni.updateVIO();
					} 
					catch (InterruptedException e)
					{
						e.printStackTrace();
					}
				}
			}
		}).start();
	}
}
