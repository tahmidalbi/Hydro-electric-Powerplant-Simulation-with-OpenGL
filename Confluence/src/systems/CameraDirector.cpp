#include "CameraDirector.h"
#include "BoatSystem.h"
#include "../core/Camera.h"
#include "../core/SceneState.h"
#include "../objects/Dam.h"
#include "../objects/Layout.h"
#include "../objects/Placement.h"
#include "../objects/Terrain.h"
#include <algorithm>
#include <cmath>

namespace {

glm::vec3 pointIn(const glm::mat4& frame, float x, float y, float z) { return glm::vec3(frame * glm::vec4(x, y, z, 1.0f)); }

glm::vec3 boatTarget(const PlantState& plant) {
    return {plant.boatXZ.x, plant.boatWaterHeight + 1.0f, plant.boatXZ.y};
}

// Where the eye goes and what it looks at, for each view.
void preset(CameraView view, const PlantState& plant, glm::vec3& eye, glm::vec3& target) {
    switch (view) {
        case CameraView::Overview:
            eye = Layout::CAMERA_EYE;
            target = Layout::CAMERA_TARGET;
            return;
        case CameraView::Turbine: {
            // In front of turbine 1's rotor, a little to the side and below the hub.
            glm::mat4 root = Placement::turbineRoot(0);
            target = Placement::turbineHub(0) - glm::vec3(0.0f, 2.0f, 0.0f);
            eye = pointIn(root, 11.0f, 8.0f, 26.0f);
            return;
        }
        case CameraView::DamAndWheel: {
            // Hovering over the river, downstream, looking back at the gates,
            // the falls, the powerhouse and the wheel. The eye is placed in
            // world space between the two river banks (Layout::leftBankX /
            // rightBankX at z = -22 are about 0.5 and 31), so no hill is in the way.
            glm::mat4 dam = Layout::damRoot();
            target = 0.5f * (pointIn(dam, Dam::bayCentre(1), 6.0f, 10.0f) + pointIn(dam, Dam::wheelMount().x, 4.0f, 11.0f));
            eye = glm::vec3(10.0f, 17.0f, -22.0f);
            return;
        }
        case CameraView::Solar: {
            glm::mat4 root = Placement::solarRoot();
            target = pointIn(root, 0.0f, 1.4f, -1.5f);
            eye = pointIn(root, 6.5f, 6.0f, 12.5f);
            return;
        }
        case CameraView::Boat: {
            // Behind and above the boat, looking along its heading.
            glm::vec2 f = BoatSystem::forwardVector(plant.boatHeadingRad);
            target = boatTarget(plant);
            eye = target + glm::vec3(-f.x * 20.0f + f.y * 7.0f, 8.0f, -f.y * 20.0f - f.x * 7.0f);
            return;
        }
    }
}

float smooth(float p) { p = std::clamp(p, 0.0f, 1.0f); return p * p * (3.0f - 2.0f * p); }

// Yaw is an angle: go the short way round (350 -> 10 is +20, not -340).
float shortestYaw(float from, float to) {
    float d = std::fmod(to - from + 540.0f, 360.0f) - 180.0f;
    return from + d;
}

// Camera limit: lift the eye (raise the pitch in half-degree steps) if it
// would be inside the hill. 340 steps cover the whole -85..85 degree range.
void keepAboveGround(Camera& camera) {
    for (int i = 0; i < 340; ++i) {
        glm::vec3 eye = camera.getPosition();
        if (eye.y > terrainHeight(eye.x, eye.z) + 1.5f || camera.pitch >= 85.0f) break;
        camera.pitch = std::min(85.0f, camera.pitch + 0.5f);
    }
}

}

const char* CameraViews::name(CameraView view) {
    switch (view) {
        case CameraView::Overview:    return "OVERVIEW";
        case CameraView::Turbine:     return "TURBINE";
        case CameraView::DamAndWheel: return "DAM + WHEEL";
        case CameraView::Solar:       return "SOLAR ARRAY";
        case CameraView::Boat:        return "BOAT (FOLLOW)";
    }
    return "?";
}

void CameraViews::goTo(CameraDirector& d, Camera& camera, CameraView view, const PlantState& plant, bool instant) {
    glm::vec3 eye, target;
    preset(view, plant, eye, target);
    Camera goal;
    goal.lookFrom(eye, target);

    d.view = view;
    d.followBoat = (view == CameraView::Boat);
    d.fromTarget = camera.target;  d.toTarget = goal.target;
    d.fromRadius = camera.radius;  d.toRadius = goal.radius;
    d.fromYaw = camera.yaw;        d.toYaw = shortestYaw(camera.yaw, goal.yaw);
    d.fromPitch = camera.pitch;    d.toPitch = goal.pitch;
    d.progress = 0.0f;
    d.gliding = !instant;
    if (instant) {
        camera.target = goal.target;
        camera.radius = goal.radius;
        camera.yaw = goal.yaw;
        camera.pitch = goal.pitch;
    }
}

void CameraViews::cancelGlide(CameraDirector& d) { d.gliding = false; }

void CameraViews::update(CameraDirector& d, Camera& camera, const PlantState& plant, float realDt) {
    // Following the boat: the glide's end point moves with the boat.
    if (d.followBoat) d.toTarget = boatTarget(plant);

    if (d.gliding) {
        d.progress = std::min(1.0f, d.progress + realDt / d.duration);
        float s = smooth(d.progress);
        camera.target = glm::mix(d.fromTarget, d.toTarget, s);
        // Distance blends in log space: 200 m -> 20 m feels even, not rushed at the end.
        camera.radius = std::exp(glm::mix(std::log(d.fromRadius), std::log(d.toRadius), s));
        camera.yaw = glm::mix(d.fromYaw, d.toYaw, s);
        camera.pitch = glm::mix(d.fromPitch, d.toPitch, s);
        if (d.progress >= 1.0f) d.gliding = false;
    } else if (d.followBoat) {
        // After arriving: track the boat smoothly (keeps the user's own orbit angle and zoom).
        camera.target += (boatTarget(plant) - camera.target) * (1.0f - std::exp(-realDt * 4.0f));
    }

    keepAboveGround(camera);
}
