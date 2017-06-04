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

#ifndef CPP_MESH_BUILDER_EXAMPLE_MESH_BUILDER_SCENE_H_
#define CPP_MESH_BUILDER_EXAMPLE_MESH_BUILDER_SCENE_H_

#include <memory>
#include <vector>

#include <tango_client_api.h>  // NOLINT
#include <tango-gl/gesture_camera.h>
#include <tango-gl/grid.h>
#include <tango-gl/tango-gl.h>
#include <tango-gl/util.h>

namespace mesh_builder {

// Scene provides OpenGL drawable objects and renders them for visualization.
class Scene {
 public:
  // Constructor and destructor.
  //
  // Scene will need a reference to pose_data_ instance to get the device motion
  // to render the camera frustum.
  Scene();
  ~Scene();

  // Allocate OpenGL resources for rendering.
  void InitGLContent();

  // Release OpenGL resources allocated.
  void DeleteResources();

  // Setup GL view port.
  void SetupViewPort(int w, int h);

  // Render loop.
  void Render();

  // Add a single dynamic mesh to the scene.
  void AddDynamicMesh(tango_gl::StaticMesh* mesh);

  // Remove all dynamic meshes from the scene.
  void ClearDynamicMeshes();

  // Camera for rendering the scene.
  tango_gl::Camera* camera_;

  // Dynamic meshes to draw.
  std::vector<tango_gl::StaticMesh*> dynamic_meshes_;
  tango_gl::Material* dynamic_mesh_material_;
};
}  // namespace mesh_builder

#endif  // CPP_MESH_BUILDER_EXAMPLE_MESH_BUILDER_SCENE_H_
