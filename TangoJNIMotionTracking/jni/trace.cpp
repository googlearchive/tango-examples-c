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
  shader_program = GlUtil::CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program) {
    LOGE("Could not create program.");
  }
  uniform_mvp_mat = glGetUniformLocation(shader_program, "mvp");
  attrib_vertices = glGetAttribLocation(shader_program, "vertex");

  vertices.reserve(kMaxTraceLength);
}

void Trace::UpdateVertexArray(glm::vec3 v) {
  vertices.push_back(v);
}

void Trace::Render(glm::mat4 view_projection_mat) {
  glUseProgram(shader_program);

  glm::mat4 model_mat = GetCurrentModelMatrix();
  glm::mat4 mvp_mat = view_projection_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  glEnableVertexAttribArray(attrib_vertices);
  glVertexAttribPointer(attrib_vertices, 3, GL_FLOAT, GL_FALSE,
                        sizeof(glm::vec3), &vertices[0]);

  glDrawArrays(GL_LINE_STRIP, 0, vertices.size());
  glUseProgram(0);
}
