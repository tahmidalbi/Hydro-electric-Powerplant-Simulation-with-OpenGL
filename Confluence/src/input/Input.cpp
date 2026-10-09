#include "Input.h"
#include "Picking.h"
#include "../App.h"
#include "../objects/Layout.h"
#include "../systems/Animation.h"
#include "../systems/BoatSystem.h"
#include "../systems/Environment.h"
#include "../systems/HydroSystem.h"
#include "../ui/DemoGallery.h"
#include "../ui/Hud.h"
#define GLFW_INCLUDE_NONE   // this file makes no OpenGL calls
#include <GLFW/glfw3.h>
#include <cstdio>
#include <iostream>
#include <string>

namespace {

// Rates for HELD keys (per REAL second).
constexpr float WIND_CHANGE_PER_SECOND = 6.0f;      // m/s per second
constexpr float TILT_CHANGE_PER_SECOND = 12.0f;     // degrees per second

App* appFrom(GLFWwindow* window) { return static_cast<App*>(glfwGetWindowUserPointer(window)); }

void say(const std::string& message) { std::cout << "  > " << message << std::endl; }

std::string formatText(const char* format, float value) {
    char buffer[96];
    std::snprintf(buffer, sizeof(buffer), format, value);
    return buffer;
}

bool held(GLFWwindow* window, int key) { return glfwGetKey(window, key) == GLFW_PRESS; }

// ---- cycling helpers for the K and L keys --------------------------------
LightingTerms nextTerms(LightingTerms t) {
    switch (t) {
        case LightingTerms::All:          return LightingTerms::AmbientOnly;
        case LightingTerms::AmbientOnly:  return LightingTerms::DiffuseOnly;
        case LightingTerms::DiffuseOnly:  return LightingTerms::SpecularOnly;
        case LightingTerms::SpecularOnly: return LightingTerms::All;
    }
    return LightingTerms::All;
}

LightSet nextLightSet(LightSet s) { return (LightSet)(((int)s + 1) % 4); }

void setShading(App& app, ShadingMode mode, const char* explanation) {
    app.state.view.shading = mode;
    say(std::string("shading: ") + Hud::shadingName(mode) + " - " + explanation);
}

void changeDetail(App& app, int step) {
    int detail = (int)app.state.view.galleryDetail + step;
    if (detail < 0) detail = 0;
    if (detail > 2) detail = 2;
    app.state.view.galleryDetail = (GalleryDetail)detail;
    const char* names[] = {"LOW", "MEDIUM", "HIGH"};
    say(std::string("gallery mesh detail: ") + names[detail] + (app.state.view.galleryActive ? "" : " (open the gallery with F6 to see it)"));
}

void goToView(App& app, CameraView view) {
    if (app.state.view.galleryActive) return;
    CameraViews::goTo(app.director, app.sceneCamera, view, app.state.plant, false);
    say(std::string("camera: ") + CameraViews::name(view));
}

void select(App& app, Selectable which) {
    app.state.view.selected = which;
    say(std::string("selected: ") + Picking::name(which));
}

// ---- TOGGLE keys: act once per press --------------------------------------
void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int mods) {
    if (action != GLFW_PRESS) return;
    App& app = *appFrom(window);
    ViewSettings& view = app.state.view;
    PlantState& plant = app.state.plant;

    switch (key) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, true); break;

        // Step 1: time
        case GLFW_KEY_SPACE:
            app.clock.togglePause();
            say(app.clock.isPaused() ? "PAUSED (camera and lamp still move)" : "resumed");
            break;
        case GLFW_KEY_LEFT_BRACKET:
            app.clock.slower();
            say(formatText("time scale x%g", app.clock.getTimeScale()));
            break;
        case GLFW_KEY_RIGHT_BRACKET:
            app.clock.faster();
            say(formatText("time scale x%g", app.clock.getTimeScale()));
            break;
        case GLFW_KEY_ENTER:
        case GLFW_KEY_KP_ENTER:
            Input::resetScene(app);
            say("scene reset: time 0, every part at its start pose, particles cleared, time scale x1");
            break;
        case GLFW_KEY_R:
            if (view.galleryActive) Input::resetGalleryCamera(app); else Input::resetSceneCamera(app, false);
            say("camera reset");
            break;

        // Step 4: shading method
        case GLFW_KEY_1: case GLFW_KEY_KP_1:
            setShading(app, ShadingMode::Flat, "one colour per triangle (lit once at its centre)");
            break;
        case GLFW_KEY_2: case GLFW_KEY_KP_2:
            setShading(app, ShadingMode::Gouraud, "lit at each vertex, colours blended");
            break;
        case GLFW_KEY_3: case GLFW_KEY_KP_3:
            setShading(app, ShadingMode::Phong, "normals blended, lit at each pixel");
            break;

        // Step 3: lighting
        case GLFW_KEY_B:
            view.useBlinn = !view.useBlinn;
            say(view.useBlinn ? "specular: Blinn-Phong (half vector H)" : "specular: Phong (reflection vector R)");
            break;
        case GLFW_KEY_K:
            view.terms = nextTerms(view.terms);
            say(std::string("lighting terms: ") + Hud::termsName(view.terms));
            break;
        case GLFW_KEY_L:
            view.lights = nextLightSet(view.lights);
            say(std::string("lights: ") + Environment::lightSetName(view.lights));
            break;

        // Step 2: proof views
        case GLFW_KEY_X:
            view.wireframe = !view.wireframe;
            say(view.wireframe ? "wireframe ON" : "wireframe OFF");
            break;
        case GLFW_KEY_N:
            view.showNormals = !view.showNormals;
            say(view.showNormals ? "normals view ON (red=+x, green=+y, blue=+z)" : "normals view OFF");
            break;
        case GLFW_KEY_EQUAL: case GLFW_KEY_KP_ADD:      changeDetail(app, +1); break;
        case GLFW_KEY_MINUS: case GLFW_KEY_KP_SUBTRACT: changeDetail(app, -1); break;
        case GLFW_KEY_F6:
            view.galleryActive = !view.galleryActive;
            say(view.galleryActive ? "GALLERY (F6 again to return to the plant)" : "back to the power plant");
            break;
        case GLFW_KEY_O:
            view.galleryWrongMatrixOrder = !view.galleryWrongMatrixOrder;
            say(view.galleryWrongMatrixOrder ? "gallery bar: WRONG order S*R*T (it flies off its post)"
                                             : "gallery bar: correct order T*R*S (spins in place)");
            break;
        case GLFW_KEY_M:
            view.galleryWrongNormalMatrix = !view.galleryWrongNormalMatrix;
            say(view.galleryWrongNormalMatrix ? "ellipsoid normals: WRONG (plain model matrix)"
                                              : "ellipsoid normals: correct (inverse-transpose)");
            break;

        // Step 6: gate
        case GLFW_KEY_G:
            HydroSystem::toggleGate(plant);
            say(plant.gateTarget > 0.5f ? "gate: OPENING (target 100%)" : "gate: CLOSING (target 0%)");
            break;
        case GLFW_KEY_J:
            plant.gateAutoDemo = !plant.gateAutoDemo;
            say(plant.gateAutoDemo ? "gate: AUTOMATIC demonstration (any gate key takes control back)" : "gate: manual");
            break;

        // Step 7: boat
        case GLFW_KEY_V:
            BoatSystem::toggleMode(plant);
            say(plant.boatMode == BoatMode::Manual ? "boat: MANUAL (W/S throttle, A/D rudder)" : "boat: PATROL (autopilot)");
            break;
        case GLFW_KEY_W: case GLFW_KEY_S: case GLFW_KEY_A: case GLFW_KEY_D:
            if (!view.galleryActive && plant.boatMode == BoatMode::Patrol)
                say("the boat is on autopilot - press V to steer it yourself");
            break;

        // Step 8: views, projection, selection, screen
        case GLFW_KEY_F1: goToView(app, CameraView::Overview); break;
        case GLFW_KEY_F2: goToView(app, CameraView::Turbine); break;
        case GLFW_KEY_F3: goToView(app, CameraView::DamAndWheel); break;
        case GLFW_KEY_F4: goToView(app, CameraView::Solar); break;
        case GLFW_KEY_F5: goToView(app, CameraView::Boat); break;
        case GLFW_KEY_P:
            view.orthographic = !view.orthographic;
            say(view.orthographic ? "projection: ORTHOGRAPHIC (parallel lines stay parallel)" : "projection: PERSPECTIVE");
            break;
        case GLFW_KEY_TAB: {
            int count = (int)Selectable::Count;
            int step = (mods & GLFW_MOD_SHIFT) ? count - 1 : 1;
            select(app, (Selectable)(((int)view.selected + step) % count));
            break;
        }
        case GLFW_KEY_BACKSPACE:
            view.hudVisible = !view.hudVisible;
            say(view.hudVisible ? "panel shown" : "panel hidden (clean view)");
            break;
        case GLFW_KEY_F11:
            view.helpVisible = !view.helpVisible;
            Hud::printHelp();
            break;
        case GLFW_KEY_F12:
            app.screenshotRequested = true;
            break;
        default: break;
    }
}

