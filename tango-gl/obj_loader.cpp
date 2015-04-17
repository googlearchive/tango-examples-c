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
#include <string>

#include "tango-gl/obj_loader.h"

namespace tango_gl {
bool obj_loader::LoadOBJData(const char *path, std::vector<GLfloat>& vertices,
                    std::vector<GLushort>& indices, std::vector<GLfloat>& normals) {
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    LOGE("Failed to open file: %s", path);
    return false;
  }

  while (1) {
    char lineHeader[128];
    int res = fscanf(file, "%s", lineHeader);
    if (res == EOF) break;
    if (strcmp(lineHeader, "v") == 0) {
      GLfloat vertex[3];
      int matches =
          fscanf(file, "%f %f %f\n", &vertex[0], &vertex[1], &vertex[2]);
      if (matches != 3) {
        LOGE("Format of 'v float float float' required for each vertice line");
        return false;
      }
      vertices.push_back(vertex[0]);
      vertices.push_back(vertex[1]);
      vertices.push_back(vertex[2]);
    } else if (strcmp(lineHeader, "f") == 0) {
      GLushort vertexIndex[3];
      int matches = fscanf(file, "%hu %hu %hu\n", &vertexIndex[0], &vertexIndex[1],
                           &vertexIndex[2]);
      if (matches != 3) {
        LOGE("Format of 'f int int int' required for each face line");
        return false;
      }
      indices.push_back(vertexIndex[0] - 1);
      indices.push_back(vertexIndex[1] - 1);
      indices.push_back(vertexIndex[2] - 1);
    } else if (strcmp(lineHeader, "vn") == 0) {
      GLfloat normal[3];
      int matches =
          fscanf(file, "%f %f %f\n", &normal[0], &normal[1], &normal[2]);
      if (matches != 3) {
        LOGE("Format of 'vn float float float' required for each normal line");
        return false;
      }
      normals.push_back(normal[0]);
      normals.push_back(normal[1]);
      normals.push_back(normal[2]);
    }
    else {
      char comments_buffer[1000];
      fgets(comments_buffer, 1000, file);
    }
  }

  fclose(file);
  return true;
}

}  // namespace tango_gl
