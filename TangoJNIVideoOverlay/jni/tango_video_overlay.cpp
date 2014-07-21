#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <tango-api/application-interface.h>
#include <tango-api/hardware-control-interface.h>
#include <tango-api/video-overlay-interface.h>
#include <tango-api/tango_client_api.h>

#define  LOG_TAG    "tango_video_overlay"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static const char kVertexShader[] = "precision highp float;\n"
    "precision highp int;\n"
    "attribute vec4 vertex;\n"
    "attribute vec2 textureCoords;\n"
    "varying vec2 f_textureCoords;\n"
    "void main() {\n"
    "  f_textureCoords = textureCoords;\n"
    "  gl_Position = vertex;\n"
    "}\n";

static const char kFragmentShader[] =
    "#extension GL_OES_EGL_image_external : require\n"
    "precision highp float;\n"
    "precision highp int;\n"
    "uniform samplerExternalOES texture;\n"
    "varying vec2 f_textureCoords;\n"
    "void main() {\n"
    "  gl_FragColor = texture2D(texture, f_textureCoords);\n"
    "}\n";

static const GLfloat kVertices[] = { -0.5, 0.5, 0.0, -0.5, -0.5, 0.0, 0.5, 0.5,
    0.0, 0.5, -0.5, 0.0 };

static const GLushort kIndices[] = { 0, 1, 2, 2, 1, 3 };

static const GLfloat kTextureCoords[] =
    { 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0 };

application_handle_t *app_handler;
struct VideoModeAttributes video_mode;

GLuint screen_width;
GLuint screen_height;

GLuint shader_program;
GLuint attrib_vertices;
GLuint attrib_textureCoords;
GLuint uniform_texture;

GLuint texture_id;
GLuint vertex_buffers[3];

double video_overlay_timestamp;

static void onPoseAvailable(TangoPoseData *pose) {
  LOGI("PoseCallback: %lf %lf %lf %lf %lf\t%lf %lf %lf\n",
         pose->timestamp,
         pose->orientation[0],
         pose->orientation[1],
         pose->orientation[2],
         pose->orientation[3],
         pose->translation[0],
         pose->translation[1],
         pose->translation[2]);
}

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
  int i;
  TangoConfig* config;
  if (TangoService_initialize() != 0) {
    LOGI("TangoService_initialize(): Failed\n");
    return -1;
  }
  
  // Allocate a TangoConfig object.
  if ((config = TangoConfig_alloc()) == NULL) {
    LOGI("TangoService_allocConfig(): Failed\n");
    return -1;
  }
  
  // Get the default TangoConfig.
  if (TangoService_getConfig(TANGO_CONFIG_DEFAULT, config) != 0) {
    LOGI("TangoService_getConfig(): Failed\n");
    return -1;
  }
  
  // Set a parameter in TangoConfig.
  if (TangoConfig_setBool(config, "disable_opengl", true) != 0) {
    LOGI("TangoConfig_setBool(\"disable_opengl\", true): Failed\n");
    return -1;
  }
  
  // Report the current TangoConfig.
  LOGI("TangoConfig:\n%s\n", TangoConfig_toString(config));
  
  // Lock in this configuration.
  if (TangoService_lockConfig(config) != 0) {
    LOGI("TangoService_lockConfig(): Failed\n");
    return -1;
  }
  
  // Attach the onPoseAvailable callback.
  if (TangoService_connectOnPoseAvailable(onPoseAvailable) != 0) {
    LOGI("TangoService_connectOnPoseAvailable(): Failed\n");
    return -1;
  }
  
  TangoService_connectTextureId(TANGO_CAMERA_COLOR,
                                texture_id);
  
  // Connect to the Tango Service.
  TangoService_connect();
  
  return true;
}

bool SetupGraphics(int w, int h) {
  LOGI("setupGraphics(%d, %d)", w, h);

  shader_program = CreateProgram(kVertexShader, kFragmentShader);
  if (!shader_program) {
    LOGE("Could not create program.");
    return false;
  }

  // init texture
//  glGenTextures(1, &texture_id);
//  glActiveTexture (GL_TEXTURE0);
//  glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id);
//  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//  CheckGlError("Texture");
  glEnable(GL_TEXTURE_EXTERNAL_OES);
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // texutre
//  uniform_texture = glGetUniformLocation(shader_program, "texture");
//  glUniform1i(uniform_texture, texture_id);
//  glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);

  glGenBuffers(3, vertex_buffers);
  // vertice
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, kVertices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // triangles
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffers[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6, kIndices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // texture coords
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * 4, kTextureCoords,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // vertex attribute
  attrib_vertices = glGetAttribLocation(shader_program, "vertex");
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[0]);
  glEnableVertexAttribArray(attrib_vertices);
  glVertexAttribPointer(attrib_vertices, 3, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // texture coords attribute
  attrib_textureCoords = glGetAttribLocation(shader_program, "textureCoords");
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[2]);
  glEnableVertexAttribArray(attrib_textureCoords);
  glVertexAttribPointer(attrib_textureCoords, 2, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  screen_width = w;
  screen_height = h;
  glViewport(0, 0, screen_width, screen_height);
  CheckGlError("glViewport");

  return true;
}

void UpdateTango()
{
  TangoService_updateTexture(TANGO_CAMERA_COLOR);
}

bool RenderFrame() {
  UpdateTango();

  glUseProgram(shader_program);
  CheckGlError("glUseProgram");
  
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);
  glEnable (GL_BLEND);
  glEnable(GL_TEXTURE_EXTERNAL_OES);

  glActiveTexture (GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id);
  CheckGlError("Texture");
  
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, screen_width, screen_height);

  // vertex
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[0]);
  glEnableVertexAttribArray(attrib_vertices);
  glVertexAttribPointer(attrib_vertices, 3, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // texture coords
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[2]);
  glEnableVertexAttribArray(attrib_textureCoords);
  glVertexAttribPointer(attrib_textureCoords, 2, GL_FLOAT, GL_FALSE, 0,
                        (const void*) 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
//  // texture
//  glActiveTexture (GL_TEXTURE0);
//  glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id);
//  CheckGlError("Texture");
//  uniform_texture = glGetUniformLocation(shader_program, "texture");
//  glUniform1i(uniform_texture, texture_id);

//  // init texture
//  glGenTextures(1, &texture_id);
//  glActiveTexture (GL_TEXTURE0);
//  glBindTexture(GL_TEXTURE_2D, texture_id);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//
//  CheckGlError("Texture");
  
  // texutre
//  glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id);
//  uniform_texture = glGetUniformLocation(shader_program, "texture");
//  glUniform1i(uniform_texture, texture_id);
  
  
  
  // bind element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffers[1]);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
  CheckGlError("glDrawElements");
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  
  return true;
}

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_com_google_tango_tangojnivideooverlay_TangoJNINative_init(
    JNIEnv * env, jobject obj, jint width, jint height)
{
  SetupGraphics(width, height);
  SetupTango();
}

JNIEXPORT void JNICALL Java_com_google_tango_tangojnivideooverlay_TangoJNINative_render(
    JNIEnv * env, jobject obj)
{
  RenderFrame();
}
#ifdef __cplusplus
}
#endif
