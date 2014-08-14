#include "pointcloud.h"

static const char kVertexShader[] = "attribute vec4 vertex;\n"
    "uniform mat4 mvp;\n"
    "varying vec4 v_color;\n"
    "void main() {\n"
    "  gl_PointSize = 4.0;\n"
    "  gl_Position = mvp*vertex;\n"
    "  v_color = vertex;\n"
    "}\n";

static const char kFragmentShader[] = "varying vec4 v_color;\n"
    "void main() {\n"
    "  gl_FragColor = vec4(v_color);\n"
    "}\n";

static const glm::mat4 inverse_z_mat = glm::mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0,
                                                 0.0, 0.0, 0.0, 0.0, -1.0, 0.0,
                                                 0.0, 0.0, 0.0, 1.0);

Pointcloud::Pointcloud() {
  // Shaders
  // #define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
  // #define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
  glEnable(0x8642);
  shader_program = GlUtil::CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program) {
    LOGE("Could not create program.");
  }
  uniform_mvp_mat = glGetUniformLocation(shader_program, "mvp");
  attrib_vertices = glGetAttribLocation(shader_program, "vertex");
  glGenBuffers(1, &vertex_buffers);
}

void Pointcloud::Render(glm::mat4 model_view_mat, float depth_buffer_size,
                        float *depth_data_buffer) {

  glUseProgram(shader_program);
  // matrix stuff.
  glm::mat4 model_mat = glm::mat4(1.0f);
  glm::mat4 mvp_mat = model_view_mat * model_mat * inverse_z_mat;
  glUniformMatrix4fv(uniform_mvp_mat, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  // vertice binding
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * depth_buffer_size,
               depth_data_buffer, GL_STATIC_DRAW);
  glEnableVertexAttribArray(attrib_vertices);
  glVertexAttribPointer(attrib_vertices, 3, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDrawArrays(GL_POINTS, 0, 3 * depth_buffer_size);
  GlUtil::CheckGlError("draw array");
  glUseProgram(0);
}
