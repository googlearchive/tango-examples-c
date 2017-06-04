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

#ifndef CPP_BASIC_EXAMPLES_HELLO_AREA_DESCRIPTION_HELLO_AREA_DESCRIPTION_APP_H_
#define CPP_BASIC_EXAMPLES_HELLO_AREA_DESCRIPTION_HELLO_AREA_DESCRIPTION_APP_H_

#include <jni.h>
#include <memory>
#include <string>
#include <vector>
#include <android/log.h>

#define LOGI(...) \
  __android_log_print(ANDROID_LOG_INFO, "hello_area_description", __VA_ARGS__)
#define LOGE(...) \
  __android_log_print(ANDROID_LOG_ERROR, "hello_area_description", __VA_ARGS__)

#include <tango_client_api.h>  // NOLINT

#include <hello_area_description/pose_data.h>

namespace hello_area_description {

// AreaLearningApp handles the application lifecycle and resources.
class AreaLearningApp {
 public:
  // Constructor and deconstructor.
  AreaLearningApp();
  ~AreaLearningApp();

  // OnCreate() callback is called when this Android application's
  // OnCreate function is called from UI thread. In the OnCreate
  // function, we are only checking the Tango Core's version.
  //
  // @param env, java environment parameter OnCreate is being called.
  // @param caller_activity, caller of this function.
  void OnCreate(JNIEnv* env, jobject caller_activity);

  // Called when the Tango service is connect. We set the binder object to Tango
  // Service in this function.
  //
  // @param env, java environment parameter.
  // @param binder, the native binder object.
  // @param is_area_learning_enabled, enable/disable the area learning mode.
  // @param is_loading_area_description, enable/disable loading the most recent
  // area description.
  // @param mode, the mode to connect the service in.
  void OnTangoServiceConnected(JNIEnv* env, jobject binder,
                               bool is_area_learning_enabled,
                               bool is_loading_area_description);

  // OnPause() callback is called when this Android application's
  // OnCreate function is called from UI thread. In our application,
  // we disconnect Tango Service and free the Tango configuration
  // file. It is important to disconnect Tango Service and release
  // the corresponding resources in the OnPause() callback from
  // Android, otherwise, this application will hold on to the Tango
  // resources and other application will not be able to connect to
  // Tango Service.
  void OnPause();

  // When the Android activity is destroyed, signal the JNI layer to remove
  // references to the activity. This should be called from the onDestroy()
  // callback of the parent activity lifecycle.
  void OnDestroy();

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

  // Return true if Tango has relocalized to the current ADF at least once.
  bool IsRelocalized();

  // Return loaded ADF's UUID.
  std::string GetLoadedAdfString() { return loaded_adf_string_; }

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
}  // namespace hello_area_description

#endif  // CPP_BASIC_EXAMPLES_HELLO_AREA_DESCRIPTION_HELLO_AREA_DESCRIPTION_APP_H_
