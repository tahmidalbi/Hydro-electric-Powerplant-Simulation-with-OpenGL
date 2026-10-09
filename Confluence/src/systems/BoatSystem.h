#pragma once
#include <glm/glm.hpp>

struct PlantState;

// STEP 7 - The boat on the reservoir.
//
// One set of simple "physics" for both modes (V switches):
//   PATROL : an autopilot steers towards a point a little ahead on a closed
//            elliptical route, at a calm cruising speed.
//   MANUAL : W/S throttle (forward / astern), A/D rudder (left / right).
//
// Every frame:
//   speed   eases towards the requested speed (a boat cannot stop instantly)
//   heading += turnRate * rudder * dt           (rudder only works when moving)
//   next    = position + forward * speed * dt
//   if the hull still fits on water at `next` -> move; else stop ("shore ahead")
//
// Floating: the wave height is sampled under the bow, stern, left and right.
//   height = average of the four
//   pitch  = atan((bow - stern) / length)      roll = atan((left - right) / beam)
// so the boat rides and rocks on exactly the waves that are drawn.
namespace BoatSystem {
    constexpr float PATROL_SPEED = 4.0f;       // m/s (about 8 knots)
    constexpr float MAX_FORWARD_SPEED = 8.0f;
    constexpr float MAX_ASTERN_SPEED = 2.5f;
    constexpr float ACCELERATION = 1.6f;       // m/s per second
    constexpr float TURN_RATE = 0.45f;         // rad/s at full rudder and speed
    constexpr float HULL_HALF_LENGTH = 2.4f;   // bow/stern sample distance from the root
    constexpr float HULL_HALF_BEAM = 0.9f;

    // Closed patrol route: an ellipse on open water in front of the dam.
    glm::vec2 patrolCentre();
    glm::vec2 patrolRadii();
    glm::vec2 patrolPoint(float angleRad);

    // Deep enough water, upstream of the dam (keeps clear of the crest).
    bool isNavigable(float x, float z);
    // All four hull sample points (bow, stern, both sides) are navigable.
    bool hullFits(glm::vec2 positionXZ, float headingRad);

    glm::vec2 forwardVector(float headingRad);   // (sin h, cos h): heading 0 = +z

    void toggleMode(PlantState& plant);           // V
    void update(PlantState& plant, float dt);
    void floatOnWaves(PlantState& plant);         // height / pitch / roll from the waves

    // Root matrix for drawing: T(position) * Ry(heading) * Rx(-pitch) * Rz(roll).
    glm::mat4 modelMatrix(const PlantState& plant);
    // Where the wake starts (just behind the stern, on the water).
    glm::vec3 sternPoint(const PlantState& plant);
}
