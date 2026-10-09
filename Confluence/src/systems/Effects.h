#pragma once
#include "../core/Noise.h"

class ParticleSystem;
struct PlantState;

// Feeds the particle pool from what the plant is doing:
//
//   spillway spray  rate = SPRAY_PER_SECOND * flowStrength   (Step 6)
//   chute mist      rate = MIST_PER_SECOND  * flowStrength   (Step 6)
//   wheel spray     rate = WHEEL_PER_SECOND * flowStrength   (Step 6)
//   boat wake       rate = WAKE_PER_METRE   * |boatSpeed|    (Step 7)
//
// A rate of 7.5 per second at 60 FPS means 0.125 particles per frame, so the
// fraction is carried over to the next frame ("carry") instead of being lost.
struct EffectsState {
    float sprayCarry = 0.0f, mistCarry = 0.0f, wheelCarry = 0.0f, wakeCarry = 0.0f;
    Noise::Rng rng{2024u};
};

namespace Effects {
    constexpr float SPRAY_PER_SECOND = 230.0f;
    constexpr float MIST_PER_SECOND = 45.0f;
    constexpr float WHEEL_PER_SECOND = 55.0f;
    constexpr float WAKE_PER_METRE = 7.0f;   // wake particles per metre travelled

    // Emits this frame's new particles, then moves all of them (calls
    // particles.update). The main loop calls only this.
    void update(ParticleSystem& particles, EffectsState& effects, const PlantState& plant, float dt);
}
