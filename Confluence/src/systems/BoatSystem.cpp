#include "BoatSystem.h"
#include "Waves.h"
#include "../core/SceneState.h"
#include "../core/Transform.h"
#include "../objects/Layout.h"
#include "../objects/Terrain.h"
#include <algorithm>
#include <cmath>

namespace {

constexpr float PI = 3.14159265358979f;
constexpr float TWO_PI = 2.0f * PI;
constexpr float MIN_WATER_DEPTH = 0.8f;        // metres under the keel
constexpr float DAM_CLEARANCE = 9.0f;          // stay this far upstream of the dam's face
constexpr float AUTOPILOT_LOOKAHEAD_METRES = 14.0f;

// Smallest signed difference between two angles, in (-pi, pi].
float angleDifference(float to, float from) {
    float d = std::fmod(to - from + PI, TWO_PI);
    if (d < 0.0f) d += TWO_PI;
    return d - PI;
}

// Autopilot: steer towards a point a little ahead on the patrol ellipse.
void autopilot(PlantState& plant, float& rudder, float& requestedSpeed) {
    // 1. Closest point of the route (sampled every 5 degrees).
    float bestAngle = 0.0f, bestDistance = 1e30f;
    for (int i = 0; i < 72; ++i) {
        float a = TWO_PI * i / 72.0f;
        float d = glm::length(BoatSystem::patrolPoint(a) - plant.boatXZ);
        if (d < bestDistance) { bestDistance = d; bestAngle = a; }
    }
    // 2. Aim a fixed distance further along it (the boat travels the route in
    //    the direction of decreasing ellipse angle).
    glm::vec2 radii = BoatSystem::patrolRadii();
    float lookaheadAngle = AUTOPILOT_LOOKAHEAD_METRES / (0.5f * (radii.x + radii.y));
    glm::vec2 aim = BoatSystem::patrolPoint(bestAngle - lookaheadAngle);

    // 3. Rudder proportional to the heading error.
    glm::vec2 toAim = aim - plant.boatXZ;
    float desiredHeading = std::atan2(toAim.x, toAim.y);   // forward = (sin h, cos h)
    rudder = std::clamp(2.0f * angleDifference(desiredHeading, plant.boatHeadingRad), -1.0f, 1.0f);
    requestedSpeed = BoatSystem::PATROL_SPEED;
}

}

// Chosen from a map of isNavigable() over the lake: the ellipse keeps several
// metres of deep water around the hull everywhere (checked by the self-test).
glm::vec2 BoatSystem::patrolCentre() { return {40.0f, -145.0f}; }
glm::vec2 BoatSystem::patrolRadii() { return {38.0f, 26.0f}; }

glm::vec2 BoatSystem::patrolPoint(float a) {
    return patrolCentre() + glm::vec2(patrolRadii().x * std::cos(a), patrolRadii().y * std::sin(a));
}

glm::vec2 BoatSystem::forwardVector(float heading) { return {std::sin(heading), std::cos(heading)}; }

bool BoatSystem::isNavigable(float x, float z) {
    if (Layout::damSide(x, z) > -DAM_CLEARANCE) return false;        // too close to (or past) the dam
    if (x * x + z * z > 420.0f * 420.0f) return false;              // far edge of the modelled lake
    return terrainHeight(x, z) < Layout::LAKE_LEVEL - MIN_WATER_DEPTH;
}

bool BoatSystem::hullFits(glm::vec2 p, float heading) {
    glm::vec2 f = forwardVector(heading);
    glm::vec2 side(f.y, -f.x);
    const glm::vec2 samples[4] = {p + f * (HULL_HALF_LENGTH + 0.6f), p - f * (HULL_HALF_LENGTH + 0.2f),
                                  p + side * (HULL_HALF_BEAM + 0.3f), p - side * (HULL_HALF_BEAM + 0.3f)};
    for (const glm::vec2& s : samples)
        if (!isNavigable(s.x, s.y)) return false;
    return true;
}

void BoatSystem::toggleMode(PlantState& plant) {
    plant.boatMode = plant.boatMode == BoatMode::Patrol ? BoatMode::Manual : BoatMode::Patrol;
    plant.boatThrottleInput = 0.0f;
    plant.boatSteerInput = 0.0f;
}

