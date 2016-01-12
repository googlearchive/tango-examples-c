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
import android.app.FragmentManager;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Point;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

// The AreaDescriptionActivity of the application which shows debug information
// and a glSurfaceView that renders graphic content.
public class AreaDescriptionActivity extends Activity implements
    View.OnClickListener, SetADFNameDialog.CallbackListener, SaveAdfTask.SaveAdfListener {
  // The input argument is invalid.
  private static final int  TANGO_INVALID = -2;
  // This error code denotes some sort of hard error occurred.
  private static final int  TANGO_ERROR = -1;
  // This code indicates success.
  private static final int  TANGO_SUCCESS = 0;

  // The minimum Tango Core version required from this application.
  private static final int  MIN_TANGO_CORE_VERSION = 6804;

  // The package name of Tang Core, used for checking minimum Tango Core version.
  private static final String TANGO_PACKAGE_NAME = "com.projecttango.tango";

  // Tag for debug logging.
  private static final String TAG = AreaDescriptionActivity.class.getSimpleName();

  // The interval at which we'll update our UI debug text in milliseconds.
  // This is the rate at which we query our native wrapper around the tango
  // service for pose and event information.
  private static final int UPDATE_UI_INTERVAL_MS = 100;

  // The flag to check if the surface is created.
  public boolean mIsSurfaceCreated = false;

  // Debug information text.
  // Current frame's pose information.
  private TextView mStartServiceTDevicePoseData;
  private TextView mADFTDevicePoseData;
  private TextView mADFTStartServicePoseData;
  private TextView mAdfUuidTextView;

  // Tango Core version.
  private TextView mVersion;
  // Application version.
  private TextView mAppVersion;
  // Latest Tango Event received.
  private TextView mEvent;

  // GLSurfaceView and its renderer, all of the graphic content is rendered
  // through OpenGL ES 2.0 in the native code.
  private Renderer mRenderer;
  private GLSurfaceView mGLView;

  // Flag that controls whether user wants to run area learning mode.
  private boolean mIsAreaLearningEnabled = false;

  // Flag that controls whether user wants to load the latest ADF file.
  private boolean mIsLoadingADF = false;

  // A flag to check if the Tango Service is connected. This flag avoids the
  // program attempting to disconnect from the service while it is not
  // connected.This is especially important in the onPause() callback for the
  // activity class.
  private boolean mIsConnectedService = false;
  
  // Screen size for normalizing the touch input for orbiting the render camera.
  private Point mScreenSize = new Point();

  // Long-running task to save an ADF.
  private SaveAdfTask mSaveAdfTask;

  // Handles the debug text UI update loop.
  private Handler mHandler = new Handler();

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setTitle(R.string.app_name);
    
    queryDataFromStartActivity();
    setupUIComponents();

    // Check if the Tango Core is out dated.
    if (!CheckTangoCoreVersion(MIN_TANGO_CORE_VERSION)) {
      Toast.makeText(this, "Tango Core out dated, please update in Play Store", 
                     Toast.LENGTH_LONG).show();
      finish();
      return;
    }

    // Initialize Tango Service, this function starts the communication
    // between the application and Tango Service.
    // The activity object is used for checking if the API version is outdated.
    TangoJNINative.initialize(this);
  }

  @Override
  protected void onResume() {
    super.onResume();
    mGLView.onResume();

    // Setup the configuration for the TangoService, passing in our setting
    // for the auto-recovery option.
    TangoJNINative.setupConfig(mIsAreaLearningEnabled, mIsLoadingADF);

    // Connect the onPoseAvailable callback.
    TangoJNINative.connectCallbacks();

    // Connect to Tango Service (returns true on success).
    // Starts Motion Tracking and Area Learning.
    if (TangoJNINative.connect()) {
      mVersion.setText(TangoJNINative.getVersionNumber());
      // Display loaded ADF's UUID.
      mAdfUuidTextView.setText(TangoJNINative.getLoadedADFUUIDString());

      // Set the connected service flag to true.
      mIsConnectedService = true;
    } else {
      // End the activity and let the user know something went wrong.
      Toast.makeText(this, R.string.tango_cant_initialize, Toast.LENGTH_LONG).show();
      finish();
    }

    // Start the debug text UI update loop.
    mHandler.post(mUpdateUiLoopRunnable);
  }

  @Override
  protected void onPause() {
    super.onPause();
    mGLView.onPause();
    TangoJNINative.deleteResources();

    // Disconnect from Tango Service, release all the resources that the app is
    // holding from Tango Service.
    if (mIsConnectedService) {
      TangoJNINative.disconnect();
      mIsConnectedService = false;
    }

    // Stop the debug text UI update loop.
    mHandler.removeCallbacksAndMessages(null);
  }
  
  @Override
  protected void onDestroy() {
    super.onDestroy();
    TangoJNINative.destroyActivity();
  }

  @Override
  public void onClick(View v) {
    // Handle button clicks.
    switch (v.getId()) {
      case R.id.first_person_button:
        TangoJNINative.setCamera(0);
        break;
      case R.id.top_down_button:
        TangoJNINative.setCamera(2);
        break;
      case R.id.third_person_button:
        TangoJNINative.setCamera(1);
        break;
      case R.id.save_adf_button:
        // Ask the user for an ADF name, then save if OK was clicked.
        showSetADFNameDialog();
        break;
      default:
        Log.w(TAG, "Unknown button click");
        return;
    }
  }

  @Override
  public boolean onTouchEvent(MotionEvent event) {
    // Pass the touch event to the native layer for camera control.
    // Single touch to rotate the camera around the device.
    // Two fingers to zoom in and out.
    int pointCount = event.getPointerCount();
    if (pointCount == 1) {
      float normalizedX = event.getX(0) / mScreenSize.x;
      float normalizedY = event.getY(0) / mScreenSize.y;
      TangoJNINative.onTouchEvent(1, 
          event.getActionMasked(), normalizedX, normalizedY, 0.0f, 0.0f);
    }
    if (pointCount == 2) {
      if (event.getActionMasked() == MotionEvent.ACTION_POINTER_UP) {
        int index = event.getActionIndex() == 0 ? 1 : 0;
        float normalizedX = event.getX(index) / mScreenSize.x;
        float normalizedY = event.getY(index) / mScreenSize.y;
        TangoJNINative.onTouchEvent(1, 
          MotionEvent.ACTION_DOWN, normalizedX, normalizedY, 0.0f, 0.0f);
      } else {
        float normalizedX0 = event.getX(0) / mScreenSize.x;
        float normalizedY0 = event.getY(0) / mScreenSize.y;
        float normalizedX1 = event.getX(1) / mScreenSize.x;
        float normalizedY1 = event.getY(1) / mScreenSize.y;
        TangoJNINative.onTouchEvent(2, event.getActionMasked(),
                normalizedX0, normalizedY0, normalizedX1, normalizedY1);
      }
    }
    return true;
  }

  /**
   * Implementation of callback listener interface in SetADFNameDialog.
   */
  @Override
  public void onAdfNameOk(String adfName, String adfUuid) {
    saveAdf(adfName);
  }

  /**
   * Implementation of callback listener interface in SetADFNameDialog.
   */
  @Override
  public void onAdfNameCancelled() {
    // Continue running.
  }

  /**
   * Save the current Area Description File.
   * Performs saving on a background thread and displays a progress dialog.
   */
  private void saveAdf(String adfName) {
    mSaveAdfTask = new SaveAdfTask(this, this, adfName);
    mSaveAdfTask.execute();
  }

  /**
   * Handles failed save from mSaveAdfTask.
   */
  @Override
  public void onSaveAdfFailed(String adfName) {
    String toastMessage = String.format(
      getResources().getString(R.string.save_adf_failed_toast_format),
      adfName);
    Toast.makeText(this, toastMessage, Toast.LENGTH_LONG).show();
    mSaveAdfTask = null;
  }

  /**
   * Handles successful save from mSaveAdfTask.
   */
  @Override
  public void onSaveAdfSuccess(String adfName, String adfUuid) {
    String toastMessage = String.format(
      getResources().getString(R.string.save_adf_success_toast_format),
      adfName, adfUuid);
    Toast.makeText(this, toastMessage, Toast.LENGTH_LONG).show();
    mSaveAdfTask = null;
    finish();
  }

  /**
   * Updates the save progress dialog (called from area_learning_app.cc).
   */
  public void updateSavingAdfProgress(int progress) {
    // Note: this method is not called from the UI thread. We read mSaveAdfTask into
    // a local variable because the UI thread may null-out the member variable at any time.
    SaveAdfTask saveAdfTask = mSaveAdfTask;
    if (saveAdfTask != null) {
      saveAdfTask.publishProgress(progress);
    }
  }

  // Query user's input for the Tango Service configuration.
  private void queryDataFromStartActivity() {
    // Get user's input from the StartActivity.
    Intent initValueIntent = getIntent();
    mIsAreaLearningEnabled =
        initValueIntent.getBooleanExtra(StartActivity.USE_AREA_LEARNING, false);
    mIsLoadingADF =
        initValueIntent.getBooleanExtra(StartActivity.LOAD_ADF, false);
  }

  // Setup UI components in the current activity.
  private void setupUIComponents() {
    setContentView(R.layout.activity_area_description);
    
    // Querying screen size, used for computing the normalized touch point.
    Display display = getWindowManager().getDefaultDisplay();
    display.getSize(mScreenSize);

    // Text views for displaying translation and rotation data
    mStartServiceTDevicePoseData = 
        (TextView) findViewById(R.id.start_service_T_device_textview);
    mADFTDevicePoseData = 
        (TextView) findViewById(R.id.adf_T_device_textview);
    mADFTStartServicePoseData = 
        (TextView) findViewById(R.id.adf_T_start_service_textview);
    mAdfUuidTextView = (TextView) findViewById(R.id.adf_uuid_textview);

    // Text views for displaying most recent Tango Event
    mEvent = (TextView) findViewById(R.id.tango_event_textview);

    // Text views for Tango library versions
    mVersion = (TextView) findViewById(R.id.version_textview);

    // Text views for application versions.
    mAppVersion = (TextView) findViewById(R.id.appversion);
    PackageInfo pInfo;
    try {
      pInfo = this.getPackageManager().getPackageInfo(this.getPackageName(), 0);
      mAppVersion.setText(pInfo.versionName);
    } catch (NameNotFoundException e) {
      e.printStackTrace();
    }

    // Buttons for selecting camera view and Set up button click listeners
    findViewById(R.id.first_person_button).setOnClickListener(this);
    findViewById(R.id.third_person_button).setOnClickListener(this);
    findViewById(R.id.top_down_button).setOnClickListener(this);
    findViewById(R.id.save_adf_button).setOnClickListener(this);

    if (mIsAreaLearningEnabled) {
      // Disable save ADF button until Tango relocalizes to the current ADF.
      findViewById(R.id.save_adf_button).setEnabled(false);
    } else {
      // Hide to save ADF button if leanring mode is off.
      findViewById(R.id.save_adf_button).setVisibility(View.GONE);
    }

    // OpenGL view where all of the graphics are drawn.
    mGLView = (GLSurfaceView) findViewById(R.id.gl_surface_view);
    
    // Configure OpenGL renderer
    mGLView.setEGLContextClientVersion(2);

    // Configure OpenGL renderer
    mRenderer = new Renderer();
    mRenderer.mAreaDescriptionActivity = this;
    mGLView.setRenderer(mRenderer);
  }

  private void showSetADFNameDialog() {
    Bundle bundle = new Bundle();
    bundle.putString("name", getResources().getString(R.string.default_adf_name));
    bundle.putString("id", ""); // UUID is generated after the ADF is saved.

    FragmentManager manager = getFragmentManager();
    SetADFNameDialog setADFNameDialog = new SetADFNameDialog();
    setADFNameDialog.setArguments(bundle);
    setADFNameDialog.show(manager, "ADFNameDialog");
  }

  // Debug text UI update loop, updating at 10Hz.
  private Runnable mUpdateUiLoopRunnable = new Runnable() {
    public void run() {
      updateUi();
      mHandler.postDelayed(this, UPDATE_UI_INTERVAL_MS);
    }
  };

  // Update the debug text UI.
  private void updateUi() {
    try {
      // Update the UI debug displays.
      mEvent.setText(TangoJNINative.getEventString());
      mStartServiceTDevicePoseData.setText(
              TangoJNINative.getStartServiceTDeviceString());
      mADFTDevicePoseData.setText(TangoJNINative.getAdfTDeviceString());
      mADFTStartServicePoseData.setText(TangoJNINative.getAdfTStartServiceString());

      // If Tango has relocalized, allow saving the ADF.
      // Note: Tango returns TANGO_INVALID if saveAdf() is called before relocalization.
      if (TangoJNINative.isRelocalized()) {
        findViewById(R.id.save_adf_button).setEnabled(true);
      }
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  private boolean CheckTangoCoreVersion(int minVersion) {
    int versionNumber = 0;
    String packageName = TANGO_PACKAGE_NAME;
    try {
      PackageInfo pi = getApplicationContext().getPackageManager().getPackageInfo(packageName,
          PackageManager.GET_META_DATA);
      versionNumber = pi.versionCode;
    } catch (NameNotFoundException e) {
      e.printStackTrace();
    }
    return (minVersion <= versionNumber);
  }
}