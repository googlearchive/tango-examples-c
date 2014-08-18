#include "trace.h"

static const char kVertexShader[] = "attribute vec4 vertex;\n"
    "uniform mat4 mvp;\n"
    "void main() {\n"
    "  gl_Position = mvp*vertex;\n"
    "}\n";

static const char kFragmentShader[] = "void main() {\n"
    "  gl_FragColor = vec4(0,0,0,1);\n"
    "}\n";

static const int kMaxTraceLength = 1000;

Trace::Trace() {
  shader_program_ = GlUtil::CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program_) {
    LOGE("Could not create program.");
  }
  uniform_mvp_mat_ = glGetUniformLocation(shader_program_, "mvp");
  attrib_vertices_ = glGetAttribLocation(shader_program_, "vertex");

  vertices_.reserve(kMaxTraceLength);
}

void Trace::UpdateVertexArray(glm::vec3 v) {
  vertices_.push_back(v);
}

void Trace::Render(glm::mat4 view_projection_mat) {
  glUseProgram(shader_program_);

  glm::mat4 model_mat = GetCurrentModelMatrix();
  glm::mat4 mvp_mat = view_projection_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE,
                        sizeof(glm::vec3), &vertices_[0]);

  glDrawArrays(GL_LINE_STRIP, 0, vertices_.size());
  glUseProgram(0);
}
