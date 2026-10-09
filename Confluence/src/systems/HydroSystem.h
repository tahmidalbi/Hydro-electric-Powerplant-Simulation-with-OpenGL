#pragma once

struct PlantState;

// STEP 6 - One gate drives the whole hydro side.
//
//   G       : gate target shut <-> open
//   F / H   : lower / raise the target while held
//   J       : automatic demonstration (the target cycles slowly by itself)
//
// The real gate (`gateOpening`) moves towards the target at most GATE_SPEED
// per second, so you can watch it travel up its slot.
//
// DESIGN DECISION: the main gate also feeds the powerhouse intake, so the
// spillway sheets, the water-wheel channel, the spray AND the wheel all use
// the same number:
//
//   flowStrength = gateOpening            (0 .. 1)
//   wheelTarget  = flowStrength * WHEEL_MAX_SPEED
//
// Close the gate and every waterfall fades out, the spray stops, and the wheel
// slows down smoothly and stops. Nothing keeps pouring from a closed supply.
namespace HydroSystem {
    constexpr float GATE_SPEED_PER_SECOND = 0.125f;   // shut -> fully open in 8 s
    constexpr float TARGET_NUDGE_PER_SECOND = 0.35f;  // F / H held
    constexpr float WHEEL_MAX_SPEED = 1.15f;          // rad/s (about 11 rpm) at full flow
    constexpr float AUTO_DEMO_PERIOD_SECONDS = 40.0f;

    float flowStrength(const PlantState& plant);
    float targetWheelSpeed(const PlantState& plant);

    void toggleGate(PlantState& plant);                  // G
    void nudgeTarget(PlantState& plant, float delta);    // F / H (also leaves the auto demo)

    void update(PlantState& plant, float dt);
}
