#define GLM_FORCE_RADIANS

#include "axis.h"
#include "camera.h"
#include "frustum.h"
#include "gl_util.h"
#include "grid.h"
#include "tango_data.h"
#include "trace.h"

GLuint screen_width;
GLuint screen_height;

Camera *cam;
Axis *axis;
Frustum *frustum;
Grid *grid;
Trace *trace;

enum CameraType {
  FIRST_PERSON = 0,
  THIRD_PERSON = 1,
  TOP_DOWN = 2
};

int camera_type;

// Quaternion format of rotation.
const glm::vec3 kThirdPersonCameraPosition = glm::vec3(0.0f, 3.0f, 3.0f);
const glm::quat kThirdPersonCameraRotation = glm::quat(0.92388f, -0.38268f,
                                                       0.0f, 0.0f);
const glm::vec3 kTopDownCameraPosition = glm::vec3(0.0f, 3.0f, 0.0f);
const glm::quat kTopDownCameraRotation = glm::quat(0.70711f, -0.70711f, 0.0f,
                                                   0.0f);

bool SetupGraphics(int w, int h) {
  LOGI("setupGraphics(%d, %d)", w, h);

  screen_width = w;
  screen_height = h;

  cam = new Camera();
  axis = new Axis();
  frustum = new Frustum();
  trace = new Trace();
  grid = new Grid();

  camera_type = FIRST_PERSON;
  cam->SetAspectRatio((float) (w / h));
  return true;
}

// Render frustum and trace with current position and rotation
// updated from TangoData, TangoPosition and TangoRotation is updated via callback function
// OnPoseAvailable(), which is updated when new pose data is available.
bool RenderFrame() {
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glViewport(0, 0, screen_width, screen_height);

  grid->SetPosition(glm::vec3(0.0f, -0.8f, 0.0f));
  grid->Render(cam->GetCurrentProjectionViewMatrix());

  glm::vec3 position = GlUtil::ConvertPositionToOpenGL(
      TangoData::GetInstance().GetTangoPosition());
  glm::quat rotation = GlUtil::ConvertRotationToOpenGL(
      TangoData::GetInstance().GetTangoRotation());

  if (camera_type == FIRST_PERSON) {
    cam->SetPosition(position);
    cam->SetRotation(rotation);
  } else {
    frustum->SetPosition(position);
    frustum->SetRotation(rotation);
    frustum->Render(cam->GetCurrentProjectionViewMatrix());

    trace->UpdateVertexArray(position);
    trace->Render(cam->GetCurrentProjectionViewMatrix());

    axis->SetPosition(position);
    axis->SetRotation(rotation);
    axis->Render(cam->GetCurrentProjectionViewMatrix());
  }
  return true;
}

void SetCamera(int camera_index) {
  camera_type = camera_index;
  switch (camera_index) {
    case FIRST_PERSON:
      LOGI("setting to First Person Camera");
      break;
    case THIRD_PERSON:
      cam->SetPosition(kThirdPersonCameraPosition);
      cam->SetRotation(kThirdPersonCameraRotation);
      LOGI("setting to Third Person Camera");
      break;
    case TOP_DOWN:
      cam->SetPosition(kTopDownCameraPosition);
      cam->SetRotation(kTopDownCameraRotation);
      LOGI("setting to Top Down Camera");
      break;
    default:
      break;
  }
}

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_OnCreate(
    JNIEnv* env, jobject obj) {
  LOGI("In onCreate: Initialing and setting config");
  if (!TangoData::GetInstance().Initialize())
  {
    LOGE("Tango initialization failed");
  }
  if (!TangoData::GetInstance().SetConfig())
  {
    LOGE("Tango set config failed");
  }
}

JNIEXPORT void JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_OnResume(
    JNIEnv* env, jobject obj) {
  LOGI("In OnResume: Locking config and connecting service");
  if (TangoData::GetInstance().LockConfig()) {
    LOGE("Tango lock config failed");
  }
  if (TangoData::GetInstance().Connect()) {
    LOGE("Tango connect failed");
  }
}

JNIEXPORT void JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_OnPause(
    JNIEnv* env, jobject obj) {
  LOGI("In OnPause: Unlocking config and disconnecting service");
  if (TangoData::GetInstance().UnlockConfig()) {
    LOGE("Tango unlock file failed");
  }
  TangoData::GetInstance().Disconnect();
}

JNIEXPORT void JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_OnDestroy(
    JNIEnv* env, jobject obj) {
  delete cam;
  delete axis;
  delete grid;
  delete frustum;
  delete trace;
}

JNIEXPORT void JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_SetupGraphic(
    JNIEnv* env, jobject obj, jint width, jint height) {
  SetupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_Render(
    JNIEnv* env, jobject obj) {
  RenderFrame();
}

JNIEXPORT void JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_SetCamera(
    JNIEnv* env, jobject obj, int camera_index) {
  SetCamera(camera_index);
}

JNIEXPORT jint JNICALL Java_com_projecttango_ctangojniareadescription_TangoJNINative_GetCurrentStatus(
    JNIEnv* env, jobject obj, int camera_index) {
  return TangoData::GetInstance().GetTangoPoseStatus();
}

#ifdef __cplusplus
}
#endif
