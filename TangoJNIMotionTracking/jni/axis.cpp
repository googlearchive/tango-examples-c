#include "axis.h"

static const char kVertexShader[] =
    "attribute vec4 vertex;\n"
    "attribute vec4 color;\n"
    "uniform mat4 mvp;\n"
    "varying vec4 v_color;\n"
    "void main() {\n"
    "  gl_Position = mvp*vertex;\n"
    "  v_color = color;\n"
    "}\n";

static const char kFragmentShader[] =
    "varying vec4 v_color;\n"
    "void main() {\n"
    "  gl_FragColor = v_color;\n"
    "}\n";

static const float vertices[] = {
  0.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,

  0.0f, 0.0f, 0.0f,
  0.0f, 1.0f, 0.0f,

  0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 1.0f
};

static const float colors[] = {
  1.0f, 0.0f, 0.0f, 1.0f,
  1.0f, 0.0f, 0.0f, 1.0f,

  0.0f, 1.0f, 0.0f, 1.0f,
  0.0f, 1.0f, 0.0f, 1.0f,

  0.0f, 0.0f, 1.0f, 1.0f,
  0.0f, 0.0f, 1.0f, 1.0f
};

Axis::Axis() {
  shader_program = GlUtil::CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program) {
    LOGE("Could not create program.");
  }
  uniform_mvp_mat = glGetUniformLocation(shader_program, "mvp");
  attrib_colors = glGetAttribLocation(shader_program, "color");
  attrib_vertices = glGetAttribLocation(shader_program, "vertex");

  glGenBuffers(1, &vertex_buffer);
  glGenBuffers(1, &color_buffer);
}

void Axis::Render(glm::mat4 view_projection_mat) {
  glUseProgram(shader_program);

  // Calculate MVP matrix and pass it to shader.
  glm::mat4 model_mat = GetCurrentModelMatrix();
  glm::mat4 mvp_mat = view_projection_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  // Vertice binding.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 18, vertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(attrib_vertices);
  glVertexAttribPointer(attrib_vertices, 3, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Color binding.
  glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 24, colors, GL_STATIC_DRAW);
  glEnableVertexAttribArray(attrib_colors);
  glVertexAttribPointer(attrib_colors, 4, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDrawArrays(GL_LINES, 0, 6 * 3);
  glUseProgram(0);
}
