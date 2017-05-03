/*
 * Copyright 2017 Google Inc. All Rights Reserved.
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

#include <math.h>

#include <tango-gl/conversions.h>
#include <tango-gl/tango-gl.h>
#include <tango-gl/texture.h>
#include <tango-gl/shaders.h>
#include <tango-gl/meshes.h>

#include "tango-marker-detection/scene.h"

namespace {
void DecomposeMatrix(const glm::mat4& transformation_matrix,
                     double translation[3], double orientation[4]) {
  translation[0] = transformation_matrix[3][0];
  translation[1] = transformation_matrix[3][1];
  translation[2] = transformation_matrix[3][2];

  const glm::quat q = glm::quat_cast(transformation_matrix);
  orientation[0] = q.x;
  orientation[1] = q.y;
  orientation[2] = q.z;
  orientation[3] = q.w;
}

// Maximum marker Id to detect.
const int kMaxMarkerId = 16;

// Physical size of the Alvar marker in meter.
const double kMarkerEdgeLength = 0.1397;
}  // namespace

namespace tango_marker_detection {

Scene::Scene() : is_content_initialized_(false) {}

Scene::~Scene() {}

void Scene::InitGLContent(AAssetManager* aasset_manager) {
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // Allocating render camera and drawable object.
  video_overlay_ =
      std::unique_ptr<tango_gl::VideoOverlay>(new tango_gl::VideoOverlay());
  camera_ = std::unique_ptr<tango_gl::Camera>(new tango_gl::Camera());

  // Initialize objects with nullptr.
  {
    // Lock objects_ during initialization.
    std::lock_guard<std::mutex> lock(objects_mutex_);
    objects_.resize(kMaxMarkerId);
    for (int i = 0; i < kMaxMarkerId; ++i) {
      objects_[i] = std::unique_ptr<MeshObject>(new MeshObject());
      objects_[i]->MakeSphere(aasset_manager,
                              i % 2 == 0 ? "earth.png" : "moon.png",
                              kMarkerEdgeLength / 2);
    }
  }

  is_content_initialized_ = true;
}

void Scene::DeleteResources() {
  if (is_content_initialized_) {
    is_content_initialized_ = false;

    // Lock objects_ during deleting.
    std::lock_guard<std::mutex> lock(objects_mutex_);
    for (int i = 0; i < kMaxMarkerId; ++i) {
      objects_[i] = nullptr;
    }

    video_overlay_ = nullptr;
    camera_ = nullptr;
  }
}

void Scene::SetupViewport(int w, int h) {
  if (h <= 0 || w <= 0) {
    LOGE("Setup graphic height not valid");
    return;
  }

  viewport_width_ = w;
  viewport_height_ = h;
}

void Scene::SetupCamera(const glm::mat4& projection_matrix,
                        const glm::mat4& transformation_matrix) {
  if (camera_ != nullptr) {
    camera_->SetProjectionMatrix(projection_matrix);
    camera_->SetTransformationMatrix(transformation_matrix);
  }
}

void Scene::Clear() {
  glViewport(0, 0, viewport_width_, viewport_height_);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void Scene::Render() {
  if (!is_content_initialized_) {
    return;
  }

  glViewport(0, 0, viewport_width_, viewport_height_);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  // If it's first person view, we will render the video overlay in full
  // screen, so we passed identity matrix as view and projection matrix.
  glDisable(GL_DEPTH_TEST);
  video_overlay_->Render(glm::mat4(1.0f), glm::mat4(1.0f));
  glEnable(GL_DEPTH_TEST);

  // Render all objects.
  {
    // Lock objects_ during rendering.
    std::lock_guard<std::mutex> lock(objects_mutex_);
    for (int i = 0; i < kMaxMarkerId; ++i) {
      objects_[i]->Render(camera_.get());
    }
  }
}

void Scene::DetectMarkers(const TangoImageBuffer& image_buffer,
                          const glm::mat4& world_T_camera) {
  if (!is_content_initialized_) return;

  double translation[3];
  double orientation[4];
  DecomposeMatrix(world_T_camera, translation, orientation);

  TangoSupportMarkerParam param;
  param.type = TANGO_MARKER_ARTAG;
  param.marker_size = kMarkerEdgeLength;

  TangoSupportMarkerList list;
  if (TANGO_SUCCESS ==
      TangoSupport_detectMarkers(&image_buffer, TANGO_CAMERA_COLOR, translation,
                                 orientation, &param, &list)) {
    // Lock objects_ during updating.
    std::lock_guard<std::mutex> lock(objects_mutex_);

    for (int i = 0; i < list.marker_count; ++i) {
      int marker_id = atoi(list.markers[i].content);
      if (marker_id > 0) {
        int index = (marker_id - 1) % kMaxMarkerId;
        // Place the mesh object at the location just above the marker center
        // along the marker plane normal direction. We use the size of the mesh
        // object as the offset.
        double object_size = kMarkerEdgeLength / 2;
        glm::vec4 object_center_local = glm::vec4(0, 0, object_size, 1);

        // Convert the object center from local(marker) space to world space,
        // using the transformation formed by the marker pose.
        glm::mat4 world_T_local = tango_gl::conversions::TransformFromArrays(
            list.markers[i].translation, list.markers[i].orientation);

        glm::vec4 object_center_world = world_T_local * object_center_local;

        double translation[3] = {object_center_world.x, object_center_world.y,
                                 object_center_world.z};

        // Reposition the object to the newly calculated location and apply
        // the marker orientation as the rotation.
        objects_[index]->Transform(translation, list.markers[i].orientation);

        // Make sure the object is visible.
        objects_[index]->SetVisible(true);
      }
    }

    // Release memory allocated by TangoSupport_detectMarkers().
    TangoSupport_freeMarkerList(&list);
  }
}

void Scene::SetVideoOverlayRotation(int display_rotation) {
  if (is_content_initialized_) {
    video_overlay_->SetDisplayRotation(
        static_cast<TangoSupportRotation>(display_rotation));
  }
}

}  // namespace tango_marker_detection
