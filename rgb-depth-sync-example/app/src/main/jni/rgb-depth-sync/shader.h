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

#ifndef RGB_DEPTH_SYNC_SHADER_H_
#define RGB_DEPTH_SYNC_SHADER_H_

namespace rgb_depth_sync {
namespace shader {

// Vertex shader for rendering a color camera texture on full screen.
static const char kColorCameraVert[] =
    "precision highp float;\n"
    "precision highp int;\n"
    "attribute vec4 vertex;\n"
    "attribute vec2 textureCoords;\n"
    "varying vec2 f_textureCoords;\n"
    "void main() {\n"
    "  f_textureCoords = textureCoords;\n"
    "  gl_Position =  vertex;\n"
    "}\n";

// Fragment shader for rendering a color texture on full screen with half alpha
// blending, please note that the color camera texture is samplerExternalOES.
static const char kColorCameraFrag[] =
    "#extension GL_OES_EGL_image_external : require\n"
    "precision highp float;\n"
    "precision highp int;\n"
    "uniform float blendAlpha;\n"
    "uniform samplerExternalOES colorTexture;\n"
    "uniform sampler2D depthTexture;\n"
    "varying vec2 f_textureCoords;\n"
    "void main() {\n"
    "  vec4 cColor = texture2D(colorTexture, f_textureCoords);\n"
    "  vec4 cDepth = texture2D(depthTexture, f_textureCoords);\n"
    "  gl_FragColor = (1.0-blendAlpha) * cColor + blendAlpha * cDepth;;\n"
    "}\n";

}  // namespace shader
}  // namespace rgb_depth_sync

#endif  // RGB_DEPTH_SYNC_SHADER_H_
