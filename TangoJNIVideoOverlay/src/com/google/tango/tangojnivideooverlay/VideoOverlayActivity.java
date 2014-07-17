package com.google.tango.tangojnivideooverlay;

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
}