// ---- Mouse: orbit, zoom, pick ---------------------------------------------
void mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
    App& app = *appFrom(window);
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        app.dragging = (action == GLFW_PRESS);
        glfwGetCursorPos(window, &app.lastMouseX, &app.lastMouseY);
        if (app.dragging) CameraViews::cancelGlide(app.director);   // the user takes the camera
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && !app.state.view.galleryActive) {
        // Window coordinates -> framebuffer pixels (they differ on high-DPI screens).
        double x, y;
        int winW, winH, fbW, fbH;
        glfwGetCursorPos(window, &x, &y);
        glfwGetWindowSize(window, &winW, &winH);
        glfwGetFramebufferSize(window, &fbW, &fbH);
        if (winW <= 0 || winH <= 0 || fbW <= 0 || fbH <= 0) return;
        x *= (double)fbW / winW;
        y *= (double)fbH / winH;

        const Camera& cam = app.sceneCamera;
        glm::mat4 projection = cam.projection((float)fbW / fbH, Layout::VFOV_DEG, app.state.view.orthographic);
        Ray ray = Picking::rayThroughPixel(x, y, fbW, fbH, cam.getViewMatrix(), projection);
        select(app, Picking::pick(ray, app.state.plant));
    }
}

void cursorPosCallback(GLFWwindow* window, double x, double y) {
    App& app = *appFrom(window);
    if (!app.dragging) return;
    app.activeCamera().orbit((float)((x - app.lastMouseX) * 0.3), (float)((y - app.lastMouseY) * 0.3));
    app.lastMouseX = x;
    app.lastMouseY = y;
}

