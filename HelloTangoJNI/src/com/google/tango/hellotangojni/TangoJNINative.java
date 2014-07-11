package com.google.tango.hellotangojni;

public class TangoJNINative {
	public native void initApplication();
    public native void updateVIO();
	public native static void onGlSurfaceCreated();
	public native static void onGLSurfaceChanged(int width, int height);
	public native static void onGLSurfaceDraw();
    
    static {
        System.loadLibrary("hello-tango-jni");
    }
}
