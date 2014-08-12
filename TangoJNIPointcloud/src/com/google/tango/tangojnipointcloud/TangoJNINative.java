package com.google.tango.tangojnipointcloud;

public class TangoJNINative {
	static {
		System.loadLibrary("tango-native-jni");
	}

	public static native void init(int width, int height);

	public static native void render();
	
	public static native void SetCamera(int camera_index);
}
