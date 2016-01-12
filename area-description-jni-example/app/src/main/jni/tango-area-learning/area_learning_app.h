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

#ifndef TANGO_AREA_LEARNING_AREA_LEARNING_APP_H_
#define TANGO_AREA_LEARNING_AREA_LEARNING_APP_H_

#include <jni.h>
#include <memory>

#include <tango_client_api.h>  // NOLINT
#include <tango-gl/util.h>

#include <tango-area-learning/pose_data.h>
#include <tango-area-learning/scene.h>
#include <tango-area-learning/tango_event_data.h>


namespace tango_area_learning {

// AreaLearningApp handles the application lifecycle and resources.
class AreaLearningApp {
 public:
  // Constructor and deconstructor.
  AreaLearningApp();
  ~AreaLearningApp();

  // Initialize Tango Service, this function starts the communication
  // between the application and Tango Service.
  // The activity object is used for checking if the API version is outdated.
  int TangoInitialize(JNIEnv* env, jobject caller_activity);

  // When the Android activity is destroyed, signal the JNI layer to remove
  // references to the activity. This should be called from the onDestroy()
  // callback of the parent activity lifecycle.
  void ActivityDestroyed();

  // Setup the configuration file for the Tango Service. We'll also se whether
  // we'd like auto-recover enabled.
  //
  // @param is_area_learning_enabled: enable/disable the area learning mode.
  // @param is_loading_adf: load the most recent Adf.
  int TangoSetupConfig(bool is_area_learning_enabled, bool is_loading_adf);

  // Connect the onPoseAvailable callback.
  int TangoConnectCallbacks();

  // Connect to Tango Service.
  // This function will start the Tango Service pipeline.
  bool TangoConnect();

  // Disconnect from Tango Service, release all the resources that the app is
  // holding from Tango Service.
  void TangoDisconnect();

  // Explicitly reset motion tracking and restart the pipeline.
  // Note that this will cause motion tracking to re-initialize. When an ADF is
  // loaded it will re-start the relocalization process.
  void TangoResetMotionTracking();

  // Save current ADF in learning mode. Note that the save function only works
  // when learning mode is on.
  //
  // @return: UUID of the saved ADF.
  std::string SaveAdf();

  // Get specifc meta value of an exsiting ADF.
  //
  // @param uuid: the UUID of the targeting ADF.
  // @param key: key value.
  //
  // @retun: the value queried from the Tango Service.
  std::string GetAdfMetadataValue(const std::string& uuid,
                                  const std::string& key);

  // Set specific meta value to an exsiting ADF.
  //
  // @param uuid: the UUID of the targeting ADF.
  // @param key: the key of the metadata.
  // @param value: the value that is going to be assigned to the key.
  void SetAdfMetadataValue(const std::string& uuid, const std::string& key,
                           const std::string& value);

  // Get all ADF's UUIDs list in one string, saperated by comma.
  //
  // @return: all ADF's UUIDs.
  std::string GetAllAdfUuids();

  // Delete a specific ADF.
  //
  // @param uuid: target ADF's uuid.
  void DeleteAdf(std::string uuid);

  // Tango service pose callback function for pose data. Called when new
  // information about device pose is available from the Tango Service.
  //
  // @param pose: The current pose returned by the service, caller allocated.
  void onPoseAvailable(const TangoPoseData* pose);

  // Tango service event callback function for pose data. Called when new events
  // are available from the Tango Service.
  //
  // @param event: Tango event, caller allocated.
  void onTangoEventAvailable(const TangoEvent* event);

  // Allocate OpenGL resources for rendering, mainly initializing the Scene.
  void InitializeGLContent();

  // Setup the view port width and height.
  void SetViewPort(int width, int height);

  // Main render loop.
  void Render();

  // Reset pose data and release resources that allocate from the program.
  void DeleteResources();

  // Return true if Tango has relocalized to the current ADF at least once.
  bool IsRelocalized();

  // Get the debug string for the Device with respect to Start Service frame.
  //
  // @return: pose debug strings for dispaly on Java activity.
  std::string GetStartServiceTDeviceString();

  // Get the debug string for the Device with respect to ADF frame.
  //
  // @return: pose debug strings for dispaly on Java activity.
  std::string GetAdfTDeviceString();

  // Get the debug string for the Start service with respect to ADF frame.
  //
  // @return: pose debug strings for dispaly on Java activity.
  std::string GetAdfTStartServiceString();

  // Retrun Tango event debug string.
  std::string GetEventString();

  // Retrun Tango Service version string.
  std::string GetVersionString();

  std::string GetLoadedAdfString() { return loaded_adf_string_; }

  // Set render camera's viewing angle, first person, third person or top down.
  //
  // @param: camera_type, camera type includes first person, third person and
  //         top down
  void SetCameraType(tango_gl::GestureCamera::CameraType camera_type);

  // Touch event passed from android activity. This function only supports two
  // touches.
  //
  // @param: touch_count, total count for touches.
  // @param: event, touch event of current touch.
  // @param: x0, normalized touch location for touch 0 on x axis.
  // @param: y0, normalized touch location for touch 0 on y axis.
  // @param: x1, normalized touch location for touch 1 on x axis.
  // @param: y1, normalized touch location for touch 1 on y axis.
  void OnTouchEvent(int touch_count, tango_gl::GestureCamera::TouchEvent event,
                    float x0, float y0, float x1, float y1);

  // Cache the Java VM
  //
  // @JavaVM java_vm: the Java VM is using from the Java layer.
  void SetJavaVM(JavaVM* java_vm) { java_vm_ = java_vm; }

  // Callback function when the Adf saving progress.
  //
  // @JavaVM progress: current progress value, the value is between 1 to 100,
  //                   inclusive.
  void OnAdfSavingProgressChanged(int progress);

 private:
  // Get the Tango Service version.
  //
  // @return: Tango Service's version.
  std::string GetTangoServiceVersion();

  // Get the vector list of all ADF stored in the Tango space.
  //
  // @adf_list: ADF UUID list to be filled in.
  void GetAdfUuids(std::vector<std::string>* adf_list);

  // pose_data_ handles all pose onPoseAvailable callbacks, onPoseAvailable()
  // in this object will be routed to pose_data_ to handle.
  PoseData pose_data_;

  // Mutex for protecting the pose data. The pose data is shared between render
  // thread and TangoService callback thread.
  std::mutex pose_mutex_;

  // tango_event_data_ handles all Tango event callbacks,
  // onTangoEventAvailable() in this object will be routed to tango_event_data_
  // to handle.
  TangoEventData tango_event_data_;

  // tango_event_data_ is share between the UI thread we start for updating
  // debug
  // texts and the TangoService event callback thread. We keep event_mutex_ to
  // protect tango_event_data_.
  std::mutex tango_event_mutex_;

  // main_scene_ includes all drawable object for visualizing Tango device's
  // movement.
  Scene main_scene_;

  // Tango configration file, this object is for configuring Tango Service setup
  // before connect to service. For example, we set the flag
  // config_enable_auto_recovery based user's input and then start Tango.
  TangoConfig tango_config_;

  // Tango service version string.
  std::string tango_core_version_string_;

  // Current loaded ADF.
  std::string loaded_adf_string_;

  // Cached Java VM, caller activity object and the request render method. These
  // variables are used for get the saving Adf progress bar update.
  JavaVM* java_vm_;
  jobject calling_activity_obj_;
  jmethodID on_saving_adf_progress_updated_;
};
}  // namespace tango_area_learning

#endif  // TANGO_AREA_LEARNING_AREA_LEARNING_APP_H_