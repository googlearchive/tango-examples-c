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

#include <tango-gl/conversions.h>
#include <tango-gl/util.h>
#include <tango_support.h>

#include "mesh_builder/mesh_builder_app.h"

namespace {
const int kInitialVertexCount = 100;
const int kInitialIndexCount = 99;
const float kGrowthFactor = 1.5;
// The minimum Tango Core version required from this application.
constexpr int kTangoCoreMinimumVersion = 9377;

// The maximum amount of ms to spend meshing.
constexpr int kMeshingBudgetMs = 10;

// This function routes onPointCloudAvailable callbacks to the application
// object for handling.
//
// @param context, context will be a pointer to a MeshBuilderApp
//        instance on which to call callbacks.
// @param point_cloud, point cloud data to route to onPointCloudAvailable
// function.
void onPointCloudAvailableRouter(void* context,
                                 const TangoPointCloud* point_cloud) {
  mesh_builder::MeshBuilderApp* app =
      static_cast<mesh_builder::MeshBuilderApp*>(context);
  app->onPointCloudAvailable(point_cloud);
}

// This function routes onFrameAvailable callbacks to the application object for
// handling.
//
// @param context, context will be a pointer to a MeshBuilderApp
//        instance on which to call callbacks.
// @param id, camera id to route to onFrameAvailable function.
// @param buffer, image data to route to onFrameAvailable function.
void onFrameAvailableRouter(void* context, TangoCameraId id,
                            const TangoImageBuffer* buffer) {
  mesh_builder::MeshBuilderApp* app =
      static_cast<mesh_builder::MeshBuilderApp*>(context);
  app->onFrameAvailable(id, buffer);
}

void extract3DRPose(const glm::mat4& mat, Tango3DR_Pose* pose) {
  glm::vec3 translation;
  glm::quat rotation;
  glm::vec3 scale;
  tango_gl::util::DecomposeMatrix(mat, &translation, &rotation, &scale);
  pose->translation[0] = translation[0];
  pose->translation[1] = translation[1];
  pose->translation[2] = translation[2];
  pose->orientation[0] = rotation[0];
  pose->orientation[1] = rotation[1];
  pose->orientation[2] = rotation[2];
  pose->orientation[3] = rotation[3];
}

}  // namespace

