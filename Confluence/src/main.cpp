// Confluence - 3D Hybrid Renewable Energy Power Plant (OpenGL 3.3)
//
// main.cpp only sets things up and runs the loop. Every frame is:
//
//     1. INPUT   - read the keyboard/mouse          (input/Input.cpp)
//     2. UPDATE  - advance the plant by sim time    (systems/Animation.cpp)
//                  spray / wake particles           (systems/Effects.cpp)
//                  camera glides and boat follow    (systems/CameraDirector.cpp)
//     3. RENDER  - draw what the state says         (rendering/FrameRenderer.cpp)
//
// Geometry, lighting maths and key handling live in their own folders.

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>
#include <new>

#include "App.h"
#include "core/Screenshot.h"
#include "input/CommandLine.h"
#include "input/Input.h"
#include "objects/Layout.h"
#include "rendering/FrameRenderer.h"
#include "systems/Animation.h"
#include "systems/CameraDirector.h"
#include "systems/Effects.h"
#include "tests/SelfTest.h"
#include "ui/Hud.h"

namespace {

// Screenshot size: the concept image's aspect ratio.
constexpr int SHOT_W = 1216, SHOT_H = 900;

GLFWwindow* createWindow(bool hidden) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    if (hidden) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    // Same aspect as the concept image, shrunk if the monitor is small.
    int winW = SHOT_W, winH = SHOT_H;
    if (GLFWmonitor* monitor = glfwGetPrimaryMonitor()) {
        int mx, my, mw, mh;
        glfwGetMonitorWorkarea(monitor, &mx, &my, &mw, &mh);
        if (mh > 0 && winH > mh * 0.88f) {
            winH = (int)(mh * 0.88f);
            winW = (int)(winH * Layout::ASPECT);
        }
    }
    return glfwCreateWindow(winW, winH, "Confluence - 3D Hybrid Renewable Energy Power Plant", nullptr, nullptr);
}

}

int main(int argc, char** argv) {
    App app;
    CommandLineOptions options = CommandLine::parse(argc, argv, app);
    if (!options.valid) return 2;

    // --selftest: pure maths checks, no window needed.
    if (options.selfTest) return SelfTest::runAll() == 0 ? 0 : 1;

    const bool offscreen = !options.screenshotPath.empty();

    if (!glfwInit()) {
        std::cerr << "glfwInit failed\n";
        return -1;
    }
    GLFWwindow* window = createWindow(offscreen);
    if (!window) {
        std::cerr << "Window creation failed (OpenGL 3.3 core is required)\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "gladLoadGL failed\n";
        glfwTerminate();
        return -1;
    }
    std::cout << "OpenGL " << glGetString(GL_VERSION) << " | " << glGetString(GL_RENDERER) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    // Face culling stays off: several thin single-sided parts (panel cells,
    // water sheets) must be visible from either side, and the gallery's
    // mirrored L-shape has reversed winding.
    glDisable(GL_CULL_FACE);

    // GPU resources (shaders, meshes) need the context, so they are created now.
    // unique_ptr: destroyed before glfwTerminate() below.
    std::unique_ptr<FrameRenderer> renderer;
    try {
        renderer = std::make_unique<FrameRenderer>();
    } catch (const std::bad_alloc&) {
        std::cerr << "Start-up failed: out of memory while building the scene meshes.\n"
                     "Close other programs and start Confluence again." << std::endl;
        glfwTerminate();
        return -1;
    } catch (const std::exception& e) {
        std::cerr << "Start-up failed: " << e.what()
                  << "\n(If a shader file was not found: run the program from the folder that contains 'shaders'.)" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Start state: plant/lamp at their start pose, then any command-line
    // changes, then the cameras (the boat view needs the boat's position).
    Input::resetScene(app);
    CommandLine::applyPlantOptions(options, app);
    Input::resetGalleryCamera(app);
    CameraViews::goTo(app.director, app.sceneCamera, options.view, app.state.plant, true);

    // --advance: run the whole simulation forward in fixed 1/60 s steps (for screenshots).
    for (float t = 0.0f; t < options.advanceSeconds; t += 1.0f / 60.0f) {
        Animation::update(app.state.plant, 1.0f / 60.0f);
        Effects::update(app.particles, app.effects, app.state.plant, 1.0f / 60.0f);
        CameraViews::update(app.director, app.sceneCamera, app.state.plant, 1.0f / 60.0f);
    }

    if (offscreen) {
        bool ok = captureToBmp(SHOT_W, SHOT_H, [&](int w, int h) { renderer->render(w, h, app, 60.0f, 0.4f); },
                               options.screenshotPath);
        if (ok) std::cout << "Saved " << options.screenshotPath << std::endl;
        renderer.reset();
        glfwTerminate();
        return ok ? 0 : 1;
    }

    Input::install(window, &app);
    Hud::printHelp();
    Hud::TitleTimer titleTimer;

    while (!glfwWindowShouldClose(window)) {
        // 1. INPUT
        glfwPollEvents();
        app.clock.tick(glfwGetTime());
        Input::handleHeldKeys(window, app, app.clock.realDelta());

        // 2. UPDATE (simulation time: 0 while paused; the camera uses real time)
        Animation::update(app.state.plant, app.clock.simulationDelta());
        Effects::update(app.particles, app.effects, app.state.plant, app.clock.simulationDelta());
        CameraViews::update(app.director, app.sceneCamera, app.state.plant, app.clock.realDelta());

        // 3. RENDER
        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        renderer->render(fbW, fbH, app, titleTimer.fps(), (float)glfwGetTime());
        glfwSwapBuffers(window);

        // F12: render the same frame again into a picture file.
        if (app.screenshotRequested) {
            app.screenshotRequested = false;
            std::string path = "confluence_shot_" + std::to_string(++app.screenshotCount) + ".bmp";
            if (captureToBmp(fbW, fbH, [&](int w, int h) { renderer->render(w, h, app, titleTimer.fps(), (float)glfwGetTime()); }, path))
                std::cout << "  > screenshot saved: " << path << std::endl;
        }

        // Live status in the window title, ~4 times per second.
        if (titleTimer.update(app.clock.realDelta()))
            glfwSetWindowTitle(window, Hud::statusLine(app, renderer->galleryDetailSummary(app), titleTimer.fps()).c_str());
    }

    renderer.reset();   // free GPU objects while the context still exists
    glfwTerminate();
    return 0;
}
