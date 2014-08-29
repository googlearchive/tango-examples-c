package com.projecttango.ctangojniareadescription;

public class TangoJNINative {
	static {
		System.loadLibrary("tango-native-jni");
	}

	public static native void Initialize(int isRecording);

	public static native void ConnectService();

	public static native void DisconnectService();

	public static native void OnDestroy();

	public static native void SetupGraphic(int width, int height);

	public static native void Render();

	public static native void SetCamera(int camera_index);
	
	public static native double GetCurrentTimestamp(int index);
	
	public static native void SaveADF();
	
	public static native void RemoveAllAdfs();
	
	public static native String GetUUID();
	
	public static native String GetIsEnabledLearn();
	
	public static native String GetIsRelocalized();
	
	public static native String GetPoseString(int index);
}