namespace mesh_builder {

bool GridIndex::operator==(const GridIndex& other) const {
  return indices[0] == other.indices[0] && indices[1] == other.indices[1] &&
         indices[2] == other.indices[2];
}

void MeshBuilderApp::onPointCloudAvailable(const TangoPointCloud* point_cloud) {
  // Be careful not to do too much processing in this callback.  If
  // you do a lot of processing, you will get disconnected from the
  // Tango service.
  //
  // Here we copy the point cloud and cache the matrix from the call.
  if (!t3dr_is_running_) {
    return;
  }

  // Get the depth transform to OpenGL world frame at the timestamp of the point
  // cloud.
  TangoSupport_MatrixTransformData matrix_transform;
  TangoSupport_getMatrixTransformAtTime(
      point_cloud->timestamp, TANGO_COORDINATE_FRAME_START_OF_SERVICE,
      TANGO_COORDINATE_FRAME_CAMERA_DEPTH, TANGO_SUPPORT_ENGINE_OPENGL,
      TANGO_SUPPORT_ENGINE_TANGO, TANGO_SUPPORT_ROTATION_IGNORED,
      &matrix_transform);
  if (matrix_transform.status_code != TANGO_POSE_VALID) {
    LOGE(
        "MeshBuilderExample: Could not find a valid matrix transform at "
        "time %lf for the depth camera.",
        point_cloud->timestamp);
    return;
  }

  std::lock_guard<std::mutex> lock(binder_mutex_);
  point_cloud_matrix_ = glm::make_mat4(matrix_transform.matrix);
  TangoSupport_updatePointCloud(point_cloud_manager_, point_cloud);
  point_cloud_available_ = true;
}

void MeshBuilderApp::onFrameAvailable(TangoCameraId id,
                                      const TangoImageBuffer* buffer) {
  // Be careful not to do too much processing in this callback.  If
  // you do a lot of processing, you will get disconnected from the
  // Tango service.
  //
  // Here we use the copied point cloud and update the 3D
  // Reconstruction state.  However we do not extract meshes because
  // that can be very expensive at high resolution.
  if (id != TANGO_CAMERA_COLOR || !t3dr_is_running_) {
    return;
  }

  std::lock_guard<std::mutex> lock(binder_mutex_);
  if (!point_cloud_available_) {
    return;
  }

  // Get the camera color transform to OpenGL world frame in OpenGL convention.
  TangoSupport_MatrixTransformData matrix_transform;
  TangoSupport_getMatrixTransformAtTime(
      buffer->timestamp, TANGO_COORDINATE_FRAME_START_OF_SERVICE,
      TANGO_COORDINATE_FRAME_CAMERA_COLOR, TANGO_SUPPORT_ENGINE_OPENGL,
      TANGO_SUPPORT_ENGINE_TANGO, TANGO_SUPPORT_ROTATION_IGNORED,
      &matrix_transform);
  if (matrix_transform.status_code != TANGO_POSE_VALID) {
    LOGE(
        "MeshBuilderExample: Could not find a valid matrix transform at "
        "time %lf for the color camera.",
        buffer->timestamp);
    return;
  }
  glm::mat4 image_matrix = glm::make_mat4(matrix_transform.matrix);

  Tango3DR_ImageBuffer t3dr_image;
  t3dr_image.width = buffer->width;
  t3dr_image.height = buffer->height;
  t3dr_image.stride = buffer->stride;
  t3dr_image.timestamp = buffer->timestamp;
  t3dr_image.format = static_cast<Tango3DR_ImageFormatType>(buffer->format);
  t3dr_image.data = buffer->data;

  Tango3DR_Pose t3dr_image_pose;
  extract3DRPose(image_matrix, &t3dr_image_pose);

  Tango3DR_PointCloud t3dr_depth;
  TangoSupport_getLatestPointCloud(point_cloud_manager_, &front_cloud_);
  t3dr_depth.timestamp = front_cloud_->timestamp;
  t3dr_depth.num_points = front_cloud_->num_points;
  t3dr_depth.points = reinterpret_cast<Tango3DR_Vector4*>(front_cloud_->points);

  Tango3DR_Pose t3dr_depth_pose;
  extract3DRPose(point_cloud_matrix_, &t3dr_depth_pose);

  Tango3DR_GridIndexArray t3dr_updated;
  Tango3DR_Status t3dr_err = Tango3DR_updateFromPointCloud(
      t3dr_context_, &t3dr_depth, &t3dr_depth_pose, &t3dr_image,
      &t3dr_image_pose, &t3dr_updated);
  if (t3dr_err != TANGO_3DR_SUCCESS) {
    LOGE("MeshBuilderApp: Tango3DR_update failed with error code %d", t3dr_err);
    return;
  }

  // It's more important to be responsive than to handle all indexes.
  // Replace the current list if we have fallen behind in processing.
  updated_indices_binder_thread_.resize(t3dr_updated.num_indices);
  std::copy(&t3dr_updated.indices[0][0],
            &t3dr_updated.indices[t3dr_updated.num_indices][0],
            reinterpret_cast<uint32_t*>(updated_indices_binder_thread_.data()));

  Tango3DR_GridIndexArray_destroy(&t3dr_updated);
  point_cloud_available_ = false;
}

MeshBuilderApp::MeshBuilderApp() {}

MeshBuilderApp::~MeshBuilderApp() {
  if (tango_config_ != nullptr) {
    TangoConfig_free(tango_config_);
    tango_config_ = nullptr;
  }
  if (point_cloud_manager_ != nullptr) {
    TangoSupport_freePointCloudManager(point_cloud_manager_);
    point_cloud_manager_ = nullptr;
  }
}

void MeshBuilderApp::ActivityCtor(bool t3dr_is_running) {
  t3dr_is_running_ = t3dr_is_running;
}

void MeshBuilderApp::OnCreate(JNIEnv* env, jobject activity) {
  // Check the installed version of the TangoCore.  If it is too old, then
  // it will not support the most up to date features.
  int version;
  TangoErrorType err = TangoSupport_getTangoVersion(env, activity, &version);
  if (err != TANGO_SUCCESS || version < kTangoCoreMinimumVersion) {
    LOGE("AugmentedRealityApp::OnCreate, Tango Core version is out of date.");
    std::exit(EXIT_SUCCESS);
  }
}

void MeshBuilderApp::OnTangoServiceConnected(JNIEnv* env, jobject binder) {
  TangoErrorType ret = TangoService_setBinder(env, binder);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MeshBuilderApp: Failed to bind Tango service with"
        "error code: %d",
        ret);
  }

  TangoSetupConfig();
  TangoConnectCallbacks();
  TangoConnect();
  TangoSetup3DR();
}

