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

package com.projecttango.experiments.nativearealearning;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.util.Log;
import android.widget.Button;
import android.widget.Toast;
import android.widget.ToggleButton;

// This starter activity allows user to setup the Area Learning configuration.
public class StartActivity extends Activity implements View.OnClickListener {
  // The unique key string for storing user's input.
  public static final String USE_AREA_LEARNING = 
    "com.projecttango.experiments.areadescriptionjava.usearealearning";
  public static final String LOAD_ADF = 
    "com.projecttango.experiments.areadescriptionjava.loadadf";

  // UI elements.
  private ToggleButton mLearningModeToggleButton;
  private ToggleButton mLoadADFToggleButton;
  private Button mStartMainActivityButton;
  private Button mStartADFListActivityButton;

  private boolean mIsUseAreaLearning;
  private boolean mIsLoadADF;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    setTitle(R.string.app_name);
    setContentView(R.layout.activity_start);
    
    // Setup UI elements and listeners.
    mLearningModeToggleButton = (ToggleButton) findViewById(R.id.learningmode);
    mLearningModeToggleButton.setOnClickListener(this);

    mLoadADFToggleButton = (ToggleButton) findViewById(R.id.loadadf);
    mLoadADFToggleButton.setOnClickListener(this);

    mStartMainActivityButton = (Button) findViewById(R.id.start);
    mStartMainActivityButton.setOnClickListener(this);

    mStartADFListActivityButton = (Button) findViewById(R.id.ADFListView);
    mStartADFListActivityButton.setOnClickListener(this);

    mIsUseAreaLearning = mLearningModeToggleButton.isChecked();
    mIsLoadADF = mLoadADFToggleButton.isChecked();

    // Initialize Tango Service, this function starts the communication
    // between the application and Tango Service.
    // The activity object is used for checking if the API version is outdated.
    TangoJNINative.initialize((Activity)this);
  }

  @Override
  public void onClick(View v) {
    switch (v.getId()) {
    case R.id.loadadf:
      mIsLoadADF = mLoadADFToggleButton.isChecked();
      break;
    case R.id.learningmode:
      mIsUseAreaLearning = mLearningModeToggleButton.isChecked();
      break;
    case R.id.start:
      startAreaDescriptionActivity();
      break;
    case R.id.ADFListView:
      startADFListView();
      break;
    }
  }

  // Start the main area description activity and pass in user's configuration.
  private void startAreaDescriptionActivity() {
    Intent startADIntent = new Intent(this, AreaDescriptionActivity.class);
    startADIntent.putExtra(USE_AREA_LEARNING, mIsUseAreaLearning);
    startADIntent.putExtra(LOAD_ADF, mIsLoadADF);
    startActivity(startADIntent);
  }

  // Start the ADF list activity.
  private void startADFListView() {
    Intent startADFListViewIntent = new Intent(this, ADFUUIDListViewActivity.class);
    startActivity(startADFListViewIntent);
  }
}
