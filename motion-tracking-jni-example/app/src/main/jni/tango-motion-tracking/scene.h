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
#include <tango-gl/axis.h>
#include <tango-gl/camera.h>
#include <tango-gl/color.h>
#include <tango-gl/gesture_camera.h>
#include <tango-gl/grid.h>
#include <tango-gl/frustum.h>
#include <tango-gl/trace.h>
#include <tango-gl/transform.h>
#include <tango-gl/util.h>

#include <tango-motion-tracking/pose_data.h>

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
  void Render(const TangoPoseData& cur_pose);

  // Set render camera's viewing angle, first person, third person or top down.
  //
  // @param: camera_type, camera type includes first person, third person and
  //         top down
  void SetCameraType(tango_gl::GestureCamera::CameraType camera_type);

  // Touch event passed from android activity. This function only support two
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

 private:
  // Camera object that allows user to use touch input to interact with.
  tango_gl::GestureCamera* gesture_camera_;

  // Device axis (in device frame of reference).
  tango_gl::Axis* axis_;

  // Device frustum.
  tango_gl::Frustum* frustum_;

  // Ground grid.
  tango_gl::Grid* grid_;

  // Trace of pose data.
  tango_gl::Trace* trace_;
};
}  // namespace tango_motion_tracking

#endif  // TANGO_MOTION_TRACKING_SCENE_H_