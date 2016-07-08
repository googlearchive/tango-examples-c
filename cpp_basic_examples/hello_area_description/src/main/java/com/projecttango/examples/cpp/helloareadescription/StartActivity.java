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

package com.projecttango.examples.cpp.helloareadescription;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ToggleButton;

/**
 * This starter activity allows user to setup the Area Learning configuration.
 */
public class StartActivity extends Activity {
    // The unique key string for storing user's input.
    public static final String USE_AREA_LEARNING =
            "com.projecttango.examples.cpp.helloareadescription.usearealearning";
    public static final String LOAD_ADF =
            "com.projecttango.examples.cpp.helloareadescription.loadadf";

    private static final String INTENT_CLASS_PACKAGE = "com.google.tango";
    private static final String INTENT_DEPRECATED_CLASS_PACKAGE = "com.projecttango.tango";

    // Key string for load/save Area Description Files.
    private static final String AREA_LEARNING_PERMISSION =
            "ADF_LOAD_SAVE_PERMISSION";

    // Permission request action.
    public static final int REQUEST_CODE_TANGO_PERMISSION = 0;
    private static final String REQUEST_PERMISSION_ACTION =
            "android.intent.action.REQUEST_TANGO_PERMISSION";

    // UI elements.
    private ToggleButton mLearningModeToggleButton;
    private ToggleButton mLoadAdfToggleButton;

    private boolean mIsUseAreaLearning;
    private boolean mIsLoadAdf;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_start);

        // Setup UI elements and listeners.
        mLearningModeToggleButton = (ToggleButton) findViewById(R.id.learningmode);
        mLoadAdfToggleButton = (ToggleButton) findViewById(R.id.loadadf);

        mIsUseAreaLearning = mLearningModeToggleButton.isChecked();
        mIsLoadAdf = mLoadAdfToggleButton.isChecked();

        if (!Util.hasPermission(getApplicationContext(), AREA_LEARNING_PERMISSION)) {
            getPermission(AREA_LEARNING_PERMISSION);
        }
    }

    /**
     * The "Load ADF" button has been clicked.
     * Defined in {@code activity_start.xml}
     * */
    public void loadAdfClicked(View v) {
        mIsLoadAdf = mLoadAdfToggleButton.isChecked();
    }

    /**
     * The "Learning Mode" button has been clicked.
     * Defined in {@code activity_start.xml}
     * */
    public void learningModeClicked(View v) {
        mIsUseAreaLearning = mLearningModeToggleButton.isChecked();
    }

    /**
     * The "Start" button has been clicked.
     * Defined in {@code activity_start.xml}
     * */
    public void startClicked(View v) {
        startAreaDescriptionActivity();
    }

    /**
     * The "ADF List View" button has been clicked.
     * Defined in {@code activity_start.xml}
     * */
    public void adfListViewClicked(View v) {
        startAdfListView();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // The result of the permission activity.
        //
        // Note that when the permission activity is dismissed, the
        // AreaDescription's onResume() callback is called. As the
        // TangoService is connected in the onResume() function, we do not call
        // connect here.
        if (requestCode == REQUEST_CODE_TANGO_PERMISSION) {
            if (resultCode == RESULT_CANCELED) {
                finish();
            }
        }
    }

    // Call the permission intent for the Tango Service to ask for permissions.
    // All permission types can be found here:
    //   https://developers.google.com/project-tango/apis/c/c-user-permissions
    private void getPermission(String permissionType) {
        Intent intent = new Intent();
        intent.setPackage(INTENT_CLASS_PACKAGE);
        if (intent.resolveActivity(getApplicationContext().getPackageManager()) == null) {
            intent = new Intent();
            intent.setPackage(INTENT_DEPRECATED_CLASS_PACKAGE);
        }
        intent.setAction(REQUEST_PERMISSION_ACTION);
        intent.putExtra("PERMISSIONTYPE", permissionType);

        // After the permission activity is dismissed, we will receive a callback
        // function onActivityResult() with user's result.
        startActivityForResult(intent, REQUEST_CODE_TANGO_PERMISSION);
    }

    // Start the main area description activity and pass in user's configuration.
    private void startAreaDescriptionActivity() {
        Intent startADIntent = new Intent(this, AreaDescriptionActivity.class);
        startADIntent.putExtra(USE_AREA_LEARNING, mIsUseAreaLearning);
        startADIntent.putExtra(LOAD_ADF, mIsLoadAdf);
        startActivity(startADIntent);
    }

    // Start the ADF list activity.
    private void startAdfListView() {
        Intent startADFListViewIntent = new Intent(this, AdfUuidListViewActivity.class);
        startActivity(startADFListViewIntent);
    }
}
