#pragma once
#include "../core/Mesh.h"

class Shader;

// Reservoir lake behind the dam (moving waves, Step 7); the river below it,
// churned white where the spillways and the water wheel discharge into it,
// with foam streaks drifting downstream (Step 6); and the wheel channel's
// outflow, which follows the gate (Step 6). The spray is particles now
// (systems/Effects.cpp).
class Water {
public:
    Water();
    void drawSurfaces(const Shader& shader, float flowStrength) const;  // lake + river (opaque); foam follows the flow
    void drawFalls(const Shader& shader, float flowStrength) const;     // translucent wheel-channel fall

private:
    Mesh lake;
    Mesh river;
    Mesh wheelFall;
};
