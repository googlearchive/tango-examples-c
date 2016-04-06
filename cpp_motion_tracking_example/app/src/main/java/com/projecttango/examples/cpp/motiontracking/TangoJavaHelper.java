/*
 * Copyright 2016 Google Inc. All Rights Reserved.
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

package com.projecttango.examples.cpp.motiontracking;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;

/**
 * Functions for simplifying the process of initializing TangoService.
 */
public class TangoJavaHelper {
    /**
     * Callbacks used by native code to initialize the Tango Service.
     *
     * When onNativeTangoServiceReady gets called, it is the client's responsibility
     * to call TangoService_setBinder(JNIEnv* env, jobject iBinder) in the C API
     * to complete the initialization process.
     */
    public interface OnNativeTangoServiceReadyListener {
        public void onNativeTangoServiceReady(IBinder nativeTangoServiceBinder);
    }

    /**
     * Only for apps using the C API:
     * Initializes the underlying TangoService for native apps.
     * 
     * @param context The Android application context TangoService should be associated with.
     * @param listener The callback to use when TangoService is ready.
     * @return ServiceConnection A handle to the TangoService which is needed to shut down Tango.
     */
    public static final ServiceConnection nativeInitTango(final Context context,
            final OnNativeTangoServiceReadyListener listener) {
        Intent intent = new Intent();
        intent.setClassName("com.google.tango", "com.google.atap.tango.TangoService");
        boolean hasJavaService = (context.getPackageManager().resolveService(intent, 0) != null);
        // User doesn't have the latest packagename for TangoCore, fallback to the previous name.
        if (!hasJavaService) {
            intent = new Intent();
            intent.setClassName("com.projecttango.tango", "com.google.atap.tango.TangoService");
            hasJavaService = (context.getPackageManager().resolveService(intent, 0) != null);
        }
        // User doesn't have a Java-fied TangoCore at all; fallback to the deprecated approach
        // of doing nothing and letting the native side auto-init to the system-service version
        // of Tango.
        if (!hasJavaService) {
            return null;
        }
        ServiceConnection connection = new ServiceConnection() {
                    public void onServiceConnected(ComponentName name, IBinder service) {
                        listener.onNativeTangoServiceReady(service);
                    }

                    public void onServiceDisconnected(ComponentName name) {
                        // Handle this if you need to gracefully shutdown/retry
                        // in the event that Tango itself crashes/gets upgraded while running.
                    }
                };
        context.bindService(intent, connection, Context.BIND_AUTO_CREATE);
        return connection;
    }

    /**
     * Only for apps using the C API:
     * Shuts down the underlying TangoService for native apps.
     * 
     * @param context The Android application context the Tango interface should be associated with.
     * @param connection A handle to the TangoService which is needed to shut down Tango.
     */
    public static final void nativeShutdownTango(final Context context,
            final ServiceConnection connection) {
        if ((connection != null) && (context != null)) {
          context.unbindService(connection);
        }
    }
}
