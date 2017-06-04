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

#ifndef CPP_RGB_DEPTH_SYNC_EXAMPLE_RGB_DEPTH_SYNC_COLOR_IMAGE_H_
#define CPP_RGB_DEPTH_SYNC_EXAMPLE_RGB_DEPTH_SYNC_COLOR_IMAGE_H_

#include "tango-gl/util.h"

namespace rgb_depth_sync {
// ColorImage is container for color camera texture.
class ColorImage {
 public:
  ColorImage();
  ~ColorImage();
  GLuint GetTextureId() const { return texture_id_; }

  void InitializeGL();

 private:
  GLuint texture_id_;
};
}  // namespace rgb_depth_sync

#endif  // CPP_RGB_DEPTH_SYNC_EXAMPLE_RGB_DEPTH_SYNC_COLOR_IMAGE_H_
