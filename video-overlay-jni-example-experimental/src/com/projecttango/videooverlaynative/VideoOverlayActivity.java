package com.projecttango.videooverlaynative;

import com.google.tango.tangojnivideooverlay.R;
import com.projecttango.videooverlaynative.TangoJNINative;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.app.Activity;

public class VideoOverlayActivity extends Activity {
	GLSurfaceView glView;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.activity_video_overlay);
		glView = (GLSurfaceView) findViewById(R.id.surfaceview);
		glView.setRenderer(new Renderer());
	}
	
	@Override
	protected void onResume() {
		super.onResume();
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
