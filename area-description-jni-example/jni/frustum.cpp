#include "frustum.h"
#include "gl_util.h"

static const char kVertexShader[] =
    "attribute vec4 vertex;\n"
    "uniform mat4 mvp;\n"
    "void main() {\n"
    "  gl_Position = mvp*vertex;\n"
    "}\n";

static const char kFragmentShader[] =
    "void main() {\n"
    "  gl_FragColor = vec4(0,0,0,1);\n"
    "}\n";

static const float vertices[] = {
    0.0f, 0.0f, 0.0f,
    -0.4f, 0.3f, -0.5f,

    0.0f, 0.0f, 0.0f,
    0.4f, 0.3f, -0.5f,

    0.0f, 0.0f, 0.0f,
    -0.4f, -0.3f, -0.5f,

    0.0f, 0.0f, 0.0f,
    0.4f, -0.3f, -0.5f,

    -0.4f, 0.3f, -0.5f,
    0.4f, 0.3f, -0.5f,

    0.4f, 0.3f, -0.5f,
    0.4f, -0.3f, -0.5f,

    0.4f, -0.3f, -0.5f,
    -0.4f, -0.3f, -0.5f,

    -0.4f, -0.3f, -0.5f,
    -0.4f, 0.3f, -0.5f
};

Frustum::Frustum() {
  shader_program_ = GlUtil::CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program_) {
    LOGE("Could not create program.");
  }
  uniform_mvp_mat_ = glGetUniformLocation(shader_program_, "mvp");
  attrib_vertices_ = glGetAttribLocation(shader_program_, "vertex");

  glGenBuffers(1, &vertex_buffer_);
}

void Frustum::Render(glm::mat4 view_projection_mat) {
  glUseProgram(shader_program_);

  // Calculate MVP matrix and pass it to shader.
  glm::mat4 model_mat = GetCurrentModelMatrix();
  glm::mat4 mvp_mat = view_projection_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  // Vertice binding
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 8, vertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDrawArrays(GL_LINES, 0, 6 * 8);
  glUseProgram(0);
}
