#include "Common.h"

namespace oria {
  inline void renderRift() {
    using namespace oglplus;
    GL_CHECK_ERROR;

    static ProgramPtr program;
    static ShapeWrapperPtr shape;
    if (!program) {
      oria::addShudownHook([&]{
        program.reset();
        shape.reset();
      });
      GL_CHECK_ERROR;
      GLenum err = glGetError();
      program = loadProgram(Resource::SHADERS_LIT_VS, Resource::SHADERS_LITCOLORED_FS);
      GL_CHECK_ERROR;

      shape = ShapeWrapperPtr(new shapes::ShapeWrapper(List("Position")("Normal").Get(), shapes::CtmMesh(Resource::MESHES_RIFT_CTM), *program));;
      GL_CHECK_ERROR;

    }

    auto & mv = Stacks::modelview();
    mv.withPush([&]{
      mv.rotate(-HALF_PI - 0.22f, Vectors::X_AXIS).scale(0.5f);
      renderGeometry(shape, program, { [&]{
        oria::bindLights(program);
      } });
    });
    GL_CHECK_ERROR;
  }

}
class SensorFusionExample : public GlfwApp {
  ovrHmd hmd;
  glm::quat orientation;
  glm::vec3 linearA;
  glm::vec3 angularV;

  bool renderSensors{ false };

public:

  SensorFusionExample() {
    hmd = ovrHmd_Create(0);
    if (!hmd) {
      FAIL("Unable to open HMD");
    }
    
    if (!ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation, 0)) {
      FAIL("Unable to locate Rift sensor device");
    }
  }

  virtual ~SensorFusionExample() {
    ovrHmd_Destroy(hmd);
  }

  virtual GLFWwindow * createRenderingTarget(glm::uvec2 & outSize, glm::ivec2 & outPosition) {
    outSize = glm::uvec2(800, 600);
    outPosition = glm::ivec2(100, 100);
    Stacks::projection().top() = glm::perspective(
      PI / 3.0f, aspect(outSize),
      0.01f, 10000.0f);
    Stacks::modelview().top() = glm::lookAt(
      glm::vec3(0.0f, 0.0f, 3.5f),
      Vectors::ORIGIN, Vectors::UP);

    return glfw::createWindow(outSize, outPosition);
  }

  void initGl() {
    GlfwApp::initGl();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    GL_CHECK_ERROR;
  }

  virtual void onKey(int key, int scancode, int action, int mods) {
    if (GLFW_PRESS != action && GLFW_REPEAT != action) {
      return;
    }

    switch (key) {
    case GLFW_KEY_S:
      if (0 == (mods & GLFW_MOD_SHIFT)) {
        renderSensors = !renderSensors;
        return;
      }
      break;

    case GLFW_KEY_R:
      ovrHmd_RecenterPose(hmd);
      return;
    }

    GlfwApp::onKey(key, scancode, action, mods);
  }

  void update() {
    ovrTrackingState sensorState = ovrHmd_GetTrackingState(hmd, 0);
    ovrPoseStatef & poseState = sensorState.HeadPose;
    orientation = ovr::toGlm(
      poseState.ThePose.Orientation);
    linearA = ovr::toGlm(
      poseState.LinearAcceleration);
    angularV = ovr::toGlm(
      poseState.AngularVelocity);
  }

  void draw() {
    GL_CHECK_ERROR;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GL_CHECK_ERROR;
    MatrixStack & mv = Stacks::modelview();
    mv.withPush([&]{
      mv.rotate(orientation); 
      oria::renderRift();
    });

    MatrixStack & pr = Stacks::projection();
    pr.withPush([&]{
      pr.top() = glm::ortho(
        -1.0f, 1.0f,
        -windowAspectInverse, windowAspectInverse,
        -100.0f, 100.0f);

      // Text display of our current sensor settings
      //mv.withPush([&]{
      //  mv.identity();
      //  glm::vec3 euler = glm::eulerAngles(orientation);
      //  glm::vec2 cursor(-0.9, windowAspectInverse * 0.9);
      //  std::string message = Platform::format(
      //    "Current orientation\n"
      //    "roll  %0.2f\n"
      //    "pitch %0.2f\n"
      //    "yaw   %0.2f",
      //    euler.z * RADIANS_TO_DEGREES,
      //    euler.x * RADIANS_TO_DEGREES,
      //    euler.y * RADIANS_TO_DEGREES);
      //  GlUtils::renderString(message, cursor, 18.0f);
      //});

      //if (renderSensors) {
      //  mv.withPush([&]{
      //    mv.top() = glm::lookAt(
      //      glm::vec3(3.0f, 1.0f, 3.0f),
      //      GlUtils::ORIGIN, GlUtils::UP);
      //    mv.translate(glm::vec3(0.75f, -0.3f, 0.0f));
      //    mv.scale(0.2f);

      //    GlUtils::draw3dGrid();
      //    GlUtils::draw3dVector(linearA, 
      //      Colors::green);
      //    GlUtils::draw3dVector(angularV, 
      //      Colors::yellow);
      //  });
      //}
    });
  }
};

RUN_OVR_APP(SensorFusionExample)
