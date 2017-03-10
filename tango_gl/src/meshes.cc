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
#include "tango-gl/meshes.h"

namespace tango_gl {
namespace meshes {
tango_gl::StaticMesh* MakePlaneMesh(double width, double height) {
  return MakePlaneMesh(width, height, 1);
}
tango_gl::StaticMesh* MakePlaneMesh(double width, double height,
                                    double tiling_factor) {
  tango_gl::StaticMesh* mesh = new tango_gl::StaticMesh();

  mesh->vertices = std::vector<glm::vec3>(4);
  mesh->vertices[0] = glm::vec3(-width / 2, height / 2, 0.0f);
  mesh->vertices[1] = glm::vec3(width / 2, height / 2, 0.0f);
  mesh->vertices[2] = glm::vec3(-width / 2, -height / 2, 0.0f);
  mesh->vertices[3] = glm::vec3(width / 2, -height / 2, 0.0f);

  mesh->uv = std::vector<glm::vec2>(4);
  mesh->uv[0] = glm::vec2(0.0, 0.0);
  mesh->uv[1] = glm::vec2(tiling_factor, 0.0);
  mesh->uv[2] = glm::vec2(0.0, tiling_factor);
  mesh->uv[3] = glm::vec2(tiling_factor, tiling_factor);

  static const GLushort indices[] = {0, 2, 1, 3};
  mesh->indices = std::vector<GLuint>(
      indices, indices + sizeof(indices) / sizeof(indices[0]));

  mesh->render_mode = GL_TRIANGLE_STRIP;

  return mesh;
}
tango_gl::StaticMesh* MakeCubeMesh(double side) {
  tango_gl::StaticMesh* mesh = new tango_gl::StaticMesh();

  glm::vec3 tlf = glm::vec3(-side / 2, side / 2, side / 2);
  glm::vec3 trf = glm::vec3(side / 2, side / 2, side / 2);
  glm::vec3 blf = glm::vec3(-side / 2, -side / 2, side / 2);
  glm::vec3 brf = glm::vec3(side / 2, -side / 2, side / 2);
  glm::vec3 tlb = glm::vec3(-side / 2, side / 2, -side / 2);
  glm::vec3 trb = glm::vec3(side / 2, side / 2, -side / 2);
  glm::vec3 blb = glm::vec3(-side / 2, -side / 2, -side / 2);
  glm::vec3 brb = glm::vec3(side / 2, -side / 2, -side / 2);

  mesh->vertices = std::vector<glm::vec3>();
  // FRONT
  mesh->vertices.push_back(tlf);
  mesh->vertices.push_back(trf);
  mesh->vertices.push_back(blf);
  mesh->vertices.push_back(brf);
  // LEFT
  mesh->vertices.push_back(tlb);
  mesh->vertices.push_back(tlf);
  mesh->vertices.push_back(blb);
  mesh->vertices.push_back(blf);
  // RIGHT
  mesh->vertices.push_back(trf);
  mesh->vertices.push_back(trb);
  mesh->vertices.push_back(brf);
  mesh->vertices.push_back(brb);
  // TOP
  mesh->vertices.push_back(tlf);
  mesh->vertices.push_back(tlb);
  mesh->vertices.push_back(trf);
  mesh->vertices.push_back(trb);
  // BOTTOM
  mesh->vertices.push_back(blf);
  mesh->vertices.push_back(brf);
  mesh->vertices.push_back(blb);
  mesh->vertices.push_back(brb);
  // BACK
  mesh->vertices.push_back(trb);
  mesh->vertices.push_back(tlb);
  mesh->vertices.push_back(brb);
  mesh->vertices.push_back(blb);

  static const glm::vec2 uv[] = {
      // FRONT
      glm::vec2(0.0, 0.0), glm::vec2(1.0, 0.0),
      glm::vec2(0.0, 1.0), glm::vec2(1.0, 1.0),
      // LEFT
      glm::vec2(0.0, 0.0), glm::vec2(1.0, 0.0),
      glm::vec2(0.0, 1.0), glm::vec2(1.0, 1.0),
      // RIGHT
      glm::vec2(0.0, 0.0), glm::vec2(1.0, 0.0),
      glm::vec2(0.0, 1.0), glm::vec2(1.0, 1.0),
      // TOP
      glm::vec2(0.0, 0.0), glm::vec2(1.0, 0.0),
      glm::vec2(0.0, 1.0), glm::vec2(1.0, 1.0),
      // BOTTOM
      glm::vec2(0.0, 0.0), glm::vec2(1.0, 0.0),
      glm::vec2(0.0, 1.0), glm::vec2(1.0, 1.0),
      // BACK
      glm::vec2(0.0, 0.0), glm::vec2(1.0, 0.0),
      glm::vec2(0.0, 1.0), glm::vec2(1.0, 1.0), };
  mesh->uv = std::vector<glm::vec2>(uv, uv + sizeof(uv) / sizeof(uv[0]));

  static const GLuint indices[] = {
      0,  2,  1,  1,  2,  3,  4,  6,  5,  5,  6,  7,  8,  10, 9,  9,  10, 11,
      12, 14, 13, 13, 14, 15, 16, 18, 17, 17, 18, 19, 20, 22, 21, 21, 22, 23, };
  mesh->indices = std::vector<GLuint>(
      indices, indices + sizeof(indices) / sizeof(indices[0]));

  mesh->render_mode = GL_TRIANGLES;

  return mesh;
}
tango_gl::StaticMesh* MakeSphereMesh(int rows, int columns, double radius) {
  tango_gl::StaticMesh* mesh = new tango_gl::StaticMesh();

  // Generate position grid.
  mesh->vertices = std::vector<glm::vec3>(rows * columns);
  mesh->normals = std::vector<glm::vec3>(rows * columns);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      float theta = i * M_PI / (rows - 1);
      float phi = j * 2 * M_PI / (columns - 1);
      float x = radius * sin(theta) * cos(phi);
      float y = radius * cos(theta);
      float z = -(radius * sin(theta) * sin(phi));
      int index = i * columns + j;
      mesh->vertices[index] = glm::vec3(x, y, z);
      mesh->normals[index] = glm::vec3(x, y, z);
    }
  }

  // Create texture UVs
  mesh->uv = std::vector<glm::vec2>(rows * columns);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      int index = i * columns + j;
      mesh->uv[index].x =
          static_cast<float>(j) / static_cast<float>(columns - 1);
      mesh->uv[index].y = static_cast<float>(i) / static_cast<float>(rows - 1);
    }
  }

  // Create indices.
  int numIndices = 2 * (rows - 1) * columns;
  mesh->indices = std::vector<GLuint>(numIndices);
  int index = 0;
  for (int i = 0; i < rows - 1; i++) {
    if ((i & 1) == 0) {
      for (int j = 0; j < columns; j++) {
        mesh->indices[index] = i * columns + j;
        index++;
        mesh->indices[index] = (i + 1) * columns + j;
        index++;
      }
    } else {
      for (int j = columns - 1; j >= 0; j--) {
        mesh->indices[index] = (i + 1) * columns + j;
        index++;
        mesh->indices[index] = i * columns + j;
        index++;
      }
    }
  }

  mesh->render_mode = GL_TRIANGLE_STRIP;

  return mesh;
}

}  // namespace meshes
}  // namespace tango_gl
