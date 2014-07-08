package com.google.tango.hellotangojni;

public class TangoJNINative {
	public native void initApplication();
    public native void updateVIO();
    
    static {
        System.loadLibrary("hello-tango-jni");
    }
}
