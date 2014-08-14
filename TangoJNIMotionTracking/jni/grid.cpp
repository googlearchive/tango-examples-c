#include "grid.h"

static const char kVertexShader[] = "attribute vec4 vertex;\n"
    "uniform mat4 mvp;\n"
    "void main() {\n"
    "  gl_Position = mvp*vertex;\n"
    "}\n";

static const char kFragmentShader[] = "void main() {\n"
    "  // Gray color."
    "  gl_FragColor = vec4(0.58f, 0.58f, 0.58f, 1.0f);\n"
    "}\n";

Grid::Grid() {
  density = 0.2f;
  quantity = 100;

  shader_program = GlUtil::CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program) {
    LOGE("Could not create program.");
  }
  uniform_mvp_mat = glGetUniformLocation(shader_program, "mvp");
  attrib_vertices = glGetAttribLocation(shader_program, "vertex");

  glGenBuffers(1, &vertex_buffer);

  int counter = 0;
  // 3 float in 1 vertex, 2 vertices form a line
  // Horizontal line and vertical line forms the grid.
  traverse_len = quantity * 2 * 3 * 2;
  vertices = new float[traverse_len];
  float width = density * quantity / 2;

  // Horizontal line.
  for (int i = 0; i < traverse_len / 2; i += 6) {
    vertices[i] = -width;
    vertices[i + 1] = 0.0f;
    vertices[i + 2] = -width + counter * density;

    vertices[i + 3] = width;
    vertices[i + 4] = 0.0f;
    vertices[i + 5] = -width + counter * density;

    counter++;
  }

  // Vertical line.
  counter = 0;
  for (int i = traverse_len / 2; i < traverse_len; i += 6) {
    vertices[i] = -width + counter * density;
    vertices[i + 1] = 0.0f;
    vertices[i + 2] = -width;

    vertices[i + 3] = -width + counter * density;
    vertices[i + 4] = 0.0f;
    vertices[i + 5] = width;

    counter++;
  }
}

void Grid::Render(glm::mat4 view_projection_mat) {
  glUseProgram(shader_program);

  // matrix stuff.
  glm::mat4 model_mat = GetCurrentModelMatrix();
  glm::mat4 mvp_mat = view_projection_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  // vertice binding
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * traverse_len, vertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(attrib_vertices);
  glVertexAttribPointer(attrib_vertices, 3, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDrawArrays(GL_LINES, 0, traverse_len);
  glUseProgram(0);
}

Grid::~Grid() {
  delete[] vertices;
}
