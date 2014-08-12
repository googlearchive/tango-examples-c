#include <stdlib.h>
#include <jni.h>
#include <vector>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <tango-api/application-interface.h>
#include <tango-api/vio-interface.h>

#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#include <gtc/type_ptr.hpp>

#define  LOG_TAG    "tango_motion_tracking"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

application_handle_t *app_handler;

GLuint mvp_matrix_id = 0;
GLuint position_id = 0;
GLuint program_id = 0;

GLuint vertex_buffer;

static glm::mat4 projection_matrix;
static glm::mat4 modelview_matrix;
static glm::mat4 modelview_matrix2;
static glm::mat4 mvp_matrix;

std::vector<glm::vec3> vertices;

static const char vertex_shader[] = {
    "uniform mat4 u_mvp_matrix;      \n"
    "attribute vec4 a_position;     \n"
    "void main()                    \n"
    "{                              \n"
    "   gl_Position = u_mvp_matrix   \n"
    "               * a_position;   \n"
    "}                              \n" };

static const char fragment_shader[] = {
    "precision mediump float;       \n"
    "void main()                    \n"
    "{                              \n"
    "   gl_FragColor = vec4(0,1,0,1);     \n"
    "}                              \n" };

static const GLfloat cube_vertices[] = {
0.0f,0.0f,0.0f,
-0.5f,0.2f,-0.3f,

0.0f,0.0f,0.0f,
0.5f,0.2f,-0.3f,

0.0f,0.0f,0.0f,
-0.5f,-0.2f,-0.3f,

0.0f,0.0f,0.0f,
0.5f,-0.2f,-0.3f,

-0.5f,0.2f,-0.3f,
0.5f,0.2f,-0.3f,

0.5f,0.2f,-0.3f,
0.5f,-0.2f,-0.3f,

0.5f,-0.2f,-0.3f,
-0.5f,-0.2f,-0.3f,

-0.5f,-0.2f,-0.3f,
-0.5f,0.2f,-0.3f,
};

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
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
      GLint info_length = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_length);
      if (info_length) {
        char* buffer = (char*) malloc(info_length * sizeof(char));
        if (buffer) {
          glGetShaderInfoLog(shader, info_length, NULL, buffer);
        }
      }
      glDeleteShader(shader);
      shader = 0;
    }
  }
  return shader;
}

GLuint CreateProgram(const char* vertex_shader_source,
                     const char* fragment_shader_source) {
  GLuint vertex_shader = LoadShader(GL_VERTEX_SHADER, vertex_shader_source);
  if (!vertex_shader) {
    return 0;
  }
  GLuint fragment_shader = LoadShader(GL_FRAGMENT_SHADER,
                                      fragment_shader_source);
  if (!fragment_shader) {
    return 0;
  }
  GLuint program = glCreateProgram();
  if (program) {
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glBindAttribLocation(program, 0, "a_position");
    glLinkProgram(program);

    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (!status) {
      GLint info_length = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_length);
      if (info_length) {
        char* buffer = (char*) malloc(info_length * sizeof(char));
        glGetProgramInfoLog(program, info_length, NULL, buffer);
      }
      glDeleteProgram(program);
      program = 0;
    }
  }
  return program;
}

bool SetupTango() {
  // initialize tango application.
  app_handler = ApplicationInitialize("[Superframes Small-Peanut]", 0);
  if (app_handler == NULL) {
    LOGI("Application initialize failed\n");
    return false;
  }

  // initialize motion tracking.
  CAPIErrorCodes ret_error;
  if ((ret_error = VIOInitialize(app_handler, 1, NULL)) != kCAPISuccess) {
    LOGI("motion tracking init failed: %d\n", ret_error);
    return false;
  }

  LOGI("Application and motion tracking initialized success");
  return true;
}

bool SetupGraphics(int w, int h) {
  LOGI("setupGraphics(%d, %d)", w, h);

  glClearColor(0, 0, 0, 1.0f);
  glEnable (GL_CULL_FACE);
  glEnable (GL_DEPTH_TEST);
  program_id = CreateProgram(vertex_shader, fragment_shader);

  projection_matrix = glm::perspective(75.0f, (GLfloat) w / h, 0.01f, 10.0f);
  glViewport(0, 0, w, h);

  mvp_matrix_id = glGetUniformLocation(program_id, "u_mvp_matrix");
  position_id = glGetAttribLocation(program_id, "a_position");

  vertices.reserve(1000);

  glGenBuffers(1, &vertex_buffer);

  return true;
}

bool RenderFrame() {
  VIOStatus viostatus;
  CAPIErrorCodes ret_error;
  if ((ret_error = ApplicationDoStep(app_handler)) != kCAPISuccess) {
    LOGE("Application do step failed: %d\n", ret_error);
    return false;
  }

  if ((ret_error = VIOGetLatestPoseUnity(app_handler, &viostatus))
      != kCAPISuccess) {
    LOGE("Application get latest pose failed: %d\n", ret_error);
    return false;
  }

  vertices.push_back(glm::vec3(viostatus.translation[0]*-1.0f,
                               viostatus.translation[1]*-1.0f,
                               viostatus.translation[2]));


  glm::mat4 translateMatrix = glm::translate(
      glm::mat4(1.0f),
      glm::vec3(viostatus.translation[0]*-1.0f,
                viostatus.translation[1]*-1.0f,
                viostatus.translation[2] - 6.0f));

  glm::quat rotationQuaterion = glm::quat(viostatus.rotation[3],
                                          viostatus.rotation[0],
                                          viostatus.rotation[1],
                                          viostatus.rotation[2]);
  glm::mat4 rotationMatrix = glm::mat4_cast(rotationQuaterion);
  modelview_matrix2 = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(0,
                  0,
                  0 - 6.0f));

 modelview_matrix = translateMatrix*rotationMatrix;



  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



  glUseProgram(program_id);



//vertice binding
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 48, cube_vertices,
                GL_STATIC_DRAW);
  glEnableVertexAttribArray(position_id);
  glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  mvp_matrix = projection_matrix * modelview_matrix;
  glUniformMatrix4fv(mvp_matrix_id, 1, false, glm::value_ptr(mvp_matrix));
  glDrawArrays(GL_LINES, 0, 16*3);
//  mvp_matrix = projection_matrix * modelview_matrix;
//  glUniformMatrix4fv(mvp_matrix_id, 1, false, glm::value_ptr(mvp_matrix));

  glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &vertices[0]) ;
  glEnableVertexAttribArray(position_id);
  mvp_matrix = projection_matrix * modelview_matrix2;
  glUniformMatrix4fv(mvp_matrix_id, 1, false, glm::value_ptr(mvp_matrix));
  glDrawArrays(GL_LINE_STRIP,0,vertices.size());


  glUseProgram(0);

  return true;
}

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_com_google_tango_tangojnimotiontracking_TangoJNINative_init(
    JNIEnv * env, jobject obj, jint width, jint height)
{
  SetupTango();
  SetupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_com_google_tango_tangojnimotiontracking_TangoJNINative_render(
    JNIEnv * env, jobject obj)
{
  RenderFrame();
}
#ifdef __cplusplus
}
#endif
