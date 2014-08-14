package com.google.tango.tangojnivideooverlay;

public class TangoJNINative {
	static {
		System.loadLibrary("tango-native-jni");
	}

	public static native void init(int width, int height);

	public static native void render();
}
