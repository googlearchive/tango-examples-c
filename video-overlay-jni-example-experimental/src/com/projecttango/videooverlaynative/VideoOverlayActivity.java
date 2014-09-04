package com.projecttango.videooverlaynative;

import com.projecttango.videooverlaynative.TangoJNINative;

import android.os.Bundle;
import android.app.Activity;

public class VideoOverlayActivity extends Activity {

	VideoOverlayView videoOverlayView;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		videoOverlayView = new VideoOverlayView(getApplication());
		setContentView(videoOverlayView);
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
