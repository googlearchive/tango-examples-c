/*
 * Copyright 2014 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.tango.tangojnimotiontracking;

public class TangoJNINative {
	static {
		System.loadLibrary("tango-native-jni");
	}

	public static native void Initialize(boolean isAutoReset);
	
	public static native void ConnectService();
	
	public static native void DisconnectService();

	public static native void OnDestroy();

	public static native void SetupGraphic(int width, int height);

	public static native void Render();

	public static native void SetCamera(int camera_index);
	
	public static native void ResetMotionTracking();
	
	public static native byte UpdateStatus();
	
	public static native String PoseToString();
	
	public static native String EventToString();
	
	public static native String GetVersionNumber();
}
