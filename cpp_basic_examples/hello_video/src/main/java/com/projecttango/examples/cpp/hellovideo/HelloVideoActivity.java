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

package com.projecttango.examples.cpp.hellovideo;

import android.app.Activity;
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.hardware.display.DisplayManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.IBinder;
import android.view.Display;
import android.view.View;
import android.widget.ToggleButton;

import com.projecttango.examples.cpp.util.TangoInitializationHelper;

/**
 * Main activity shows video overlay scene.
 */
public class HelloVideoActivity extends Activity {

    private ServiceConnection mTangoServiceCoonnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder binder) {
            TangoJniNative.onTangoServiceConnected(binder);
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            // Handle this if you need to gracefully shutdown/retry
            // in the event that Tango itself crashes/gets upgraded while running.
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        TangoJniNative.onCreate(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        TangoInitializationHelper.bindTangoService(this, mTangoServiceCoonnection);
    }

    @Override
    protected void onPause() {
        super.onPause();
        TangoJniNative.onPause();
        unbindService(mTangoServiceCoonnection);
    }
}
