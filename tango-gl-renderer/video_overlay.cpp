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

#include "video_overlay.h"

static const char kVertexShader[] =
"precision highp float;\n"
"precision highp int;\n"
"attribute vec4 vertex;\n"
"attribute vec2 textureCoords;\n"
"varying vec2 f_textureCoords;\n"
"void main() {\n"
"  f_textureCoords = textureCoords;\n"
"  gl_Position = vertex;\n"
"}\n";

static const char kFragmentShader[] =
"#extension GL_OES_EGL_image_external : require\n"
"precision highp float;\n"
"precision highp int;\n"
"uniform samplerExternalOES texture;\n"
"varying vec2 f_textureCoords;\n"
"void main() {\n"
    "  float k1 = 0.220545;                                  \n"
    "  float k2 =-0.634778;                               \n"
    "  float k3 = 0.603205;                               \n"
    "  vec2 cxy = vec2(639.358/1280.0, 355.708/720.0);\n"
    "  vec2 xy = f_textureCoords - cxy;\n"
    "  float r2 = xy[0] * xy[0] + xy[1] * xy[1];\n"
    "  float r4 = r2 * r2;\n"
    "  float r6 = r2 * r2 * r2;\n"
    "  vec2 delta_px = vec2( (xy[0] * 640.0) * (k1 * r2 + k2 * r4 + k3 * r6),\n"
    "                        (xy[1] * 360.0) * (k1 * r2 + k2 * r4 + k3 * r6));\n"
    "  vec2 delta = vec2(delta_px[0]/1280.0, delta_px[1]/720.0);\n"
"  gl_FragColor = texture2D(texture, f_textureCoords+delta);\n"
"}\n";

static const GLfloat kVertices[] =
{ -1.0, 1.0, 0.0,
  -1.0, -1.0, 0.0,
  1.0, 1.0, 0.0,
  1.0, -1.0, 0.0 };

static const GLushort kIndices[] =
{ 0, 1, 2, 2, 1, 3 };

static const GLfloat kTextureCoords[] =
{ 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0 };

VideoOverlay::VideoOverlay() {
  glEnable (GL_VERTEX_PROGRAM_POINT_SIZE);
  shader_program_ = GlUtil::CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program_) {
    LOGE("Could not create program.");
  }

  glEnable (GL_TEXTURE_EXTERNAL_OES);
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  uniform_texture = glGetUniformLocation(shader_program_, "texture");
  glUniform1i(uniform_texture, texture_id);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);

  glGenBuffers(3, vertex_buffers);
  // Allocate vertices buffer.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, kVertices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Allocate triangle indices buffer.
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffers[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6, kIndices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Allocate texture coordinates buufer.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * 4, kTextureCoords,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Assign the vertices attribute data.
  attrib_vertices_ = glGetAttribLocation(shader_program_, "vertex");
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[0]);
  glEnableVertexAttribArray (attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Assign the texture coordinates attribute data.
  attrib_textureCoords = glGetAttribLocation(shader_program_, "textureCoords");
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[2]);
  glEnableVertexAttribArray (attrib_textureCoords);
  glVertexAttribPointer(attrib_textureCoords, 2, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

  // Bind vertices buffer.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[0]);
  glEnableVertexAttribArray (attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Bind texture coordinates buffer.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[2]);
  glEnableVertexAttribArray (attrib_textureCoords);
  glVertexAttribPointer(attrib_textureCoords, 2, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Bind element array buffer.
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffers[1]);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
  GlUtil::CheckGlError("glDrawElements");
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glUseProgram(0);
  GlUtil::CheckGlError("glUseProgram()");
}
