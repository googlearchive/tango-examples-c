package com.google.tango.hellotangojni;

public class TangoJNINative {
	static {
		System.loadLibrary("tango-native-jni");
	}

	public static native void onCreate();
	
	public static native void onResume();
	
	public static native void onPause();
	
}