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

package com.projecttango.experiments.rgbdepthsync;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Point;
import android.net.Uri;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.WindowManager;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.Toast;

/**
 * Activity that load up the main screen of the app, this is the launcher activity.
 */
public class MainActivity extends Activity {
    // Used for startActivityForResult on our motion tracking permission.
    private static final int REQUEST_PERMISSION_MOTION_TRACKING = 0;
    /// The input argument is invalid.
    private static final int  TANGO_INVALID = -2;
    /// This error code denotes some sort of hard error occurred.
    private static final int  TANGO_ERROR = -1;
    /// This code indicates success.
    private static final int  TANGO_SUCCESS = 0;

    // Motion Tracking permission request action.
    private static final String MOTION_TRACKING_PERMISSION_ACTION =
            "android.intent.action.REQUEST_TANGO_PERMISSION";

    // Key string for requesting and checking Motion Tracking permission.
    private static final String MOTION_TRACKING_PERMISSION =
            "MOTION_TRACKING_PERMISSION";

    private GLSurfaceRenderer mRenderer;
    private GLSurfaceView mGLView;

    private SeekBar mDepthOverlaySeekbar;
    private CheckBox mdebugOverlayCheckbox;

    private boolean mIsConnectedService = false;

    private static final String TAG = "RGBDepthSync";

    private class DepthOverlaySeekbarListner implements SeekBar.OnSeekBarChangeListener {
        @Override
        public void onProgressChanged(SeekBar seekBar, int progress,
                boolean fromUser) {
            JNIInterface.setDepthAlphaValue((float) progress / (float) seekBar.getMax());
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {}

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {}
    }

    private class DebugOverlayCheckboxListner implements CheckBox.OnCheckedChangeListener {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            if (buttonView == mdebugOverlayCheckbox) {
                if (isChecked) {
                    float progress = mDepthOverlaySeekbar.getProgress();
                    float max = mDepthOverlaySeekbar.getMax();
                    JNIInterface.setDepthAlphaValue(progress / max);
                    mDepthOverlaySeekbar.setVisibility(View.VISIBLE);
                } else {
                    JNIInterface.setDepthAlphaValue(0.0f);
                    mDepthOverlaySeekbar.setVisibility(View.GONE);
                }
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Display display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        int status = JNIInterface.tangoInitialize(this);
        if (status != TANGO_SUCCESS) {
          if (status == TANGO_INVALID) {
            Toast.makeText(this, 
              "Tango Service version mis-match", Toast.LENGTH_SHORT).show();
          } else {
            Toast.makeText(this, 
              "Tango Service initialize internal error",
              Toast.LENGTH_SHORT).show();
          }
        }
        setContentView(R.layout.activity_main);

        mDepthOverlaySeekbar = (SeekBar) findViewById(R.id.depth_overlay_alpha_seekbar);
        mDepthOverlaySeekbar.setOnSeekBarChangeListener(new DepthOverlaySeekbarListner());
        mDepthOverlaySeekbar.setVisibility(View.GONE);
        
        mdebugOverlayCheckbox = (CheckBox) findViewById(R.id.debug_overlay_checkbox);
        mdebugOverlayCheckbox.setOnCheckedChangeListener(new DebugOverlayCheckboxListner());

        // OpenGL view where all of the graphics are drawn
        mGLView = (GLSurfaceView) findViewById(R.id.gl_surface_view);

        // Configure OpenGL renderer
        mRenderer = new GLSurfaceRenderer(this);
        mGLView.setRenderer(mRenderer);
    }

    @Override
    protected void onResume() {
        // We moved most of the onResume lifecycle calls to the surfaceCreated,
        // surfaceCreated will be called after the GLSurface is created.
        super.onResume();

        // Though we're going to use Tango's C interface so that we have more
        // low level control of our graphics, we can still use the Java API to
        // check that we have the correct permissions.
        if (!hasPermission(this, MOTION_TRACKING_PERMISSION)) {
            getMotionTrackingPermission();
        } else {
            mGLView.onResume();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        mGLView.onPause();
        if (mIsConnectedService) {
            JNIInterface.tangoDisconnect();
        }
        JNIInterface.freeGLContent();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    public void surfaceCreated() {
        JNIInterface.initializeGLContent();
        int ret = JNIInterface.tangoConnectTexture();
        if (ret != TANGO_SUCCESS) {
            Log.e(TAG, "Failed to connect texture with code: "  + ret);
            finish();
        }

        ret = JNIInterface.tangoSetupConfig();
        if (ret != TANGO_SUCCESS) {
            Log.e(TAG, "Failed to set config with code: "  + ret);
            finish();
        }

        ret = JNIInterface.tangoConnectCallbacks();
        if (ret != TANGO_SUCCESS) {
            Log.e(TAG, "Failed to set connect cbs with code: "  + ret);
            finish();
        }

        ret = JNIInterface.tangoConnect();
        if (ret != TANGO_SUCCESS) {
            Log.e(TAG, "Failed to set connect service with code: "  + ret);
            finish();
        }

        ret = JNIInterface.tangoSetIntrinsicsAndExtrinsics();
        if (ret != TANGO_SUCCESS) {
            Log.e(TAG, "Failed to extrinsics and intrinsics code: "  + ret);
            finish();
        }

        mIsConnectedService = true;
    }

    @Override
    protected void onActivityResult (int requestCode, int resultCode, Intent data) {
        if (requestCode == REQUEST_PERMISSION_MOTION_TRACKING) {
            if (resultCode == RESULT_CANCELED) {
                mIsConnectedService = false;
                finish();
            }
        }
    }

    public boolean hasPermission(Context context, String permissionType){
        Uri uri = Uri.parse("content://com.google.atap.tango.PermissionStatusProvider/" +
                permissionType);
        Cursor cursor = context.getContentResolver().query(uri, null, null, null, null);
        if (cursor == null) {
            return false;
        } else {
            return true;
        }
    }

    // Call the permission intent for the Tango Service to ask for motion tracking
    // permissions. All permission types can be found here:
    //   https://developers.google.com/project-tango/apis/c/c-user-permissions
    private void getMotionTrackingPermission() {
        Intent intent = new Intent();
        intent.setAction(MOTION_TRACKING_PERMISSION_ACTION);
        intent.putExtra("PERMISSIONTYPE", MOTION_TRACKING_PERMISSION);

        // After the permission activity is dismissed, we will receive a callback
        // function onActivityResult() with user's result.
        startActivityForResult(intent, 0);
    }
}
