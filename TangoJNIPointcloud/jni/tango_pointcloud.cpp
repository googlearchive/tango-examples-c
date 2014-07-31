#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>


#include <tango-api/application-interface.h>
#include <tango-api/hardware-control-interface.h>
#include <tango-api/depth-interface.h>

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
"varying vec4 v_color;\n"
"void main() {\n"
"  gl_PointSize = 4.0;\n"
"  gl_Position = mvp*vertex;\n"
"  v_color = vertex;\n"
"}\n";

static const char kFragmentShader[] =
"varying vec4 v_color;\n"
"void main() {\n"
"  gl_FragColor = vec4(v_color);\n"
"}\n";

static const glm::mat4 inverse_z_mat =
  glm::mat4(1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, -1.0, 0.0,
            0.0, 0.0, 0.0, 1.0);

static const int kMaxVertCount = 61440;

application_handle_t *app_handler;

GLuint screen_width;
GLuint screen_height;

GLuint shader_program;
GLuint attrib_vertices;
GLuint attrib_textureCoords;
GLuint uniform_texture;
GLuint uniform_mvp_mat;

GLuint vertex_buffers;

glm::mat4 projection_matrix;

double pointcloud_timestamp = 0.0;
float depth_data_buffer[kMaxVertCount * 3];
int depth_buffer_size = kMaxVertCount * 3;

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
  app_handler = ApplicationInitialize("[Superframes Small-Peanut]", 1);
  if (app_handler == NULL) {
    LOGI("Application initialize failed\n");
    return false;
  }
  
  CAPIErrorCodes ret_error;
  if ((ret_error = DepthStartBuffering(app_handler)) != kCAPISuccess) {
    LOGI("DepthStartBuffering failed: %d\n", ret_error);
    return false;
  }
  
  return true;
}

bool SetupGraphics(int w, int h) {
  LOGI("setupGraphics(%d, %d)", w, h);
  
  // Shaders
  // #define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
  // #define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
  glEnable(0x8642);
  
  shader_program = CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program) {
    LOGE("Could not create program.");
    return false;
  }
  
  glGenBuffers(1, &vertex_buffers);
  
  uniform_mvp_mat = glGetUniformLocation(shader_program, "mvp");
  
  screen_width = w;
  screen_height = h;
  projection_matrix =
    glm::perspective(45.0f, (float)(screen_width / screen_height), 0.1f, 100.0f);
  glViewport(0, 0, screen_width, screen_height);
  CheckGlError("glViewport");

  return true;
}

bool UpdateTango()
{
  ApplicationDoStep(app_handler);
  
  depth_buffer_size = kMaxVertCount * 3;
  pointcloud_timestamp = 0.0f;
  CAPIErrorCodes ret_error;
  if ((ret_error = DepthGetPointCloudUnity(
                    app_handler, &pointcloud_timestamp,
                    0.5f, depth_data_buffer, &depth_buffer_size)) != kCAPISuccess) {
    LOGI("DepthGetPointCloud failed: %d\n", ret_error);
    return false;
  }
  return true;
}

bool RenderFrame() {
  UpdateTango();
  
  glUseProgram(shader_program);
  CheckGlError("glUseProgram");
  glViewport(0, 0, screen_width, screen_height);
  
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);
  
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  
  depth_buffer_size = glm::clamp(depth_buffer_size, 0, kMaxVertCount);
  for (int i = 0; i < 3 * depth_buffer_size; i++) {
    depth_data_buffer[i] = depth_data_buffer[i] * 0.001f;
  }
  
  // matrix stuff.
  glm::mat4 view_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
  glm::mat4 model_mat = glm::mat4(1.0f);
  glm::mat4 mvp_mat = projection_matrix * view_mat * model_mat * inverse_z_mat;
  glUniformMatrix4fv(uniform_mvp_mat, 1, GL_FALSE, glm::value_ptr(mvp_mat));
  
  // vertice binding
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * depth_buffer_size, depth_data_buffer,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(attrib_vertices);
  glVertexAttribPointer(attrib_vertices, 3, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  glDrawArrays(GL_POINTS, 0, 3 * depth_buffer_size);
  CheckGlError("glDrawElements");
  
  return true;
}

#ifdef __cplusplus
extern "C" {
#endif
  JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_init(
    JNIEnv * env, jobject obj, jint width, jint height){
    SetupGraphics(width, height);
    SetupTango();
  }
  
  JNIEXPORT void JNICALL Java_com_google_tango_tangojnipointcloud_TangoJNINative_render(
    JNIEnv * env, jobject obj){
    RenderFrame();
  }
#ifdef __cplusplus
}
#endif