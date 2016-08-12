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
#include <android/asset_manager.h>

#include <unistd.h>
#include "tango-gl/texture.h"
#include "tango-gl/util.h"

namespace tango_gl {

static const int kMaxExponentiation = 12;

static int RoundUpPowerOfTwo(int w) {
  int start = 2;
  for (int i = 0; i <= kMaxExponentiation; ++i) {
    if (w <= start) {
      w = start;
      break;
    } else {
      start = start << 1;
    }
  }
  return w;
}

Texture::Texture(GLuint texture_id, GLenum texture_target) {
  texture_id_ = texture_id;
  texture_target_ = texture_target;
}

Texture::Texture(AAssetManager* mgr, const char* file_path) {
  AAsset* asset = AAssetManager_open(mgr, file_path, AASSET_MODE_STREAMING);
  if (asset == NULL) {
    LOGE("Error opening asset %s", file_path);
    return;
  }
  off_t length;
  off_t start;
  int fd = AAsset_openFileDescriptor(asset, &start, &length);
  lseek(fd, start, SEEK_CUR);
  FILE* file = fdopen(fd, "rb");
  if (!LoadFromPNG(file)) {
    LOGE("Texture initialing error");
  }
  fclose(file);
  AAsset_close(asset);
}

bool Texture::LoadFromPNG(FILE* file) {
  fseek(file, 8, SEEK_CUR);

  png_structp png_ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);

  png_init_io(png_ptr, file);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &width_, &height_, &bit_depth_, &color_type_,
               NULL, NULL, NULL);

  width_ = RoundUpPowerOfTwo(width_);
  height_ = RoundUpPowerOfTwo(height_);
  int row = width_ * (color_type_ == PNG_COLOR_TYPE_RGBA ? 4 : 3);
  char* byte_data = new char[row * height_];

  png_bytep* row_pointers = new png_bytep[height_];
  for (uint i = 0; i < height_; ++i) {
    row_pointers[i] = (png_bytep)(byte_data + i * row);
  }
  png_read_image(png_ptr, row_pointers);
  png_destroy_read_struct(&png_ptr, &info_ptr, 0);

  texture_target_ = GL_TEXTURE_2D;

  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  util::CheckGlError("glBindTexture");
  if (color_type_ == PNG_COLOR_TYPE_RGBA) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, byte_data);
  } else {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, byte_data);
  }

  util::CheckGlError("glTexImage2D");
  glBindTexture(GL_TEXTURE_2D, 0);

  delete[] row_pointers;
  delete[] byte_data;

  return true;
}

GLuint Texture::GetTextureID() const { return texture_id_; }

GLuint Texture::GetTextureTarget() const { return texture_target_; }

}  // namespace tango_gl
