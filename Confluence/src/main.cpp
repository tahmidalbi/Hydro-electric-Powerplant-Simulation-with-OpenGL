#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstring>
#include <iostream>
#include <string>

#include "core/Camera.h"
#include "core/Light.h"
#include "core/MeshLibrary.h"
#include "core/Render.h"
#include "core/Screenshot.h"
#include "core/Shader.h"
#include "objects/Layout.h"
#include "objects/Scene.h"
#include "objects/Sky.h"

static Camera g_camera;
static bool   g_dragging = false;
static double g_lastX = 0.0, g_lastY = 0.0;

static int g_shadingMode = 2; // 0 Flat, 1 Gouraud, 2 Phong
static int g_termMask    = 7; // ambient | diffuse | specular
static int g_useBlinn    = 0;

static void resetCamera() { g_camera.lookFrom(Layout::CAMERA_EYE, Layout::CAMERA_TARGET); }

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        g_dragging = (action == GLFW_PRESS);
        glfwGetCursorPos(window, &g_lastX, &g_lastY);
    }
}

static void cursorPosCallback(GLFWwindow*, double x, double y) {
    if (g_dragging) {
        g_camera.orbit((float)((x - g_lastX) * 0.3), (float)((y - g_lastY) * 0.3));
        g_lastX = x;
        g_lastY = y;
    }
}

static void scrollCallback(GLFWwindow*, double /*xoff*/, double yoff) {
    g_camera.zoom((float)(-yoff * 4.0));
}

static void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_1) g_shadingMode = 0;
    if (key == GLFW_KEY_2) g_shadingMode = 1;
    if (key == GLFW_KEY_3) g_shadingMode = 2;
    if (key == GLFW_KEY_B) g_useBlinn = !g_useBlinn;
    if (key == GLFW_KEY_R) resetCamera();
}

int main(int argc, char** argv) {
    // --screenshot <file.bmp>: render one frame offscreen at the reference size and exit.
    std::string screenshotPath;
    for (int i = 1; i + 1 < argc; ++i)
        if (std::strcmp(argv[i], "--screenshot") == 0) screenshotPath = argv[i + 1];
    const bool offscreen = !screenshotPath.empty();

    if (!glfwInit()) {
        std::cerr << "glfwInit failed\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    if (offscreen) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    // Same aspect as the concept image, shrunk if the monitor is small.
    int winW = 1216, winH = 900;
    if (GLFWmonitor* monitor = glfwGetPrimaryMonitor()) {
        int mx, my, mw, mh;
        glfwGetMonitorWorkarea(monitor, &mx, &my, &mw, &mh);
        if (mh > 0 && winH > mh * 0.88f) {
            winH = (int)(mh * 0.88f);
            winW = (int)(winH * Layout::ASPECT);
        }
    }

    GLFWwindow* window = glfwCreateWindow(winW, winH, "Confluence - 3D Hybrid Renewable Energy Power Plant", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "gladLoadGL failed\n";
        return -1;
    }

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    // Face culling stays off: several thin single-sided parts (panel cells,
    // water sheets) must be visible from either side.
    glDisable(GL_CULL_FACE);

    Shader phong("shaders/phong.vert", "shaders/phong.frag");
    MeshLibrary meshes;
    Scene scene;
    Sky sky;
    resetCamera();

    // Sun high behind the viewer's left shoulder, so the faces of the dam,
    // turbines, panels and the hillside turned towards the camera are sunlit.
    Light sun;
    sun.type      = 0;
    sun.direction = glm::normalize(glm::vec3(0.20f, -0.85f, -0.50f));
    sun.diffuse   = {1.00f, 0.97f, 0.90f};
    sun.specular  = {1.0f, 1.0f, 1.0f};

    // Sky fill: soft light from the opposite side so shaded faces aren't black.
    Light fill;
    fill.type      = 0;
    fill.direction = glm::normalize(glm::vec3(-0.6f, -0.5f, 0.6f));
    fill.diffuse   = {0.28f, 0.31f, 0.36f};
    fill.specular  = {0.0f, 0.0f, 0.0f};

    const glm::vec3 globalAmbient(0.40f, 0.44f, 0.50f);

    auto renderFrame = [&](int w, int h) {
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = h > 0 ? (float)w / (float)h : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(Layout::VFOV_DEG), aspect, 0.3f, 1500.0f);
        glm::mat4 view = g_camera.getViewMatrix();

        sky.draw(view, projection, g_camera.getPosition());

        phong.use();
        phong.setMat4("uProjection", projection);
        phong.setMat4("uView", view);
        phong.setVec3("uViewPos", g_camera.getPosition());
        phong.setVec3("uGlobalAmbient", globalAmbient);
        phong.setInt("uShadingMode", g_shadingMode);
        phong.setInt("uTermMask", g_termMask);
        phong.setInt("uUseBlinn", g_useBlinn);
        phong.setVec3("uFogColor", Layout::FOG_COLOR);
        phong.setFloat("uFogDensity", Layout::FOG_DENSITY);
        setLightUniforms(phong, 0, sun);
        setLightUniforms(phong, 1, fill);

        scene.draw(phong, meshes);
    };

    if (offscreen) {
        bool ok = captureToBmp(1216, 900, renderFrame, screenshotPath);
        glfwTerminate();
        return ok ? 0 : 1;
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        renderFrame(fbW, fbH);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
