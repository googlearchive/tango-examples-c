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

#ifndef CPP_MOTION_TRACKING_EXAMPLE_TANGO_MOTION_TRACKING_SCENE_H_
#define CPP_MOTION_TRACKING_EXAMPLE_TANGO_MOTION_TRACKING_SCENE_H_

#include <jni.h>
#include <memory>

#include <android/asset_manager.h>
#include <tango_client_api.h>  // NOLINT
#include <tango-gl/gesture_camera.h>
#include <tango-gl/grid.h>
#include <tango-gl/texture.h>
#include <tango-gl/shaders.h>
#include <tango-gl/tango-gl.h>
#include <tango-gl/util.h>

namespace tango_motion_tracking {

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
  void InitGLContent(AAssetManager* aasset_manager);

  // Release OpenGL resources allocated.
  void DeleteResources();

  // Setup GL view port.
  void SetupViewPort(int w, int h);

  // Rotate the logo cube in proportion of the time elapsed.
  void RotateCubeByPose(const TangoPoseData& pose);

  // Render loop.
  void Render(const TangoPoseData& pose);

 private:
  // Camera for rendering the scene.
  tango_gl::Camera* camera_;

  // Floor components.
  tango_gl::StaticMesh* floor_mesh_;
  tango_gl::Material* floor_material_;
  tango_gl::Texture* grass_texture_;
  tango_gl::Transform floor_transform_;

  // Cube components.
  tango_gl::StaticMesh* cube_mesh_;
  tango_gl::Material* cube_material_;
  tango_gl::Texture* logo_texture_;
  tango_gl::Transform cube_transform_;

  // Last pose timestamp received
  double last_pose_timestamp_;
  double last_angle_;
};
}  // namespace tango_motion_tracking

#endif  // CPP_MOTION_TRACKING_EXAMPLE_TANGO_MOTION_TRACKING_SCENE_H_
