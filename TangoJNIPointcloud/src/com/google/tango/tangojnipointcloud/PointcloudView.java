package com.google.tango.tangojnipointcloud;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;

public class PointcloudView extends GLSurfaceView {

	public PointcloudView(Context context) {
		super(context);
		setRenderer(new Renderer());
	}

	private static class Renderer implements GLSurfaceView.Renderer {
		public void onDrawFrame(GL10 gl) {
			TangoJNINative.Render();
		}

		public void onSurfaceChanged(GL10 gl, int width, int height) {
			TangoJNINative.SetupGraphic(width, height);
		}

		public void onSurfaceCreated(GL10 gl, EGLConfig config) {

		}
	}

}
