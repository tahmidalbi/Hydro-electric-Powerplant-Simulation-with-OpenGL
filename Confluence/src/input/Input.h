#pragma once

struct GLFWwindow;
struct App;

// Turns keys and mouse movements into changes of App state.
//
// Two kinds of key (roadmap Step 8, "Handle keys correctly"):
//   TOGGLE keys act ONCE when pressed (pause, gate open/shut, views, modes ...).
//     -> handled in the GLFW key callback, only for GLFW_PRESS.
//   HELD keys change a value continuously while held (wind, gate target,
//     panel tilt, boat throttle/rudder, the lamp).
//     -> polled every frame in handleHeldKeys(), scaled by the frame time,
//        so the speed never depends on the keyboard's auto-repeat rate.
namespace Input {
    // Registers the key/mouse/scroll callbacks. `app` must outlive the window.
    void install(GLFWwindow* window, App* app);

    // Call once per frame with REAL time (settings can be changed while paused).
    void handleHeldKeys(GLFWwindow* window, App& app, float realDeltaSeconds);

    // Camera presets.
    void resetSceneCamera(App& app, bool instant);
    void resetGalleryCamera(App& app);

    // Enter: plant back to its start pose, particles cleared, lamp back to its
    // start positions, time scale back to 1. View modes (shading etc.) are kept.
    void resetScene(App& app);
}
