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
#include "tango-gl/shaders.h"

namespace tango_gl {
namespace shaders {
std::string GetBasicVertexShader() {
  return "precision mediump float;\n"
         "precision mediump int;\n"
         "attribute vec4 vertex;\n"
         "uniform mat4 mvp;\n"
         "uniform vec4 color;\n"
         "varying vec4 v_color;\n"
         "void main() {\n"
         "  gl_Position = mvp*vertex;\n"
         "  v_color = color;\n"
         "}\n";
}

std::string GetBasicFragmentShader() {
  return "precision mediump float;\n"
         "varying vec4 v_color;\n"
         "void main() {\n"
         "  gl_FragColor = v_color;\n"
         "}\n";
}

std::string GetColorVertexShader() {
  return "precision mediump float;\n"
         "precision mediump int;\n"
         "attribute vec4 vertex;\n"
         "attribute vec4 color;\n"
         "uniform mat4 mvp;\n"
         "varying vec4 v_color;\n"
         "void main() {\n"
         "  gl_Position = mvp*vertex;\n"
         "  v_color = color;\n"
         "}\n";
}

std::string GetVideoOverlayVertexShader() {
  return "precision highp float;\n"
         "precision highp int;\n"
         "attribute vec4 vertex;\n"
         "attribute vec2 textureCoords;\n"
         "varying vec2 f_textureCoords;\n"
         "uniform mat4 mvp;\n"
         "void main() {\n"
         "  f_textureCoords = textureCoords;\n"
         "  gl_Position = mvp * vertex;\n"
         "}\n";
}

std::string GetVideoOverlayFragmentShader() {
  return "#extension GL_OES_EGL_image_external : require\n"
         "precision highp float;\n"
         "precision highp int;\n"
         "uniform samplerExternalOES texture;\n"
         "varying vec2 f_textureCoords;\n"
         "void main() {\n"
         "  gl_FragColor = texture2D(texture, f_textureCoords);\n"
         "}\n";
}

std::string GetShadedVertexShader() {
  return "attribute vec4 vertex;\n"
         "attribute vec3 normal;\n"
         "uniform mat4 mvp;\n"
         "uniform mat4 mv;\n"
         "uniform vec4 color;\n"
         "uniform vec3 lightPos;\n"
         "varying vec4 v_color;\n"
         "void main() {\n"
         "  vec3 mvVertex = vec3(mv * vertex);\n"
         "  vec3 mvNormal = vec3(mv * vec4(normal, 0.0));\n"
         "  float distance = length(lightPos-mvVertex);\n"
         "  vec3 lightVec = normalize(lightPos-mvVertex);\n"
         "  float diffuse = max(dot(mvNormal, lightVec), 0.0);\n"
         "  diffuse = diffuse * (1.0/(1.0 + (0.4 * distance * distance)));\n"
         "  v_color = color * diffuse;\n"
         "  gl_Position = mvp*vertex;\n"
         "}\n";
}
}  // namespace shaders
}  // namespace tango_gl