void BoatSystem::update(PlantState& plant, float dt) {
    if (dt <= 0.0f) { floatOnWaves(plant); return; }

    // 1. What the helm asks for.
    float rudder = 0.0f, requestedSpeed = 0.0f;
    if (plant.boatMode == BoatMode::Patrol) {
        autopilot(plant, rudder, requestedSpeed);
    } else {
        rudder = std::clamp(plant.boatSteerInput, -1.0f, 1.0f);
        float throttle = std::clamp(plant.boatThrottleInput, -1.0f, 1.0f);
        requestedSpeed = throttle >= 0.0f ? throttle * MAX_FORWARD_SPEED : throttle * MAX_ASTERN_SPEED;
    }

    // 2. Speed changes gradually.
    float step = ACCELERATION * dt;
    plant.boatSpeed += std::clamp(requestedSpeed - plant.boatSpeed, -step, step);

    // 3. The rudder needs water flowing past it; a stopped boat can still
    //    swing slowly (so it can turn away from a shore it ran into).
    float effectiveness = std::max(0.25f, std::min(1.0f, std::fabs(plant.boatSpeed) / 3.0f));
    float direction = plant.boatSpeed < -0.05f ? -1.0f : 1.0f;   // astern: steering reverses
    float newHeading = plant.boatHeadingRad + TURN_RATE * rudder * effectiveness * direction * dt;
    if (hullFits(plant.boatXZ, newHeading)) plant.boatHeadingRad = std::fmod(newHeading, TWO_PI);

    // 4. Move, unless the hull would touch the shore or the dam.
    glm::vec2 next = plant.boatXZ + forwardVector(plant.boatHeadingRad) * plant.boatSpeed * dt;
    if (hullFits(next, plant.boatHeadingRad)) {
        plant.boatXZ = next;
        plant.boatBlockedByShore = false;
    } else {
        plant.boatSpeed = 0.0f;
        plant.boatBlockedByShore = true;
    }

    floatOnWaves(plant);
}

void BoatSystem::floatOnWaves(PlantState& plant) {
    const float t = (float)plant.simulationTimeSeconds;
    const float wind = plant.windMetersPerSecond;
    glm::vec2 f = forwardVector(plant.boatHeadingRad);
    glm::vec2 side(f.y, -f.x);   // points to the boat's left (+x when the bow faces +z)
    glm::vec2 p = plant.boatXZ;

    float bow   = Waves::height(p.x + f.x * HULL_HALF_LENGTH, p.y + f.y * HULL_HALF_LENGTH, t, wind);
    float stern = Waves::height(p.x - f.x * HULL_HALF_LENGTH, p.y - f.y * HULL_HALF_LENGTH, t, wind);
    float left  = Waves::height(p.x + side.x * HULL_HALF_BEAM, p.y + side.y * HULL_HALF_BEAM, t, wind);
    float right = Waves::height(p.x - side.x * HULL_HALF_BEAM, p.y - side.y * HULL_HALF_BEAM, t, wind);

    plant.boatWaterHeight = 0.25f * (bow + stern + left + right);
    // A planing boat lifts its bow a little as it speeds up.
    plant.boatPitchRad = std::atan((bow - stern) / (2.0f * HULL_HALF_LENGTH)) + 0.010f * std::max(plant.boatSpeed, 0.0f);
    plant.boatRollRad = std::atan((left - right) / (2.0f * HULL_HALF_BEAM));
}

glm::mat4 BoatSystem::modelMatrix(const PlantState& plant) {
    using namespace Transform;
    // Pitch: rotateX(+a) LOWERS the bow (+z), so a bow-up pitch uses -a.
    // Roll:  rotateZ(+a) raises the +x side (the boat's left).
    return translate({plant.boatXZ.x, plant.boatWaterHeight, plant.boatXZ.y})
         * rotateY(plant.boatHeadingRad) * rotateX(-plant.boatPitchRad) * rotateZ(plant.boatRollRad);
}

glm::vec3 BoatSystem::sternPoint(const PlantState& plant) {
    glm::vec2 f = forwardVector(plant.boatHeadingRad);
    glm::vec2 s = plant.boatXZ - f * (HULL_HALF_LENGTH + 0.3f);
    return {s.x, plant.boatWaterHeight + 0.05f, s.y};
}
