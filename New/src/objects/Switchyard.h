// =============================================================================
//  Switchyard.h  -  the outdoor high-voltage yard behind the powerhouse
//  (video 2:50).  Along the line bay, in the order the power flows:
//     transformer HV bushings -> gantry -> DISCONNECTOR -> CIRCUIT BREAKER
//     -> CURRENT TRANSFORMERS -> SURGE ARRESTERS -> line gantry -> power line
//  The circuit breaker connects the plant to the grid (CLOSE BREAKER).
// =============================================================================
#pragma once
#include "../render/Renderer.h"
#include "FlowLines.h"

namespace YardDims {
    const float BAY_Z[3] = {59.0f, 62.0f, 65.0f};   // the three phases of the main bay
    const float GANTRY_X = 93.0f, BREAKER_X = 102.0f, LINE_GANTRY_X = 116.0f;
    inline Vec3 lineExit(int p) { return Vec3(LINE_GANTRY_X, 20.0f, BAY_Z[p]); }
}

class Switchyard {
public:
    void build();
    void draw(Renderer& r, float breakerClosed01);
    // glow: before the breaker (energised by the transformer) and after it (grid side)
    void drawGlow(Renderer& r, float before, float after, float speed);
    static void latticeTower(MeshBuilder& b, const Vec3& base, float baseHalf, float topHalf, float height, int levels);

private:
    Mesh platform, steelwork, porcelain, breakers, tanks, conductors, building, windowsMesh, fence;
    Mesh lampsGreen, lampsRed;
    FlowLines glowBefore, glowAfter;
};