void MeshBuilderApp::TangoSetupConfig() {
  // Here, we'll configure the service to run in the way we'd want. For this
  // application, we'll start from the default configuration
  // (TANGO_CONFIG_DEFAULT).
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);

  // This enables basic motion tracking capabilities.
  if (tango_config_ == nullptr) {
    LOGE("MeshBuilderApp: Failed to get default config form");
    std::exit(EXIT_SUCCESS);
  }

  // Set auto-recovery for motion tracking as requested by the user.
  int ret =
      TangoConfig_setBool(tango_config_, "config_enable_auto_recovery", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MeshBuilderApp: config_enable_auto_recovery failed with error "
        "code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  // Enable depth.
  ret = TangoConfig_setBool(tango_config_, "config_enable_depth", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MeshBuilderApp: config_enable_depth failed with error "
        "code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  // Need to specify the depth_mode as XYZC.
  ret = TangoConfig_setInt32(tango_config_, "config_depth_mode",
                             TANGO_POINTCLOUD_XYZC);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "Failed to set 'depth_mode' configuration flag with error"
        " code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  // Enable color camera.
  ret = TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MeshBuilderApp: config_enable_color_camera failde with error "
        "code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  if (point_cloud_manager_ == nullptr) {
    int32_t max_point_cloud_elements;
    ret = TangoConfig_getInt32(tango_config_, "max_point_cloud_elements",
                               &max_point_cloud_elements);
    if (ret != TANGO_SUCCESS) {
      LOGE("Failed to query maximum number of point cloud elements.");
      std::exit(EXIT_SUCCESS);
    }

    ret = TangoSupport_createPointCloudManager(max_point_cloud_elements,
                                               &point_cloud_manager_);
    if (ret != TANGO_SUCCESS) {
      std::exit(EXIT_SUCCESS);
    }
  }
}

void MeshBuilderApp::TangoSetup3DR() {
  // Now that Tango is configured correctly, we also need to configure
  // 3D Reconstruction the way we want.
  Tango3DR_Config t3dr_config =
      Tango3DR_Config_create(TANGO_3DR_CONFIG_RECONSTRUCTION);
  Tango3DR_Status t3dr_err;
  t3dr_err = Tango3DR_Config_setDouble(t3dr_config, "resolution", 0.05);
  if (t3dr_err != TANGO_3DR_SUCCESS) {
    LOGE("MeshBuilderApp: 3dr resolution failed with error code: %d", t3dr_err);
    std::exit(EXIT_SUCCESS);
  }

  t3dr_err = Tango3DR_Config_setBool(t3dr_config, "generate_color", true);
  if (t3dr_err != TANGO_3DR_SUCCESS) {
    LOGE("MeshBuilderApp: 3dr generate_color failed with error code: %d",
         t3dr_err);
    std::exit(EXIT_SUCCESS);
  }

  t3dr_context_ = Tango3DR_ReconstructionContext_create(t3dr_config);
  if (t3dr_context_ == nullptr) {
    LOGE("MeshBuilderApp: Unable to create 3DR context.");
    std::exit(EXIT_SUCCESS);
  }

  // Configure the color intrinsics to be used with updates to the mesh.
  t3dr_err = Tango3DR_ReconstructionContext_setColorCalibration(
      t3dr_context_, &t3dr_intrinsics_);
  if (t3dr_context_ == nullptr) {
    LOGE("MeshBuilderApp: Unable to set color calibration.");
    std::exit(EXIT_SUCCESS);
  }

  Tango3DR_Config_destroy(t3dr_config);
}

