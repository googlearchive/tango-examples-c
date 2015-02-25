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

package com.projecttango.experiments.nativeactivity;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.Toast;

/**
 * Ensures the correct permissions are set before launching the actual NativeActivity.
 */
public class PermissionActivity extends Activity {
    public static final String EXTRA_KEY_PERMISSIONTYPE = "PERMISSIONTYPE";
    public static final String EXTRA_VALUE_VIO = "MOTION_TRACKING_PERMISSION";
    public static final String EXTRA_VALUE_VIOADF = "ADF_LOAD_SAVE_PERMISSION";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        // For native activity, the native libraries needs to be explicitly loaded
        // before the native activity starts.
        System.loadLibrary("tango_client_api");
        System.loadLibrary("native_activity_example");

        super.onCreate(savedInstanceState);
        Intent intent1 = new Intent();
        intent1.setAction("android.intent.action.REQUEST_TANGO_PERMISSION");
        intent1.putExtra(EXTRA_KEY_PERMISSIONTYPE, EXTRA_VALUE_VIO);
        startActivityForResult(intent1, 0);

        Intent intent2 = new Intent();
        intent2.setAction("android.intent.action.REQUEST_TANGO_PERMISSION");
        intent2.putExtra(EXTRA_KEY_PERMISSIONTYPE, EXTRA_VALUE_VIOADF);
        startActivityForResult(intent2, 0);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == 0) {
            if (resultCode == RESULT_CANCELED) {
                Toast.makeText(this,
                "Motion Tracking Permission Needed!", Toast.LENGTH_SHORT).show();
                finish();
            }
        }
        if (requestCode == 1) {
            if (resultCode == RESULT_CANCELED) {
                Toast.makeText(this,
                "ADF Permission Needed!", Toast.LENGTH_SHORT).show();
                finish();
            }
        }
        // If permission granted, start Native Activity, and by this moment, 
        // two shared libs are already loaded.
        Intent intent = new Intent(this, android.app.NativeActivity.class);
        this.startActivity(intent);
    }
}
