#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>


#include <tango-api/application-interface.h>
#include <tango-api/hardware-control-interface.h>
#include <tango-api/video-overlay-interface.h>

#define GLM_FORCE_RADIANS
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "gtc/type_ptr.hpp"

#define  LOG_TAG    "tango_pointcloud"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static const char kVertexShader[] =
"attribute vec4 vertex;\n"
"uniform mat4 mvp;\n"
"varying vec4 v_normal;\n"
"void main() {\n"
"  gl_PointSize = 10.0;\n"
"  gl_Position = mvp*vertex;\n"
"  v_normal = vertex;\n"
"}\n";

static const char kFragmentShader[] =
"varying vec4 v_normal;\n"
"void main() {\n"
"  gl_FragColor = vec4(v_normal);\n"
"}\n";

static const GLfloat kVertices[] = {
  -0.5f, 0.5f, -0.5f,
  0.5f, 0.5f, -0.5f,
  -0.5f, -0.5f, -0.5f,
  0.5f, -0.5f, -0.5f,
  
  -0.5f, 0.5f, 0.5f,
  0.5f, 0.5f, 0.5f,
  -0.5f, -0.5f, 0.5f,
  0.5f, -0.5f, 0.5f};

static const GLushort kIndices[] = {
  5,4,6,
  5,6,7,
  
  1,5,7,
  1,7,3,
  
  1,3,2,
  0,1,2,
  
  4,0,6,
  6,0,2,
  
  5,0,4,
  1,0,5,
  
  7,6,2,
  7,2,3
};


application_handle_t *app_handler;
struct VideoModeAttributes video_mode;

GLuint screen_width;
GLuint screen_height;

GLuint shader_program;
GLuint attrib_vertices;
GLuint attrib_textureCoords;
GLuint uniform_texture;
GLuint uniform_mvp_mat;

GLuint texture_id;
GLuint vertex_buffers[2];

double video_overlay_timestamp;

static void CheckGlError(const char* operation) {
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("after %s() glError (0x%x)\n", operation, error);
  }
}

GLuint LoadShader(GLenum shader_type, const char* shader_source) {
  GLuint shader = glCreateShader(shader_type);
  if (shader) {
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
      GLint info_len = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
      if (info_len) {
        char* buf = (char*) malloc(info_len);
        if (buf) {
          glGetShaderInfoLog(shader, info_len, NULL, buf);
          LOGE("Could not compile shader %d:\n%s\n", shader_type, buf);
          free(buf);
        }
        glDeleteShader(shader);
        shader = 0;
      }
    }
  }
  return shader;
}

GLuint CreateProgram(const char* vertex_source, const char* fragment_source) {
  GLuint vertexShader = LoadShader(GL_VERTEX_SHADER, vertex_source);
  if (!vertexShader) {
    return 0;
  }
  
  GLuint fragment_shader = LoadShader(GL_FRAGMENT_SHADER, fragment_source);
  if (!fragment_shader) {
    return 0;
  }
  
  GLuint program = glCreateProgram();
  if (program) {
    glAttachShader(program, vertexShader);
    CheckGlError("glAttachShader");
    glAttachShader(program, fragment_shader);
    CheckGlError("glAttachShader");
    glLinkProgram(program);
    GLint link_status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE) {
      GLint buf_length = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &buf_length);
      if (buf_length) {
        char* buf = (char*) malloc(buf_length);
        if (buf) {
          glGetProgramInfoLog(program, buf_length, NULL, buf);
          LOGE("Could not link program:\n%s\n", buf);
          free(buf);
        }
      }
      glDeleteProgram(program);
      program = 0;
    }
  }
  return program;
}

bool SetupTango() {
  
  return true;
}

bool SetupGraphics(int w, int h) {
  LOGI("setupGraphics(%d, %d)", w, h);
  
  // /* Shaders */
  // #define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
  // #define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
  glEnable(0x8642);
  
  shader_program = CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program) {
    LOGE("Could not create program.");
    return false;
  }
  
  glGenBuffers(3, vertex_buffers);

  // vertice
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 8, kVertices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  // triangles
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffers[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 3 * 12, kIndices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  
  // vertex attribute
  attrib_vertices = glGetAttribLocation(shader_program, "vertex");
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[0]);
  glEnableVertexAttribArray(attrib_vertices);
  glVertexAttribPointer(attrib_vertices, 3, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  screen_width = w;
  screen_height = h;
  glViewport(0, 0, screen_width, screen_height);
  CheckGlError("glViewport");
  
  uniform_mvp_mat = glGetUniformLocation(shader_program, "mvp");
  
  return true;
}

void UpdateTango()
{
  
}

float rotate_rate = 0.0f;

bool RenderFrame() {
  
  UpdateTango();
  
  glUseProgram(shader_program);
  CheckGlError("glUseProgram");
  glViewport(0, 0, screen_width, screen_height);
  
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);
  
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  
  glm::mat4 Projection = glm::perspective(45.0f, (float)(screen_width / screen_height), 0.1f, 100.0f);
  glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
  
  
  rotate_rate+=0.02f;
  glm::mat4 Model = glm::rotate(glm::mat4(1.0f), rotate_rate, glm::vec3(0.0f, 1.0f, 1.0f));
  glm::mat4 MVP = Projection * View * Model;
  glUniformMatrix4fv(uniform_mvp_mat, 1, GL_FALSE, glm::value_ptr(MVP));
  
  // vertex
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[0]);
  glEnableVertexAttribArray(attrib_vertices);
  glVertexAttribPointer(attrib_vertices, 3, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  // bind element array buffer
//  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffers[1]);
//  glDrawElements(GL_POINTS, 32, GL_UNSIGNED_SHORT, 0);
  glDrawArrays(GL_POINTS, 0, 8);
  CheckGlError("glDrawElements");
//  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  
  return true;
}

#ifdef __cplusplus
extern "C" {
#endif
  JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_init(
                                                                                        JNIEnv * env, jobject obj, jint width, jint height)
  {
    SetupGraphics(width, height);
    SetupTango();
  }
  
  JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_render(
                                                                                          JNIEnv * env, jobject obj)
  {
    RenderFrame();
  }
#ifdef __cplusplus
}
#endif