void MeshBuilderApp::TangoConnectCallbacks() {
  // Attach onPointCloudAvailable callback.
  // The callback will be called after the service is connected.
  TangoErrorType ret =
      TangoService_connectOnPointCloudAvailable(onPointCloudAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MeshBuilderApp: Failed to connect to xyzij callback with error "
        "code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }

  // Attach onFrameAvailable callback.
  // The callback will be called after the service is connected.
  ret = TangoService_connectOnFrameAvailable(TANGO_CAMERA_COLOR, this,
                                             onFrameAvailableRouter);
  if (ret != TANGO_SUCCESS) {
    LOGE(
        "MeshBuilderApp: Faild to connect to frame callback with error "
        "code: %d",
        ret);
    std::exit(EXIT_SUCCESS);
  }
}

// Connect to Tango Service, service will start running, and
// pose can be queried.
void MeshBuilderApp::TangoConnect() {
  TangoErrorType err = TangoService_connect(this, tango_config_);
  if (err != TANGO_SUCCESS) {
    LOGE(
        "MeshBuilderApp: Failed to connect to the Tango service with"
        "error code: %d",
        err);
    std::exit(EXIT_SUCCESS);
  }

  // Initialize TangoSupport context.
  TangoSupport_initialize(TangoService_getPoseAtTime,
                          TangoService_getCameraIntrinsics);

  // Update the camera intrinsics too.
  TangoCameraIntrinsics intrinsics;
  err = TangoService_getCameraIntrinsics(TANGO_CAMERA_COLOR, &intrinsics);
  if (err != TANGO_SUCCESS) {
    LOGE(
        "MeshBuilderApp: Failed to get camera intrinsics with error "
        "code: %d",
        err);
    std::exit(EXIT_SUCCESS);
  }
  t3dr_intrinsics_.calibration_type =
      static_cast<Tango3DR_TangoCalibrationType>(intrinsics.calibration_type);
  t3dr_intrinsics_.width = intrinsics.width;
  t3dr_intrinsics_.height = intrinsics.height;
  t3dr_intrinsics_.fx = intrinsics.fx;
  t3dr_intrinsics_.fy = intrinsics.fy;
  t3dr_intrinsics_.cx = intrinsics.cx;
  t3dr_intrinsics_.cy = intrinsics.cy;
  std::copy(std::begin(intrinsics.distortion), std::end(intrinsics.distortion),
            std::begin(t3dr_intrinsics_.distortion));
}

void MeshBuilderApp::TangoDisconnect() {
  // When disconnecting from the Tango Service, it is important to make sure to
  // free your configuration object. Note that disconnecting from the service,
  // resets all configuration, and disconnects all callbacks. If an application
  // resumes after disconnecting, it must re-register configuration and
  // callbacks with the service.
  TangoConfig_free(tango_config_);
  tango_config_ = nullptr;
  TangoService_disconnect();
}

void MeshBuilderApp::OnPause() {
  TangoDisconnect();
  DeleteResources();

  // Since motion tracking is lost when disconnected from Tango, any
  // existing 3D reconstruction state no longer is lined up with the
  // real world. Best we can do is clear the state.
  OnClearButtonClicked();
}

void MeshBuilderApp::OnSurfaceCreated() { main_scene_.InitGLContent(); }

void MeshBuilderApp::OnSurfaceChanged(int width, int height) {
  main_scene_.SetupViewPort(width, height);
}

void MeshBuilderApp::OnDrawFrame() {
  // Get the most up to date data from the binder thread. It's more
  // important to be responsive than to handle all indexes.  Replace
  // the current list if we have fallen behind in processing.
  {
    std::lock_guard<std::mutex> lock(binder_mutex_);
    swap(updated_indices_binder_thread_, updated_indices_gl_thread_);
    updated_indices_binder_thread_.clear();
  }

  // Get the last device transform to start of service frame in OpenGL
  // convention.
  TangoSupport_MatrixTransformData matrix_transform;
  TangoSupport_getMatrixTransformAtTime(
      0, TANGO_COORDINATE_FRAME_START_OF_SERVICE, TANGO_COORDINATE_FRAME_DEVICE,
      TANGO_SUPPORT_ENGINE_OPENGL, TANGO_SUPPORT_ENGINE_OPENGL,
      TANGO_SUPPORT_ROTATION_IGNORED, &matrix_transform);
  if (matrix_transform.status_code == TANGO_POSE_VALID) {
    start_service_T_device_ = glm::make_mat4(matrix_transform.matrix);
  } else {
    LOGE(
        "MeshBuilderExample: Could not find a valid matrix transform at "
        "current time for the device.");
  }

  std::vector<GridIndex> needs_resize;
  auto start_time = std::chrono::high_resolution_clock::now();
  unsigned int it;
  for (it = 0; it < updated_indices_gl_thread_.size(); ++it) {
    auto updated_index = updated_indices_gl_thread_[it];

    std::shared_ptr<SingleDynamicMesh>& dynamic_mesh = meshes_[updated_index];
    if (dynamic_mesh == nullptr) {
      // Build a dynamic mesh and add it to the scene.
      dynamic_mesh = std::make_shared<SingleDynamicMesh>();
      dynamic_mesh->mesh.render_mode = GL_TRIANGLES;
      dynamic_mesh->mesh.vertices.reserve(kInitialVertexCount);
      dynamic_mesh->mesh.colors.reserve(kInitialVertexCount);
      dynamic_mesh->mesh.indices.reserve(kInitialIndexCount);

      main_scene_.AddDynamicMesh(&dynamic_mesh->mesh);
    }

    // Last frame the mesh needed more space.  Give it room now.
    if (dynamic_mesh->needs_to_grow) {
      int new_vertex_size =
          dynamic_mesh->mesh.vertices.capacity() * kGrowthFactor;
      int new_index_size =
          dynamic_mesh->mesh.indices.capacity() * kGrowthFactor;
      new_index_size -= new_index_size % 3;
      dynamic_mesh->mesh.vertices.reserve(new_vertex_size);
      dynamic_mesh->mesh.colors.reserve(new_vertex_size);
      dynamic_mesh->mesh.indices.reserve(new_index_size);
      dynamic_mesh->needs_to_grow = false;
    }

    // Make sure we have enough room in the mesh for the maximum
    // amount of data potentially returned.
    //
    // We must do this call to resize() before the data gets filled
    // in because when resize() grows the array size it fills in all
    // the new elements.
    dynamic_mesh->mesh.vertices.resize(dynamic_mesh->mesh.vertices.capacity());
    dynamic_mesh->mesh.colors.resize(dynamic_mesh->mesh.colors.capacity());
    dynamic_mesh->mesh.indices.resize(dynamic_mesh->mesh.indices.capacity());

    Tango3DR_Mesh tango_mesh = {
        /* timestamp */ 0.0,
        /* num_vertices */ 0u,
        /* num_faces */ 0u,
        /* num_textures */ 0u,
        /* max_num_vertices */ static_cast<uint32_t>(
            dynamic_mesh->mesh.vertices.capacity()),
        /* max_num_faces */ static_cast<uint32_t>(
            dynamic_mesh->mesh.indices.capacity() / 3),
        /* max_num_textures */ 0u,
        /* vertices */ reinterpret_cast<Tango3DR_Vector3*>(
            dynamic_mesh->mesh.vertices.data()),
        /* faces */ reinterpret_cast<Tango3DR_Face*>(
            dynamic_mesh->mesh.indices.data()),
        /* normals */ nullptr,
        /* colors */ reinterpret_cast<Tango3DR_Color*>(
            dynamic_mesh->mesh.colors.data()),
        /* texture_coords */ nullptr,
        /*texture_ids */ nullptr,
        /* textures */ nullptr};

    Tango3DR_Status err = Tango3DR_extractPreallocatedMeshSegment(
        t3dr_context_, updated_index.indices, &tango_mesh);
    if (err == TANGO_3DR_ERROR) {
      LOGE("extractPreallocatedMeshSegment failed with error code: %d", err);
      continue;
    } else if (err == TANGO_3DR_INSUFFICIENT_SPACE) {
      LOGI(
          "extractPreallocatedMeshSegment ran out of space with room for "
          "%zu vertices, %zu indices.",
          dynamic_mesh->mesh.vertices.capacity(),
          dynamic_mesh->mesh.indices.capacity());
      dynamic_mesh->needs_to_grow = true;
      needs_resize.push_back(updated_index);
    }
    dynamic_mesh->mesh.vertices.resize(tango_mesh.num_vertices);
    dynamic_mesh->mesh.colors.resize(tango_mesh.num_vertices);
    dynamic_mesh->mesh.indices.resize(tango_mesh.num_faces * 3);

    // If we are out of time, stop meshing this batch.
    auto now = std::chrono::high_resolution_clock::now();
    auto delta_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
    if (delta_ms.count() >= kMeshingBudgetMs) {
      break;
    }
  }

  // Any leftover grid indices also need to get processed next frame.
  while (it < updated_indices_gl_thread_.size()) {
    needs_resize.push_back(updated_indices_gl_thread_[it]);
    ++it;
  }

  swap(needs_resize, updated_indices_gl_thread_);

  main_scene_.camera_->SetTransformationMatrix(start_service_T_device_);
  main_scene_.Render();
}

void MeshBuilderApp::DeleteResources() { main_scene_.DeleteResources(); }

void MeshBuilderApp::OnToggleButtonClicked(bool t3dr_is_running) {
  std::lock_guard<std::mutex> lock(binder_mutex_);
  t3dr_is_running_ = t3dr_is_running;
}

void MeshBuilderApp::OnClearButtonClicked() {
  Tango3DR_clear(t3dr_context_);
  meshes_.clear();
  main_scene_.ClearDynamicMeshes();
}

}  // namespace mesh_builder
