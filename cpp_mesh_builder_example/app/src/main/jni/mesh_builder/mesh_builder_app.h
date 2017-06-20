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

#ifndef CPP_MESH_BUILDER_EXAMPLE_MESH_BUILDER_MESH_BUILDER_APP_H_
#define CPP_MESH_BUILDER_EXAMPLE_MESH_BUILDER_MESH_BUILDER_APP_H_

#include <jni.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <tango_client_api.h>  // NOLINT
#include <tango-gl/tango-gl.h>
#include <tango-gl/util.h>
#include <tango_3d_reconstruction_api.h>
#include <tango_support.h>

#include "mesh_builder/scene.h"

namespace mesh_builder {

// GridIndex makes indices into a value-struct instead of just an
// array.
struct GridIndex {
  Tango3DR_GridIndex indices;

  bool operator==(const GridIndex& other) const;
};

// Hash functor for GridIndex.
struct GridIndexHasher {
  std::size_t operator()(const mesh_builder::GridIndex& index) const {
    std::size_t val = std::hash<int>()(index.indices[0]);
    val = hash_combine(val, std::hash<int>()(index.indices[1]));
    val = hash_combine(val, std::hash<int>()(index.indices[2]));
    return val;
  }

  // This is a simple hash_combine function for illustrative purposes.
  // Replace this with a better hash function.
  static std::size_t hash_combine(std::size_t val1, std::size_t val2) {
    return (val1 << 1) ^ val2;
  }
};

struct SingleDynamicMesh {
  tango_gl::StaticMesh mesh;
  bool needs_to_grow;
};

// MeshBuilderApp handles the application lifecycle and resources and
// sends data to the 3D Reconstruction library.
class MeshBuilderApp {
 public:
  // Constructor and deconstructor.
  MeshBuilderApp();
  ~MeshBuilderApp();

  // ActivityCtor is called when this Android application's
  // constructor is called.
  void ActivityCtor(bool t3dr_is_running);

  // OnCreate() callback is called when this Android application's
  // OnCreate function is called from UI thread. In the OnCreate
  // function, we are only checking the Tango Core's version.
  //
  // @param env, java environment parameter OnCreate is being called.
  // @param caller_activity, caller of this function.
  void OnCreate(JNIEnv* env, jobject caller_activity);

  // OnPause() callback is called when this Android application's
  // OnCreate function is called from UI thread. In our application,
  // we disconnect Tango Service and free the Tango configuration
  // file. It is important to disconnect Tango Service and release
  // the coresponding resources in the OnPause() callback from
  // Android, otherwise, this application will hold on to the Tango
  // resources and other application will not be able to connect to
  // Tango Service.
  void OnPause();

  // Called when Tango service is connected.
  void OnTangoServiceConnected(JNIEnv* env, jobject binder);

  // Tango service point cloud callback function for depth data. Called when new
  // point cloud data is available from the Tango Service.
  //
  // @param pose: The current pose returned by the service, caller allocated.
  void onPointCloudAvailable(const TangoPointCloud* point_cloud);

  // Tango service image callback function for camera data. Called when new
  // camera data is available from the Tango Service.
  //
  // @param id: camera id for the image.
  // @param buffer: image data.
  void onFrameAvailable(TangoCameraId id, const TangoImageBuffer* buffer);

  // Allocate OpenGL resources for rendering, mainly initializing the Scene.
  void OnSurfaceCreated();

  // Setup the view port width and height.
  void OnSurfaceChanged(int width, int height);

  // Main render loop.
  void OnDrawFrame();

  // Called when the Toggle button is clicked. t3dr_is_running is the new
  // running state.
  void OnToggleButtonClicked(bool t3dr_is_running);

  // Called when the Clear button is clicked.
  void OnClearButtonClicked();

 private:
  // Setup the configuration file for the Tango Service.
  void TangoSetupConfig();

  // Setup the 3D reconstruction service and objects.
  void TangoSetup3DR();

  // Connect the onPoseAvailable callback.
  void TangoConnectCallbacks();

  // Connect to Tango Service.
  // This function will start the Tango Service pipeline, in this case, it will
  // start Motion Tracking, Depth Sensing, and 3D Reconstruction.
  void TangoConnect();

  // Disconnect from Tango Service, release all the resources that the app is
  // holding from Tango Service.
  void TangoDisconnect();

  // Release all resources that allocate from the program.
  void DeleteResources();

  // Last valid transform of Device to Start of Service.
  glm::mat4 start_service_T_device_;

  // Context for a 3D Reconstruction.  Maintains the state of a single
  // mesh being reconstructed.
  Tango3DR_ReconstructionContext t3dr_context_;

  // If the 3D Reconstruction is paused or not.  When paused, depth
  // and color updates will get ignored.
  bool t3dr_is_running_;

  // Constant camera intrinsics for the color camera.
  Tango3DR_CameraCalibration t3dr_intrinsics_;

  // Set if there depth points are available.
  bool point_cloud_available_;

  // Point cloud manager
  TangoSupport_PointCloudManager* point_cloud_manager_;

  // The point cloud of the most recent depth received.  Stored
  // as float tuples (X,Y,Z,C).
  TangoPointCloud* front_cloud_;

  // A matrix for open_gl_T_depth_camera of the most recent depth
  // received.
  //
  // Only meaningful if point_cloud_available_ is true.
  glm::mat4 point_cloud_matrix_;

  // Mutex for protecting all Tango data. Tango data is shared between render
  // thread and TangoService callback binder thread.
  std::mutex binder_mutex_;

  // main_scene_ includes all drawable object for visualizing Tango device's
  // movement.
  Scene main_scene_;

  // Tango configration file, this object is for configuring Tango Service setup
  // before connect to service.
  TangoConfig tango_config_;

  // Updated indices from the 3D Reconstruction library. The grids for
  // each of these needs to be re-extracted.
  //
  // This data is protected by binder_mutex_.
  std::vector<GridIndex> updated_indices_binder_thread_;

  // Updated indices from the 3D Reconstruction library. The grids for
  // each of these nedes to be re-extracted.
  //
  // This data is not protected by a mutex, it is only accessed from the GL
  // thread.
  std::vector<GridIndex> updated_indices_gl_thread_;

  // Meshes from the 3D Reconstruction library.
  std::unordered_map<GridIndex, std::shared_ptr<SingleDynamicMesh>,
                     GridIndexHasher> meshes_;
};
}  // namespace mesh_builder

#endif  // CPP_MESH_BUILDER_EXAMPLE_MESH_BUILDER_MESH_BUILDER_APP_H_
