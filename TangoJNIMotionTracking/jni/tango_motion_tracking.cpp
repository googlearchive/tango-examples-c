#include <stdlib.h>
#include <jni.h>
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
GLuint color_id = 0;
GLuint program_id = 0;

static glm::mat4 projection_matrix;
static glm::mat4 modelview_matrix;
static glm::mat4 mvp_matrix;

static const char vertex_shader[] = { "uniform mat4 u_mvp_matrix;      \n"
    "attribute vec4 a_position;     \n"
    "attribute vec4 a_color;        \n"
    "varying vec4 v_color;          \n"
    "void main()                    \n"
    "{                              \n"
    "   v_color = a_color;          \n"
    "   gl_Position = u_mvp_matrix   \n"
    "               * a_position;   \n"
    "}                              \n" };

static const char fragment_shader[] = { "precision mediump float;       \n"
    "varying vec4 v_color;          \n"
    "void main()                    \n"
    "{                              \n"
    "   gl_FragColor = v_color;     \n"
    "}                              \n" };

static const GLfloat cube_vertices[] = {
    // front
    -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,

    // right
    0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
    0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f,

    // back
    0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f,
    -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,

    // left
    -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,

    // top
    -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,

    // bottom
    -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
    0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f };

static const GLfloat cube_colors[] = {
    // front, blue
    0.0625f, 0.5742f, 0.9257f, 1.0f, 0.0625f, 0.5742f, 0.9257f, 1.0f, 0.0625f,
    0.5742f, 0.9257f, 1.0f, 0.0625f, 0.5742f, 0.9257f, 1.0f, 0.0625f, 0.5742f,
    0.9257f, 1.0f, 0.0625f, 0.5742f, 0.9257f, 1.0f,

    // right, purple
    0.8549f, 0.4471f, 0.9176f, 1.0f, 0.8549f, 0.4471f, 0.9176f, 1.0f, 0.8549f,
    0.4471f, 0.9176f, 1.0f, 0.8549f, 0.4471f, 0.9176f, 1.0f, 0.8549f, 0.4471f,
    0.9176f, 1.0f, 0.8549f, 0.4471f, 0.9176f, 1.0f,

    // back, yellow
    0.9098f, 0.8706f, 0.4118f, 1.0f, 0.9098f, 0.8706f, 0.4118f, 1.0f, 0.9098f,
    0.8706f, 0.4118f, 1.0f, 0.9098f, 0.8706f, 0.4118f, 1.0f, 0.9098f, 0.8706f,
    0.4118f, 1.0f, 0.9098f, 0.8706f, 0.4118f, 1.0f,

    // left, yellow
    0.9098f, 0.8706f, 0.4118f, 1.0f, 0.9098f, 0.8706f, 0.4118f, 1.0f, 0.9098f,
    0.8706f, 0.4118f, 1.0f, 0.9098f, 0.8706f, 0.4118f, 1.0f, 0.9098f, 0.8706f,
    0.4118f, 1.0f, 0.9098f, 0.8706f, 0.4118f, 1.0f,

    // top, orange
    0.8980f, 0.4627f, 0.1922f, 1.0f, 0.8980f, 0.4627f, 0.1922f, 1.0f, 0.8980f,
    0.4627f, 0.1922f, 1.0f, 0.8980f, 0.4627f, 0.1922f, 1.0f, 0.8980f, 0.4627f,
    0.1922f, 1.0f, 0.8980f, 0.4627f, 0.1922f, 1.0f,

    // bottom, green
    0.1921f, 0.8981f, 0.3019f, 1.0f, 0.1921f, 0.8981f, 0.3019f, 1.0f, 0.1921f,
    0.8981f, 0.3019f, 1.0f, 0.1921f, 0.8981f, 0.3019f, 1.0f, 0.1921f, 0.8981f,
    0.3019f, 1.0f, 0.1921f, 0.8981f, 0.3019f, 1.0f, };

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
    glBindAttribLocation(program, 1, "a_color");
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

  glm::mat4 translateMatrix = glm::translate(
      glm::mat4(1.0f),
      glm::vec3(viostatus.translation[0],
                viostatus.translation[1],
                viostatus.translation[2] - 6.0f));
  glm::quat rotationQuaterion = glm::quat(viostatus.rotation[3],
                                          viostatus.rotation[0],
                                          viostatus.rotation[1],
                                          viostatus.rotation[2]);
  glm::mat4 rotationMatrix = glm::mat4_cast(rotationQuaterion);

  modelview_matrix = translateMatrix * rotationMatrix;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(program_id);
  mvp_matrix_id = glGetUniformLocation(program_id, "u_mvp_matrix");
  position_id = glGetAttribLocation(program_id, "a_position");
  color_id = glGetAttribLocation(program_id, "a_color");

  glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, cube_vertices);
  glEnableVertexAttribArray(position_id);
  glVertexAttribPointer(color_id, 4, GL_FLOAT, GL_FALSE, 0, cube_colors);
  glEnableVertexAttribArray(color_id);

  mvp_matrix = projection_matrix * modelview_matrix;
  glUniformMatrix4fv(mvp_matrix_id, 1, false, glm::value_ptr(mvp_matrix));
  glDrawArrays(GL_TRIANGLES, 0, 36);

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