void scrollCallback(GLFWwindow* window, double /*xOffset*/, double yOffset) {
    App& app = *appFrom(window);
    CameraViews::cancelGlide(app.director);
    Camera& cam = app.activeCamera();
    // Zoom by a fraction of the distance: same feel close up and far away.
    cam.zoom((float)(-yOffset) * 0.12f * cam.radius);
}

}

void Input::install(GLFWwindow* window, App* app) {
    glfwSetWindowUserPointer(window, app);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
}

// ---- HELD keys: polled every frame ----------------------------------------
void Input::handleHeldKeys(GLFWwindow* window, App& app, float dt) {
    PlantState& plant = app.state.plant;

    // The inspection lamp (works while paused).
    const float lampSpeed = app.state.view.galleryActive ? 5.0f : 10.0f;
    glm::vec3 move(0.0f);
    if (held(window, GLFW_KEY_LEFT))      move.x -= 1.0f;
    if (held(window, GLFW_KEY_RIGHT))     move.x += 1.0f;
    if (held(window, GLFW_KEY_UP))        move.z -= 1.0f;   // away from the camera
    if (held(window, GLFW_KEY_DOWN))      move.z += 1.0f;
    if (held(window, GLFW_KEY_PAGE_UP))   move.y += 1.0f;
    if (held(window, GLFW_KEY_PAGE_DOWN)) move.y -= 1.0f;
    app.state.activeLampPosition() += move * lampSpeed * dt;

    // Step 5: wind target.
    if (held(window, GLFW_KEY_Q)) plant.windTargetMetersPerSecond -= WIND_CHANGE_PER_SECOND * dt;
    if (held(window, GLFW_KEY_E)) plant.windTargetMetersPerSecond += WIND_CHANGE_PER_SECOND * dt;

    // Step 6: gate target (also ends the automatic demonstration).
    if (held(window, GLFW_KEY_F)) HydroSystem::nudgeTarget(plant, -HydroSystem::TARGET_NUDGE_PER_SECOND * dt);
    if (held(window, GLFW_KEY_H)) HydroSystem::nudgeTarget(plant, +HydroSystem::TARGET_NUDGE_PER_SECOND * dt);

    // Step 7: solar tilt.
    if (held(window, GLFW_KEY_Z)) plant.solarTiltDeg -= TILT_CHANGE_PER_SECOND * dt;
    if (held(window, GLFW_KEY_C)) plant.solarTiltDeg += TILT_CHANGE_PER_SECOND * dt;

    // Step 7: boat helm. Stored as INPUTS; the boat itself moves in the
    // update step with simulation time (so pausing freezes it).
    bool steering = plant.boatMode == BoatMode::Manual && !app.state.view.galleryActive;
    plant.boatThrottleInput = steering ? (held(window, GLFW_KEY_W) ? 1.0f : 0.0f) - (held(window, GLFW_KEY_S) ? 1.0f : 0.0f) : 0.0f;
    plant.boatSteerInput = steering ? (held(window, GLFW_KEY_A) ? 1.0f : 0.0f) - (held(window, GLFW_KEY_D) ? 1.0f : 0.0f) : 0.0f;
}

void Input::resetSceneCamera(App& app, bool instant) {
    CameraViews::goTo(app.director, app.sceneCamera, CameraView::Overview, app.state.plant, instant);
}

void Input::resetGalleryCamera(App& app) { app.galleryCamera.lookFrom(DemoGallery::cameraEye(), DemoGallery::cameraTarget()); }

void Input::resetScene(App& app) {
    Animation::reset(app.state.plant);
    app.particles.clear();
    app.effects = EffectsState{};
    app.state.lamp.scenePosition = Environment::defaultSceneLampPosition();
    app.state.lamp.galleryPosition = DemoGallery::lampStartPosition();
    app.clock.resetTimeScale();
}
