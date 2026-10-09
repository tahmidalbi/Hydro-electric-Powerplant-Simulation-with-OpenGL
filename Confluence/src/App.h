#pragma once
#include "core/Camera.h"
#include "core/Clock.h"
#include "core/SceneState.h"
#include "systems/CameraDirector.h"
#include "systems/Effects.h"
#include "systems/Particles.h"

// Everything the running program remembers between frames, in one place.
// main.cpp owns one App; the input callbacks receive a pointer to it.
struct App {
    Clock clock;                 // real time + simulation time (Step 1)
    SceneState state;            // plant poses, view modes, lamp (Steps 1-8)

    Camera sceneCamera;          // orbit camera for the power plant
    Camera galleryCamera;        // orbit camera for the F6 gallery
    CameraDirector director;     // Step 8: named views, glides, boat follow

    ParticleSystem particles;    // spray, mist, wake (Steps 6-7)
    EffectsState effects;        // emission bookkeeping for the particles

    // Mouse-drag bookkeeping for orbiting.
    bool dragging = false;
    double lastMouseX = 0.0, lastMouseY = 0.0;

    bool screenshotRequested = false;   // F12: save the next frame to a BMP
    int screenshotCount = 0;

    Camera& activeCamera() { return state.view.galleryActive ? galleryCamera : sceneCamera; }
    const Camera& activeCamera() const { return state.view.galleryActive ? galleryCamera : sceneCamera; }
};
