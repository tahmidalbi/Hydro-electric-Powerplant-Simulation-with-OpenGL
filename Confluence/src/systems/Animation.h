#pragma once

struct PlantState;

// STEP 1 - Update.
//
// Advances every moving part of the plant by one frame of SIMULATION time.
// Pure maths: no OpenGL, no GLFW, no drawing. The self-test calls it directly.
//
//   simulationTime += simulationDelta
//   then each system:   WindSystem  (Step 5: wind, rotors, clouds)
//                       HydroSystem (Step 6: gate, flow, water wheel)
//                       BoatSystem  (Step 7: boat on the waves)
namespace Animation {
    void update(PlantState& plant, float simulationDeltaSeconds);

    // Puts every moving part back to its documented start pose (Enter key).
    // Rotor and wheel speeds start at the speed the start wind / flow asks
    // for, so nothing has to spin up after a reset.
    void reset(PlantState& plant);
}
