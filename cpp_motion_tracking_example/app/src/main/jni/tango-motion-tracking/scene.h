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

#ifndef TANGO_MOTION_TRACKING_SCENE_H_
#define TANGO_MOTION_TRACKING_SCENE_H_

#include <jni.h>
#include <memory>

#include <tango_client_api.h>  // NOLINT
#include <tango-gl/gesture_camera.h>
#include <tango-gl/grid.h>
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
  void InitGLContent();

  // Release OpenGL resources allocated.
  void DeleteResources();

  // Setup GL view port.
  void SetupViewPort(int w, int h);

  // Render loop.
  void Render(const glm::vec3& position, const glm::quat& roatation);

 private:
  // Camera for rendering the scene.
  tango_gl::Camera* camera_;

  // Ground grid.
  tango_gl::Grid* grid_;
};
}  // namespace tango_motion_tracking

#endif  // TANGO_MOTION_TRACKING_SCENE_H_
