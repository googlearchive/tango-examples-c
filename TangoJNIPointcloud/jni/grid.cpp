#include "grid.h"
#include "gl_util.h"

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
"  gl_FragColor = vec4(0.58f, 0.58f, 0.58f, 1.0f);\n"
"}\n";

//static const float vertices[] = {
//  0.0f, 0.0f, 0.0f,
//  1.0f, 0.0f, 0.0f,
//  
//  0.0f, 0.0f, 0.0f,
//  0.0f, 1.0f, 0.0f,
//  
//  0.0f, 0.0f, 0.0f,
//  0.0f, 0.0f, 1.0f
//};
//
//static const float colors[] = {
//  1.0f, 0.0f, 0.0f, 1.0f,
//  1.0f, 0.0f, 0.0f, 1.0f,
//  
//  0.0f, 1.0f, 0.0f, 1.0f,
//  0.0f, 1.0f, 0.0f, 1.0f,
//  
//  0.0f, 0.0f, 1.0f, 1.0f,
//  0.0f, 0.0f, 1.0f, 1.0f
//};

Grid::Grid()
{
  shader_program = GlUtil::CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program) {
    LOGE("Could not create program.");
  }
  uniform_mvp_mat = glGetUniformLocation(shader_program, "mvp");
//  attrib_colors = glGetAttribLocation(shader_program, "color");
  attrib_vertices = glGetAttribLocation(shader_program, "vertex");
  
  glGenBuffers(1, &vertex_buffer);
//  glGenBuffers(1, &color_buffer);
  
  int counter = 0;
  vertices = new float[1200];
  
  // Horizontal line.
  for (int i=0; i<600; i+=6) {
    vertices[i]   = -10.0f;
    vertices[i+1] = 0.0f;
    vertices[i+2] = -10.0f + counter*0.2f;
    
    vertices[i+3] = 10.0f;
    vertices[i+4] = 0.0f;
    vertices[i+5] = -10.0f + counter*0.2f;
    
    counter++;
  }
  
  // Vertical line.
  counter = 0;
  for (int i = 600; i<1200; i+=6) {
    vertices[i]   = -10.0f + counter*0.2f;
    vertices[i+1] = 0.0f;
    vertices[i+2] = -10.0f;
    
    vertices[i+3] = -10.0f + counter*0.2f;
    vertices[i+4] = 0.0f;
    vertices[i+5] = 10.0f;
    
    counter++;
  }
}

void Grid::Render(glm::mat4 view_projection_mat)
{
  glUseProgram(shader_program);
  
  // matrix stuff.
  glm::mat4 model_mat = GetCurrentModelMatrix();
  glm::mat4 mvp_mat = view_projection_mat * model_mat;
  glUniformMatrix4fv(uniform_mvp_mat, 1, GL_FALSE, glm::value_ptr(mvp_mat));
  
  // vertice binding
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 1200, vertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(attrib_vertices);
  glVertexAttribPointer(attrib_vertices, 3, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  // color binding
//  glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
//  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 24, colors,
//               GL_STATIC_DRAW);
//  glEnableVertexAttribArray(attrib_colors);
//  glVertexAttribPointer(attrib_colors, 4, GL_FLOAT, GL_FALSE, 0,
//                        (const void*) 0);
//  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  glDrawArrays(GL_LINES, 0, 1200);
  glUseProgram(0);
}

Grid::~Grid(){
  delete[] vertices;
}
