package com.google.tango.hellotangojni;

import android.os.Bundle;
import android.app.Activity;
import android.opengl.GLSurfaceView;

public class HelloTangoJNIActivity extends Activity {

	TangoJNINative nativeJni;
	private GLSurfaceView glSurfaceView;
	private boolean isRendererCreated;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		//setContentView(R.layout.activity_hello_tango_jni);

		//Initiate the Renderer, set content view as glSurfaceView
		glSurfaceView = new GLSurfaceView(this);
		glSurfaceView.setEGLContextClientVersion(2);
		glSurfaceView.setRenderer(new TangoJNIRenderer());
		isRendererCreated = true;
		setContentView(glSurfaceView);
		
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

	@Override
	protected void onPause() {
		super.onPause();
		if (isRendererCreated) {
			glSurfaceView.onPause();
		}
	}

	@Override
	protected void onResume() {
		super.onResume();
		if (isRendererCreated) {
			glSurfaceView.onResume();
		}
	}
}
