package com.projecttango.ctangojniareadescription;

public class TangoJNINative {
	static {
		System.loadLibrary("tango-native-jni");
	}

	public static native void OnCreate(int isRecording);

	public static native void OnResume();

	public static native void OnPause();

	public static native void OnDestroy();

	public static native void SetupGraphic(int width, int height);

	public static native void Render();

	public static native void SetCamera(int camera_index);
	
	public static native int GetCurrentStatus(int index);
	
	public static native void SaveADF();
	
	public static native void RemoveAllAdfs();
}
