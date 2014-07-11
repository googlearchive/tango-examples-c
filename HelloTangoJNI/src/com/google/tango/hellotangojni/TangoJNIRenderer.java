package com.google.tango.hellotangojni;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.opengl.GLSurfaceView.Renderer;

public class TangoJNIRenderer implements Renderer {
	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		TangoJNINative.onGlSurfaceCreated();
	}

	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height) {
		TangoJNINative.onGLSurfaceChanged(width, height);
	}

	@Override
	public void onDrawFrame(GL10 gl) {
		TangoJNINative.onGLSurfaceDraw();
	}
}