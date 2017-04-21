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

#ifndef CPP_MARKER_DETECTION_EXAMPLE_TANGO_MARKER_DETECTION_MESH_OBJECT_H_
#define CPP_MARKER_DETECTION_EXAMPLE_TANGO_MARKER_DETECTION_MESH_OBJECT_H_

#include <android/asset_manager.h>
#include <tango-gl/transform.h>
#include <tango-gl/util.h>
#include <tango-gl/texture.h>
#include <tango-gl/meshes.h>
#include <tango-gl/shaders.h>
#include <tango-gl/camera.h>

namespace tango_marker_detection {

// 3D mesh object
class MeshObject {
 public:
  // Constructor and destructor.
  MeshObject();
  ~MeshObject();

  // Create a sphere mesh with a specified texture.
  // @param aasset_manager pointer to Android asset manager.
  // @param texture_file the name of the texture file in Android asset.
  // @param radius the radius of the sphere, in meters.
  void MakeSphere(AAssetManager* aasset_manager, const char* texture_file,
                  double radius);

  // Delete internal resources.
  void Delete();

  // Render the mesh object.
  // @param camera the camera object to setup the view matrix.
  void Render(tango_gl::Camera* camera);

  // Transform the mesh object with a given pose.
  // @param translation the new location of the mesh.
  // @param rotation a quaternion that represents the new rotation of the mesh.
  void Transform(const double* translation, const double* rotation);

  // Set visibility of the object.
  // @param visible the new visibility status.
  void SetVisible(bool visible);

 private:
  // Flag to indicate if the object has been initialized.
  bool initialized_;

  // Flag to indicate if the object is visible.
  bool visible_;

  // Mesh of the object.
  tango_gl::StaticMesh* mesh_;

  // Texture of the object.
  tango_gl::Texture* texture_;

  // Material of the object.
  tango_gl::Material* material_;

  // Transform of the object.
  tango_gl::Transform transform_;
};
}  // namespace tango_marker_detection

#endif  // CPP_MARKER_DETECTION_EXAMPLE_TANGO_MARKER_DETECTION_MESH_OBJECT_H_
