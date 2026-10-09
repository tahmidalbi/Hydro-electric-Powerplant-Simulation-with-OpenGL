#pragma once
#include <glm/glm.hpp>

class Camera;
struct PlantState;

// STEP 8 - Named camera views and smooth, cinematic moves between them.
//
//   F1 overview   F2 turbine   F3 dam + wheel   F4 solar array   F5 boat (follows it)
//
// A view is just "where the eye is" and "what it looks at". Instead of
// jumping, the camera GLIDES there: target, distance, yaw and pitch are
// blended from the old values to the new ones with an ease-in-ease-out curve
//   s = smoothstep(progress) = 3p^2 - 2p^3
// so it starts gently, speeds up, and settles gently (like a camera crane).
// Dragging the mouse during a glide cancels it -- the user is always in charge.
//
// Camera limits: the eye is never allowed below the ground.
enum class CameraView { Overview = 0, Turbine, DamAndWheel, Solar, Boat };

struct CameraDirector {
    CameraView view = CameraView::Overview;
    bool followBoat = false;          // F5: keep looking at the moving boat

    bool gliding = false;
    float progress = 0.0f;            // 0 .. 1 through the glide
    float duration = 1.8f;            // seconds (real time)
    glm::vec3 fromTarget{0.0f}, toTarget{0.0f};
    float fromRadius = 1.0f, toRadius = 1.0f;
    float fromYaw = 0.0f, toYaw = 0.0f;
    float fromPitch = 0.0f, toPitch = 0.0f;
};

namespace CameraViews {
    const char* name(CameraView view);

    // Start gliding to a view (instant = jump, used for screenshots).
    void goTo(CameraDirector& director, Camera& camera, CameraView view, const PlantState& plant, bool instant);

    // The user grabbed the camera: stop any glide where it is.
    void cancelGlide(CameraDirector& director);

    // Once per frame with REAL time (glides keep working while paused).
    void update(CameraDirector& director, Camera& camera, const PlantState& plant, float realDt);
}
