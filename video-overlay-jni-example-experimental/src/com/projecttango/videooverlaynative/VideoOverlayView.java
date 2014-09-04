package com.projecttango.videooverlaynative;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;

public class VideoOverlayView extends GLSurfaceView {

	public VideoOverlayView(Context context) {
		super(context);
		setRenderer(new Renderer());
	}

	private static class Renderer implements GLSurfaceView.Renderer {
		public void onDrawFrame(GL10 gl) {
			TangoJNINative.Render();
		}

		public void onSurfaceChanged(GL10 gl, int width, int height) {
			TangoJNINative.SetupGraphic(width, height);
			TangoJNINative.OnCreate();
			TangoJNINative.OnResume();
		}

		public void onSurfaceCreated(GL10 gl, EGLConfig config) {
			// Do nothing.
		}
	}

}
