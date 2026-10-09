#pragma once
#include <glm/glm.hpp>

struct PlantState;

// STEP 5 - Wind drives the turbines and the clouds.
//
// The rotor speed follows a simplified version of a real turbine's operating
// curve:
//
//   wind <  3 m/s  (cut-in)   : too weak, the rotor stands still
//   3 .. 12 m/s               : speed rises from 6 rpm to 16 rpm
//   12 .. 25 m/s  (rated)     : held at 16 rpm (the blades pitch to spill power)
//   wind > 25 m/s (cut-out)   : storm protection -- the rotor is braked to a stop
//
// Nothing jumps: the wind eases towards the Q/E target, and each heavy rotor
// eases towards the speed the wind asks for (inertia), then
//   rotorAngle += rotorSpeed * simulationDelta
namespace WindSystem {
    constexpr float CUT_IN = 3.0f;
    constexpr float RATED = 12.0f;
    constexpr float CUT_OUT = 25.0f;
    constexpr float MAX_WIND = 30.0f;          // Q/E range: 0 .. 30 m/s
    constexpr float MIN_RPM = 6.0f;
    constexpr float MAX_RPM = 16.0f;
    constexpr float SECOND_TURBINE_FACTOR = 0.93f;   // the two machines are not identical

    // The wind blows towards -x (from the right of the default view to the left).
    glm::vec3 direction();

    // Rotor speed the wind asks for, in rad/s (0 below cut-in and above cut-out).
    float targetRotorSpeed(float windMetersPerSecond);

    // How fast the clouds drift, m/s (winds aloft are a bit faster than at the ground).
    float cloudSpeed(float windMetersPerSecond);

    // A cloud's x position: start position moved by the drift, wrapped into
    // [-WRAP, WRAP] far outside the view, so the jump back is never seen.
    constexpr float CLOUD_WRAP = 520.0f;
    float cloudX(float startX, float driftMeters, float parallax);

    void update(PlantState& plant, float dt);
}
