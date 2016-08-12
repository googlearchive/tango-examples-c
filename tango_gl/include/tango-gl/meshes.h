/**
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

#ifndef TANGO_GL_MESHES_H_
#define TANGO_GL_MESHES_H_

#include <string>

#include "tango-gl/tango-gl.h"

namespace tango_gl {
namespace meshes {
tango_gl::StaticMesh* MakeSphereMesh(int rows, int columns, double radius);
tango_gl::StaticMesh* MakeCubeMesh(double side);
tango_gl::StaticMesh* MakePlaneMesh(double width, double height);
tango_gl::StaticMesh* MakePlaneMesh(double width, double height,
                                    double tiling_factor);
}  // namespace meshes
}  // namespace tango_gl
#endif  // TANGO_GL_MESHES_H_
